#include <pgmspace.h>
#include "SPI.h"
#include "LPD8806.h"
#include "TimerOne.h"

#include "Constants.h"
#include "Utils.h"
#include "Geo.h"


// Hardware SPI for ultra-fast writes
// fixed to very specific pin numbers: on Arduino 168/328, data = pin 11,
// clock = pin 13.  On Mega, data = pin 51, clock = pin 52.
LPD8806 strip = LPD8806(numPixelsReal);

// Principle of operation: at any given time, the LEDs depict an image or
// animation effect (referred to as the "back" image throughout this code).
// Periodically, a transition to a new image or animation effect (referred
// to as the "front" image) occurs.  During this transition, a third buffer
// (the "alpha channel") determines how the front and back images are
// combined; it represents the opacity of the front image.  When the
// transition completes, the "front" then becomes the "back," a new front
// is chosen, and the process repeats.
byte imgData[2][numPixels * 3], // Data for 2 strips worth of imagery
     alphaMask[numPixels],      // Alpha channel for compositing images
     backImgIdx = 0,            // Index of 'back' image (always 0 or 1)
     fxIdx[3];                  // Effect # for back & front images + alpha
int  fxVars[3][50],             // Effect instance variables (explained later)
     tCounter   = -1,           // Countdown to next transition
     transitionTime;            // Duration (in frames) of current transition


// function prototypes, leave these be :)
void renderEffect00(byte idx);
void renderEffect01(byte idx);
void renderEffect02(byte idx);
void renderEffect03(byte idx);
void renderEffectCircleFlow(byte idx);
void renderEffectGroupTest(byte idx);
void renderAlpha00(void);
void renderAlpha01(void);
void renderAlpha02(void);
void renderAlpha03(void);
void callback();

// List of image effect and alpha channel rendering functions; the code for
// each of these appears later in this file.  Just a few to start with...
// simply append new ones to the appropriate list here:
void (*renderEffect[])(byte) = {
//  renderEffect00,
//  renderEffect01
//  renderEffect02,
//  renderEffect03,
//  renderEffectCircleFlow,
  renderEffectGroupTest
},
(*renderAlpha[])(void)  = {
  renderAlpha00,
  renderAlpha01
//  renderAlpha02
};

// ---------------------------------------------------------------------------

void setup() {
	Serial.begin(9600);
	Serial.println("Transition started!");

  // Start up the LED strip.  Note that strip.show() is NOT called here --
  // the callback function will be invoked immediately when attached, and
  // the first thing the calback does is update the strip.
  strip.begin();

  // Initialize random number generator from a floating analog input.
  randomSeed(analogRead(0));
  memset(imgData, 0, sizeof(imgData)); // Clear image data
  fxVars[backImgIdx][0] = 1;           // Mark back image as initialized

  // Timer1 is used so the strip will update at a known fixed frame rate.
  // Each effect rendering function varies in processing complexity, so
  // the timer allows smooth transitions between effects (otherwise the
  // effects and transitions would jump around in speed...not attractive).
  Timer1.initialize();
  Timer1.attachInterrupt(callback, 1000000 / 60); // 60 frames/second
}

void loop() {
  // Do nothing.  All the work happens in the callback() function below,
  // but we still need loop() here to keep the compiler happy.
}

