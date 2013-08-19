//aldhfalsdghöaidg

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
int  fxVars[3][50];             // Effect instance variables (explained later)
long tCounter   = -1,           // Countdown to next transition
     transitionTime;            // Duration (in frames) of current transition


// function prototypes, leave these be :)
void renderEffectSimpleFill(byte idx);
void renderEffectRainbow(byte idx);
void renderEffectWaveChase(byte idx);
void renderEffectWavyFlag(byte idx);
void renderEffectRingFlow(byte idx);
void renderEffectPent(byte idx);
void renderEffectStars(byte idx);
void renderAlphaSimpleFade(void);
void renderAlphaSinusSchlange(void);
void renderAlphaPixelForPixel(void);
void renderAlphaSinusWobbler(void);
void renderAlphaSparkle(void);
void renderAlphaSwipe(void);
void renderAlphaCentricSwipe(void);
void callback();

// List of image effect and alpha channel rendering functions; the code for
// each of these appears later in this file.  Just a few to start with...
// simply append new ones to the appropriate list here:
void (*renderEffect[])(byte) = {
 /* renderEffectSimpleFill */
//  renderEffectRainbow
  renderEffectWaveChase,
  /* renderEffectWavyFlag, */
  renderEffectRingFlow,
	renderEffectPent,
	renderEffectStars
},
(*renderAlpha[])(void)  = {
  /* renderAlphaSimpleFade */
	renderAlphaSinusSchlange,
  /* renderAlphaPixelForPixel, */
  renderAlphaSinusWobbler,
  renderAlphaSparkle,
	renderAlphaSwipe,
	renderAlphaCentricSwipe
};

// ---------------------------------------------------------------------------