// Timer1 interrupt handler.  Called at equal intervals; 60 Hz by default.
void callback() {
  // Very first thing here is to issue the strip data generated from the
  // *previous* callback.  It's done this way on purpose because show() is
  // roughly constant-time, so the refresh will always occur on a uniform
  // beat with respect to the Timer1 interrupt.  The various effects
  // rendering and compositing code is not constant-time, and that
  // unevenness would be apparent if show() were called at the end.
  strip.show();

  byte frontImgIdx = 1 - backImgIdx,
       *backPtr    = &imgData[backImgIdx][0],
       r, g, b;
  int  i;

  // Always render back image based on current effect index:
  (*renderEffect[fxIdx[backImgIdx]])(backImgIdx);

  // Front render and composite only happen during transitions...
  if(tCounter > 0) {
    // Transition in progress
    byte *frontPtr = &imgData[frontImgIdx][0];
    int  alpha, inv;

    // Render front image and alpha mask based on current effect indices...
    (*renderEffect[fxIdx[frontImgIdx]])(frontImgIdx);
    (*renderAlpha[fxIdx[2]])();

    // ...then composite front over back:
    for(i=0; i<numPixels; i++) {
      alpha = alphaMask[i] + 1; // 1-256 (allows shift rather than divide)
      inv   = 257 - alpha;      // 1-256 (ditto)
      // r, g, b are placed in variables (rather than directly in the
      // setPixelColor parameter list) because of the postincrement pointer
      // operations -- C/C++ leaves parameter evaluation order up to the
      // implementation; left-to-right order isn't guaranteed.
      r = gamma((*frontPtr++ * alpha + *backPtr++ * inv) >> 8);
      g = gamma((*frontPtr++ * alpha + *backPtr++ * inv) >> 8);
      b = gamma((*frontPtr++ * alpha + *backPtr++ * inv) >> 8);
      strip.setPixelColor(i*2, r, g, b);
      strip.setPixelColor(i*2+1, r, g, b);
    }
  } else {
    // No transition in progress; just show back image
    for(i=0; i<numPixels; i++) {
      // See note above re: r, g, b vars.
      r = gamma(*backPtr++);
      g = gamma(*backPtr++);
      b = gamma(*backPtr++);
      strip.setPixelColor(i*2, r, g, b);
      strip.setPixelColor(i*2+1, r, g, b);
    }
  }

  // Count up to next transition (or end of current one):
  tCounter++;
  if(tCounter == 0) { // Transition start
		Serial.println("Transition started!");
    // Randomly pick next image effect and alpha effect indices:
    fxIdx[frontImgIdx] = random((sizeof(renderEffect) / sizeof(renderEffect[0])));
    fxIdx[2]           = random((sizeof(renderAlpha)  / sizeof(renderAlpha[0])));
    transitionTime     = random(1*60, 4*60);
//    transitionTime     = 600;
    fxVars[frontImgIdx][0] = 0; // Effect not yet initialized
    fxVars[2][0]           = 0; // Transition not yet initialized
  } else if(tCounter >= transitionTime) { // End transition
    fxIdx[backImgIdx] = fxIdx[frontImgIdx]; // Move front effect index to back
    backImgIdx        = 1 - backImgIdx;     // Invert back index
    tCounter          = -2*60 - random(2*60); // Hold image 2 to 6 seconds
  }
}

// ---------------------------------------------------------------------------
// Image effect rendering functions.  Each effect is generated parametrically
// (that is, from a set of numbers, usually randomly seeded).  Because both
// back and front images may be rendering the same effect at the same time
// (but with different parameters), a distinct block of parameter memory is
// required for each image.  The 'fxVars' array is a two-dimensional array
// of integers, where the major axis is either 0 or 1 to represent the two
// images, while the minor axis holds 50 elements -- this is working scratch
// space for the effect code to preserve its "state."  The meaning of each
// element is generally unique to each rendering effect, but the first element
// is most often used as a flag indicating whether the effect parameters have
// been initialized yet.  When the back/front image indexes swap at the end of
// each transition, the corresponding set of fxVars, being keyed to the same
// indexes, are automatically carried with them.

// Simplest rendering effect: fill entire image with solid color
void renderEffect00(byte idx) {
  // Only needs to be rendered once, when effect is initialized:
  if(fxVars[idx][0] == 0) {
    byte *ptr = &imgData[idx][0],
      r = random(256), g = random(256), b = random(256);
    for(int i=0; i<numPixels; i++) {
      *ptr++ = r; *ptr++ = g; *ptr++ = b;
    }
    fxVars[idx][0] = 1; // Effect initialized
  }
}

// Rainbow effect (1 or more full loops of color wheel at 100% saturation).
// Not a big fan of this pattern (it's way overused with LED stuff), but it's
// practically part of the Geneva Convention by now.
void renderEffect01(byte idx) {
  if(fxVars[idx][0] == 0) { // Initialize effect?
    // Number of repetitions (complete loops around color wheel); any
    // more than 4 per meter just looks too chaotic and un-rainbow-like.
    // Store as hue 'distance' around complete belt:
    fxVars[idx][1] = (1 + random(4 * ((numPixels + 31) / 32))) * 1536;
    // Frame-to-frame hue increment (speed) -- may be positive or negative,
    // but magnitude shouldn't be so small as to be boring.  It's generally
    // still less than a full pixel per frame, making motion very smooth.
    fxVars[idx][2] = 4 + random(fxVars[idx][1]) / numPixels;
    // Reverse speed and hue shift direction half the time.
    if(random(2) == 0) fxVars[idx][1] = -fxVars[idx][1];
    if(random(2) == 0) fxVars[idx][2] = -fxVars[idx][2];
    fxVars[idx][3] = 0; // Current position
    fxVars[idx][0] = 1; // Effect initialized
  }

  byte *ptr = &imgData[idx][0];
  long color, i;
  for(i=0; i<numPixels; i++) {
    color = hsv2rgb(fxVars[idx][3] + fxVars[idx][1] * i / numPixels,
      255, 255);
    *ptr++ = color >> 16; *ptr++ = color >> 8; *ptr++ = color;
  }
  fxVars[idx][3] += fxVars[idx][2];
}

// Sine wave chase effect
void renderEffect02(byte idx) {
  if(fxVars[idx][0] == 0) { // Initialize effect?
    fxVars[idx][1] = random(1536); // Random hue
    // Number of repetitions (complete loops around color wheel);
    // any more than 4 per meter just looks too chaotic.
    // Store as distance around complete belt in half-degree units:
    fxVars[idx][2] = (1 + random(4 * ((numPixels + 31) / 32))) * 720;
    // Frame-to-frame increment (speed) -- may be positive or negative,
    // but magnitude shouldn't be so small as to be boring.  It's generally
    // still less than a full pixel per frame, making motion very smooth.
    fxVars[idx][3] = 1 + random(fxVars[idx][1]) / numPixels;
    // Reverse direction half the time.
    if(random(2) == 0) fxVars[idx][3] = -fxVars[idx][3];
    fxVars[idx][4] = 0; // Current position
    fxVars[idx][0] = 1; // Effect initialized
  }

  byte *ptr = &imgData[idx][0];
  int  foo;
  long color;
  for(long i=0; i<numPixels; i++) {
    foo = fixSin(fxVars[idx][4] + fxVars[idx][2] * i / numPixels);
    // Peaks of sine wave are white, troughs are black, mid-range
    // values are pure hue (100% saturated).
    color = (foo >= 0) ?
       hsv2rgb(fxVars[idx][1], 254 - (foo * 2), 255) :
       hsv2rgb(fxVars[idx][1], 255, 254 + foo * 2);
    *ptr++ = color >> 16; *ptr++ = color >> 8; *ptr++ = color;
  }
  fxVars[idx][4] += fxVars[idx][3];
}

// Data for American-flag-like colors (20 pixels representing
// blue field, stars and stripes).  This gets "stretched" as needed
// to the full LED strip length in the flag effect code, below.
// Can change this data to the colors of your own national flag,
// favorite sports team colors, etc.  OK to change number of elements.
#define C_RED   160,   0,   0
#define C_WHITE 255, 255, 255
#define C_BLUE    0,   0, 100
static const uint8_t PROGMEM flagTable[]  = {
  C_BLUE , C_WHITE, C_BLUE , C_WHITE, C_BLUE , C_WHITE, C_BLUE,
  C_RED  , C_WHITE, C_RED  , C_WHITE, C_RED  , C_WHITE, C_RED ,
  C_WHITE, C_RED  , C_WHITE, C_RED  , C_WHITE, C_RED };