void setup() {
	Serial.begin(115200);

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
  Timer1.attachInterrupt(callback, 1000000 / 60); // frames/second
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
    // Randomly pick next image effect and alpha effect indices:
    fxIdx[frontImgIdx] = random((sizeof(renderEffect) / sizeof(renderEffect[0])));
    fxIdx[2]           = random((sizeof(renderAlpha)  / sizeof(renderAlpha[0])));
    transitionTime     = random(100, 5*30);
		/* transitionTime     = 7*60; */
    fxVars[frontImgIdx][0] = 0; // Effect not yet initialized
    fxVars[2][0]           = 0; // Transition not yet initialized
  } else if(tCounter >= transitionTime) { // End transition
    fxIdx[backImgIdx] = fxIdx[frontImgIdx]; // Move front effect index to back
    backImgIdx        = 1 - backImgIdx;     // Invert back index
    tCounter          = -4*60 - random(6*60); // Hold image 2 to 6 seconds
    /* tCounter          = -60*20; */
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
void renderEffectSimpleFill(byte idx) {
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
void renderEffectRainbow(byte idx) {
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
void renderEffectWaveChase(byte idx) {
  if(fxVars[idx][0] == 0) { // Initialize effect?
    fxVars[idx][1] = random(1536); // Random hue
    // Number of repetitions (complete loops around color wheel);
    // any more than 4 per meter just looks too chaotic.
    // Store as distance around complete belt in half-degree units:
    fxVars[idx][2] = (1 + random(4 * ((numPixels + 31) / 32))) * 720;
    // Frame-to-frame increment (speed) -- may be positive or negative,
    // but magnitude shouldn't be so small as to be boring.  It's generally
    // still less than a full pixel per frame, making motion very smooth.
    fxVars[idx][3] = 1 + random(fxVars[idx][1] / 12) / numPixels;
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
    // Peaks of sine wave are colored, troughs are black, mid-range
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
void renderEffectWavyFlag(byte idx) {
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

void renderEffectRingFlow(byte idx) {
  long c;
  int r, g, b, min_value = 100, min_sat = 130, hue_step = 5;
  if(fxVars[idx][0] == 0) { // Initialize effect?
    int step1 = 80 + random(380), step2 = 30 + random(220);
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
  setRangeColor(idx, imgData, ring1, r, g, b);

  c = hsv2rgb(fxVars[idx][2], fxVars[idx][9], fxVars[idx][6]);
  r = c >> 16, g = c >> 8, b = c;
  setRangeColor(idx, imgData, ring2, r, g, b);

  c = hsv2rgb(fxVars[idx][3], fxVars[idx][9], fxVars[idx][6]);
  r = c >> 16, g = c >> 8, b = c;
  setRangeColor(idx, imgData, ring3, r, g, b);

  c = hsv2rgb(fxVars[idx][4], fxVars[idx][9], fxVars[idx][6]);
  r = c >> 16, g = c >> 8, b = c;
  setRangeColor(idx, imgData, ring4, r, g, b);

  c = hsv2rgb(fxVars[idx][5], fxVars[idx][9], fxVars[idx][6]);
  r = c >> 16, g = c >> 8, b = c;
  setRangeColor(idx, imgData, ring5, r, g, b);
}

void renderEffectPent(byte idx) {
  long c;
  int r, g, b, min_value = 125, min_sat = 150, hue_step = 5;

  if(fxVars[idx][0] == 0) { // Initialize effect?
    int step1 = 20 + random(60), step2 = random(10,50);
    fxVars[idx][1] = random(1536);   // p center hue
    fxVars[idx][2] = fxVars[idx][1] + step1;   // p2 hue
    fxVars[idx][3] = fxVars[idx][1] + step2;   // p3 hue
    fxVars[idx][4] = fxVars[idx][1] + step1*2;   // p4 hue
    fxVars[idx][5] = fxVars[idx][3] + step2;   // p5 hue
    fxVars[idx][6] = fxVars[idx][3] + step2*2;   // p5 hue

    fxVars[idx][7] = 255 - random(255 - min_value);   // value
    fxVars[idx][8] = 1 + random(5);   // value speed
    fxVars[idx][9] = random(2);   // value direction

    fxVars[idx][10] = 255 - random(255 - min_sat);   // saturation
    fxVars[idx][11] = 1 + random(5);   // sat speed
    fxVars[idx][12] = random(2);   // sat direction

    fxVars[idx][13] = random(15);   // hue speed
    fxVars[idx][14] = random(2);   // hue direction

    fxVars[idx][0] = 1;   // Effect initialized
  }

  // value variation
  if (random(fxVars[idx][8]) == 0) {
    if (fxVars[idx][9] == 0)
      fxVars[idx][7] -= 1;
    else
      fxVars[idx][7] += 1;
  }
  if (fxVars[idx][7] > 254) {
    fxVars[idx][8] = 1 + random(5);
    fxVars[idx][9] = 0;
  }
  else if (fxVars[idx][7] < min_value) {
    fxVars[idx][8] = 1 + random(5);
    fxVars[idx][9] = 1;
  }

  // sat variation
  if (random(fxVars[idx][11]) == 0) {
    if (fxVars[idx][12] == 0)
      fxVars[idx][10] -= 1;
    else
      fxVars[idx][10] += 1;
  }
  if (fxVars[idx][10] > 254) {
    fxVars[idx][11] = 1 + random(5);
    fxVars[idx][12] = 0;
  }
  else if (fxVars[idx][10] < min_sat) {
    fxVars[idx][11] = 1 + random(5);
    fxVars[idx][12] = 1;
  }

  // hue variation
  if (random(fxVars[idx][13]) == 0) {
    if (fxVars[idx][14] == 0) {
      fxVars[idx][1] += hue_step;
      fxVars[idx][2] += hue_step;
      fxVars[idx][3] += hue_step;
      fxVars[idx][4] += hue_step;
      fxVars[idx][5] += hue_step;
      fxVars[idx][6] += hue_step;
    } else {
      fxVars[idx][1] -= hue_step;
      fxVars[idx][2] -= hue_step;
      fxVars[idx][3] -= hue_step;
      fxVars[idx][4] -= hue_step;
      fxVars[idx][5] -= hue_step;
      fxVars[idx][6] -= hue_step;
    }
  }

  c = hsv2rgb(fxVars[idx][1], fxVars[idx][10], fxVars[idx][7]);
  r = c >> 16, g = c >> 8, b = c;
	setGroupColor(idx, imgData, pentCenter, pentCenterLength, r, g, b);

  c = hsv2rgb(fxVars[idx][2], fxVars[idx][10], fxVars[idx][7]);
  r = c >> 16, g = c >> 8, b = c;
	setGroupColor(idx, imgData, pent1, pent1Length, r, g, b);

  c = hsv2rgb(fxVars[idx][3], fxVars[idx][10], fxVars[idx][7]);
  r = c >> 16, g = c >> 8, b = c;
	setGroupColor(idx, imgData, pent2, pent2Length, r, g, b);

  c = hsv2rgb(fxVars[idx][4], fxVars[idx][10], fxVars[idx][7]);
  r = c >> 16, g = c >> 8, b = c;
	setGroupColor(idx, imgData, pent3, pent3Length, r, g, b);

  c = hsv2rgb(fxVars[idx][5], fxVars[idx][10], fxVars[idx][7]);
  r = c >> 16, g = c >> 8, b = c;
	setGroupColor(idx, imgData, pent4, pent4Length, r, g, b);

  c = hsv2rgb(fxVars[idx][6], fxVars[idx][10], fxVars[idx][7]);
  r = c >> 16, g = c >> 8, b = c;
	setGroupColor(idx, imgData, pent5, pent5Length, r, g, b);
}

void renderEffectStars(byte idx) {
  int hue_step = 5;
  if(fxVars[idx][0] == 0) { // Initialize effect?
		fxVars[idx][1] = random(3); // structure to show
		fxVars[idx][2] = random(1536); // hue
		fxVars[idx][3] = random(100,255); // sat
		fxVars[idx][4] = random(120,255); // value
    fxVars[idx][5] = 4 + random(10);   // hue speed
    fxVars[idx][6] = random(2);   // hue direction

    fxVars[idx][7] = random(600) - 300;   // small star hue deviation

    fxVars[idx][0] = 1;   // Effect initialized
  }

  // hue variation
  if (random(fxVars[idx][5]) == 0) {
    if (fxVars[idx][6] == 0) fxVars[idx][2] += hue_step;
    else fxVars[idx][2] -= hue_step;
  }

	long c = hsv2rgb(fxVars[idx][2], fxVars[idx][3], fxVars[idx][4]);
	int r = c >> 16, g = c >> 8, b = c;
	if (fxVars[idx][1] == 0)
		setGroupColor(idx, imgData, starSmall, starSmallLength, r, g, b);
	else if (fxVars[idx][1] == 1)
		setGroupColor(idx, imgData, starMiddle, starMiddleLength, r, g, b);
	else if (fxVars[idx][1] == 2) {
		setGroupColor(idx, imgData, starBig, starBigLength, r, g, b);
		c = hsv2rgb(fxVars[idx][2] + fxVars[idx][7], fxVars[idx][3], fxVars[idx][4]);
		r = c >> 16, g = c >> 8, b = c;
		setGroupColor(idx, imgData, starSmall, starSmallLength, r, g, b);
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
void renderAlphaSimpleFade(void) {
  byte fade = 255L * tCounter / transitionTime;
  for(int i=0; i<numPixels; i++) alphaMask[i] = fade;
}

// Straight left-to-right or right-to-left wipe
void renderAlphaSinusSchlange(void) {
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
void renderAlphaPixelForPixel(void) {
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

void renderAlphaSinusWobbler(void) {
  if(fxVars[2][0] == 0) {
		fxVars[2][0] = 1;
		fxVars[2][1] = random(6);
	}

	float t = (float)tCounter / (float)transitionTime;
	float t_inv = 1. - t;

	int i, x = t * 360 * 3 * fxVars[2][1];
	int s = fixSin(x - 180) + 127;
	int a = (int)(s * t_inv) + (int)(t * 255);
	for(i=0; i<numPixels; i++) {
		alphaMask[i] = a;
	}
}

void renderAlphaSparkle(void) {
  if(fxVars[2][0] == 0) {
		fxVars[2][0] = 1;
		fxVars[2][1] = random(40, 70); // sparkle strength
	}

	float t = (float)tCounter / (float)transitionTime;
	float t_inv = 1. - t;
	int i, r = (int)(t * (float)fxVars[2][1]);
	for(i=0; i<numPixels; i++) {
		alphaMask[i] = (int)(random(r) * t_inv) + (int)(t * 255);
	}
}

void renderAlphaSwipe(void) {
  if(fxVars[2][0] == 0) {
		int i;
		for(i=0; i<numPixels; i++) {
			alphaMask[i] = 0;
		}
		fxVars[2][1] = random(2); // direction
		fxVars[2][0] = 1;
	}
	float slot = 1./11.;
	float t = (float)tCounter / (float)transitionTime;
	if (fxVars[2][1] == 0) {
		if (t < 4.*slot)
			setGroupAlpha(alphaMask, line1, line1Length, t * 11./4. * 255);
		if (t > slot && t < 5.*slot)
			setGroupAlpha(alphaMask, line2, line2Length, (t * 11./4. - 0.25) * 255);
		if (t > 2.*slot && t < 6.*slot)
			setGroupAlpha(alphaMask, line3, line3Length, (t * 11./4. - 0.5) * 255);
		if (t > 3.*slot && t < 7.*slot)
			setGroupAlpha(alphaMask, line4, line4Length, (t * 11./4. - 0.75) * 255);
		if (t > 4.*slot && t < 8.*slot)
			setGroupAlpha(alphaMask, line5, line5Length, (t * 11./4. - 1.) * 255);
		if (t > 5.*slot && t < 9.*slot)
			setGroupAlpha(alphaMask, line6, line6Length, (t * 11./4. - 1.25) * 255);
		if (t > 6.*slot && t < 10.*slot)
			setGroupAlpha(alphaMask, line7, line7Length, (t * 11./4. - 1.5) * 255);
		if (t > 7.*slot && t < 11.*slot)
			setGroupAlpha(alphaMask, line8, line8Length, (t * 11./4. - 1.75) * 255);
	}
	else {
		if (t < 4.*slot)
			setGroupAlpha(alphaMask, line8, line8Length, t * 11./4. * 255);
		if (t > slot && t < 5.*slot)
			setGroupAlpha(alphaMask, line7, line7Length, (t * 11./4. - 0.25) * 255);
		if (t > 2.*slot && t < 6.*slot)
			setGroupAlpha(alphaMask, line6, line6Length, (t * 11./4. - 0.5) * 255);
		if (t > 3.*slot && t < 7.*slot)
			setGroupAlpha(alphaMask, line5, line5Length, (t * 11./4. - 0.75) * 255);
		if (t > 4.*slot && t < 8.*slot)
			setGroupAlpha(alphaMask, line4, line4Length, (t * 11./4. - 1.) * 255);
		if (t > 5.*slot && t < 9.*slot)
			setGroupAlpha(alphaMask, line3, line3Length, (t * 11./4. - 1.25) * 255);
		if (t > 6.*slot && t < 10.*slot)
			setGroupAlpha(alphaMask, line2, line2Length, (t * 11./4. - 1.5) * 255);
		if (t > 7.*slot && t < 11.*slot)
			setGroupAlpha(alphaMask, line1, line1Length, (t * 11./4. - 1.75) * 255);
	}
}

void renderAlphaCentricSwipe(void) {
  if(fxVars[2][0] == 0) {
		int i;
		for(i=0; i<numPixels; i++) {
			alphaMask[i] = 0;
		}
		fxVars[2][1] = random(2); // direction
		fxVars[2][0] = 1;
	}
	float slot = 1./7.;
	float t = (float)tCounter / (float)transitionTime;
	if (fxVars[2][1] == 0) {
		if (t < 3.*slot)
			setRangeAlpha(alphaMask, ring1, t * 7./3. * 255);
		if (t > slot && t < 4.*slot)
			setRangeAlpha(alphaMask, ring2, (t * 7./3. - 0.333) * 255);
		if (t > 2.*slot && t < 5.*slot)
			setRangeAlpha(alphaMask, ring3, (t * 7./3. - 0.667) * 255);
		if (t > 3.*slot && t < 6.*slot)
			setRangeAlpha(alphaMask, ring4, (t * 7./3. - 1.) * 255);
		if (t > 4.*slot && t < 7.*slot)
			setRangeAlpha(alphaMask, ring5, (t * 7./3. - 1.333) * 255);
	} else {
		if (t < 3.*slot)
			setRangeAlpha(alphaMask, ring5, t * 7./3. * 255);
		if (t > slot && t < 4.*slot)
			setRangeAlpha(alphaMask, ring4, (t * 7./3. - 0.333) * 255);
		if (t > 2.*slot && t < 5.*slot)
			setRangeAlpha(alphaMask, ring3, (t * 7./3. - 0.667) * 255);
		if (t > 3.*slot && t < 6.*slot)
			setRangeAlpha(alphaMask, ring2, (t * 7./3. - 1.) * 255);
		if (t > 4.*slot && t < 7.*slot)
			setRangeAlpha(alphaMask, ring1, (t * 7./3. - 1.333) * 255);
	}
}