// Wavy flag effect
void renderEffect03(byte idx) {
  long i, sum, s, x;
  int  idx1, idx2, a, b;
  if(fxVars[idx][0] == 0) { // Initialize effect?
    fxVars[idx][1] = 720 + random(720); // Wavyness
    fxVars[idx][2] = 4 + random(10);    // Wave speed
    fxVars[idx][3] = 200 + random(200); // Wave 'puckeryness'
    fxVars[idx][4] = 0;                 // Current  position
    fxVars[idx][0] = 1;                 // Effect initialized
  }
  for(sum=0, i=0; i<numPixels-1; i++) {
    sum += fxVars[idx][3] + fixCos(fxVars[idx][4] + fxVars[idx][1] *
      i / numPixels);
  }

  byte *ptr = &imgData[idx][0];
  for(s=0, i=0; i<numPixels; i++) {
    x = 256L * ((sizeof(flagTable) / 3) - 1) * s / sum;
    idx1 =  (x >> 8)      * 3;
    idx2 = ((x >> 8) + 1) * 3;
    b    = (x & 255) + 1;
    a    = 257 - b;
    *ptr++ = ((pgm_read_byte(&flagTable[idx1    ]) * a) +
              (pgm_read_byte(&flagTable[idx2    ]) * b)) >> 8;
    *ptr++ = ((pgm_read_byte(&flagTable[idx1 + 1]) * a) +
              (pgm_read_byte(&flagTable[idx2 + 1]) * b)) >> 8;
    *ptr++ = ((pgm_read_byte(&flagTable[idx1 + 2]) * a) +
              (pgm_read_byte(&flagTable[idx2 + 2]) * b)) >> 8;
    s += fxVars[idx][3] + fixCos(fxVars[idx][4] + fxVars[idx][1] *
      i / numPixels);
  }

  fxVars[idx][4] += fxVars[idx][2];
  if(fxVars[idx][4] >= 720) fxVars[idx][4] -= 720;
}

void renderEffectCircleFlow(byte idx) {
  long c;
  int r, g, b, min_value = 125, min_sat = 150, hue_step = 5;
  if(fxVars[idx][0] == 0) { // Initialize effect?
    int step1 = 50 + random(250), step2 = random(150);
    fxVars[idx][1] = random(1536);   // c1 hue
    fxVars[idx][2] = fxVars[idx][1] + step1;   // c2 hue
    fxVars[idx][3] = fxVars[idx][1] + step2;   // c3 hue
    fxVars[idx][4] = fxVars[idx][1] + step1*2;   // c4 hue
    fxVars[idx][5] = fxVars[idx][3] + step2;   // c5 hue

    fxVars[idx][6] = 255 - random(255 - min_value);   // value
    fxVars[idx][7] = 1 + random(5);   // value speed
    fxVars[idx][8] = random(2);   // value direction

    fxVars[idx][9] = 255 - random(255 - min_sat);   // saturation
    fxVars[idx][10] = 1 + random(5);   // sat speed
    fxVars[idx][11] = random(2);   // sat direction

    fxVars[idx][13] = random(15);   // hue speed
    fxVars[idx][14] = random(2);   // hue direction

    fxVars[idx][0] = 1;   // Effect initialized
  }

  // value variation
  if (random(fxVars[idx][7]) == 0) {
    if (fxVars[idx][8] == 0)
      fxVars[idx][6] -= 1;
    else
      fxVars[idx][6] += 1;
  }
  if (fxVars[idx][6] > 254) {
    fxVars[idx][7] = 1 + random(5);
    fxVars[idx][8] = 0;
  }
  else if (fxVars[idx][6] < min_value) {
    fxVars[idx][7] = 1 + random(5);
    fxVars[idx][8] = 1;
  }

  // sat variation
  if (random(fxVars[idx][10]) == 0) {
    if (fxVars[idx][11] == 0)
      fxVars[idx][9] -= 1;
    else
      fxVars[idx][9] += 1;
  }
  if (fxVars[idx][9] > 254) {
    fxVars[idx][10] = 1 + random(5);
    fxVars[idx][11] = 0;
  }
  else if (fxVars[idx][9] < min_sat) {
    fxVars[idx][10] = 1 + random(5);
    fxVars[idx][11] = 1;
  }

  // hue variation
  if (random(fxVars[idx][13]) == 0) {
    if (fxVars[idx][14] == 0) {
      fxVars[idx][1] += hue_step;
      fxVars[idx][2] += hue_step;
      fxVars[idx][3] += hue_step;
      fxVars[idx][4] += hue_step;
      fxVars[idx][5] += hue_step;
    } else {
      fxVars[idx][1] -= hue_step;
      fxVars[idx][2] -= hue_step;
      fxVars[idx][3] -= hue_step;
      fxVars[idx][4] -= hue_step;
      fxVars[idx][5] -= hue_step;
    }
  }

  c = hsv2rgb(fxVars[idx][1], fxVars[idx][9], fxVars[idx][6]);
  r = c >> 16, g = c >> 8, b = c;
  setRangeColor(idx, imgData, firstCircle, r, g, b);

  c = hsv2rgb(fxVars[idx][2], fxVars[idx][9], fxVars[idx][6]);
  r = c >> 16, g = c >> 8, b = c;
  setRangeColor(idx, imgData, secondCircle, r, g, b);

  c = hsv2rgb(fxVars[idx][3], fxVars[idx][9], fxVars[idx][6]);
  r = c >> 16, g = c >> 8, b = c;
  setRangeColor(idx, imgData, thirdCircle, r, g, b);

  c = hsv2rgb(fxVars[idx][4], fxVars[idx][9], fxVars[idx][6]);
  r = c >> 16, g = c >> 8, b = c;
  setRangeColor(idx, imgData, fourthCircle, r, g, b);

  c = hsv2rgb(fxVars[idx][5], fxVars[idx][9], fxVars[idx][6]);
  r = c >> 16, g = c >> 8, b = c;
  setRangeColor(idx, imgData, fifthCircle, r, g, b);
}

void renderEffectGroupTest(byte idx) {
  if(fxVars[idx][0] == 0) { // Initialize effect?
    long c = hsv2rgb(random(1536), random(130, 255), random(255));
    int r = c >> 16, g = c >> 8, b = c;
    int rand = random(3);
    if (rand == 0)
      setGroupColor(idx, imgData, starSmall, starSmallLength, r, g, b);
    else if (rand == 1)
      setGroupColor(idx, imgData, starMiddle, starMiddleLength, r, g, b);
    else if (rand == 2)
      setGroupColor(idx, imgData, Line1, Line1Length, r, g, b);
    fxVars[idx][0] = 1;   // Effect initialized
  }
}

// TO DO: Add more effects here...Larson scanner, etc.

// ---------------------------------------------------------------------------
// Alpha channel effect rendering functions.  Like the image rendering
// effects, these are typically parametrically-generated...but unlike the
// images, there is only one alpha renderer "in flight" at any given time.
// So it would be okay to use local static variables for storing state
// information...but, given that there could end up being many more render
// functions here, and not wanting to use up all the RAM for static vars
// for each, a third row of fxVars is used for this information.

// Simplest alpha effect: fade entire strip over duration of transition.
void renderAlpha00(void) {
  byte fade = 255L * tCounter / transitionTime;
  for(int i=0; i<numPixels; i++) alphaMask[i] = fade;
}

// Straight left-to-right or right-to-left wipe
void renderAlpha01(void) {
  long x, y, b;
  if(fxVars[2][0] == 0) {
    fxVars[2][1] = random(1, numPixels); // run, in pixels
    fxVars[2][2] = (random(2) == 0) ? 255 : -255; // rise
    fxVars[2][0] = 1; // Transition initialized
  }

  b = (fxVars[2][2] > 0) ?
    (255L + (numPixels * fxVars[2][2] / fxVars[2][1])) *
      tCounter / transitionTime - (numPixels * fxVars[2][2] / fxVars[2][1]) :
    (255L - (numPixels * fxVars[2][2] / fxVars[2][1])) *
      tCounter / transitionTime;
  for(x=0; x<numPixels; x++) {
    y = x * fxVars[2][2] / fxVars[2][1] + b; // y=mx+b, fixed-point style
    if(y < 0)         alphaMask[x] = 0;
    else if(y >= 255) alphaMask[x] = 255;
    else              alphaMask[x] = (byte)y;
  }
}

// Dither reveal between images
void renderAlpha02(void) {
  long fade;
  int  i, bit, reverse, hiWord;

  if(fxVars[2][0] == 0) {
    // Determine most significant bit needed to represent pixel count.
    int hiBit, n = (numPixels - 1) >> 1;
    for(hiBit=1; n; n >>=1) hiBit <<= 1;
    fxVars[2][1] = hiBit;
    fxVars[2][0] = 1; // Transition initialized
  }

  for(i=0; i<numPixels; i++) {
    // Reverse the bits in i for ordered dither:
    for(reverse=0, bit=1; bit <= fxVars[2][1]; bit <<= 1) {
      reverse <<= 1;
      if(i & bit) reverse |= 1;
    }
    fade   = 256L * numPixels * tCounter / transitionTime;
    hiWord = (fade >> 8);
    if(reverse == hiWord)     alphaMask[i] = (fade & 255); // Remainder
    else if(reverse < hiWord) alphaMask[i] = 255;
    else                      alphaMask[i] = 0;
  }
}
