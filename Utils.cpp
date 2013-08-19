#ifndef UTILS_cpp
#define UTILS_cpp

#include <pgmspace.h>

#include "Constants.h"
#include "Utils.h"


// Fixed-point colorspace conversion: HSV (hue-saturation-value) to RGB.
// This is a bit like the 'Wheel' function from the original strandtest
// code on steroids.  The angular units for the hue parameter may seem a
// bit odd: there are 1536 increments around the full color wheel here --
// not degrees, radians, gradians or any other conventional unit I'm
// aware of.  These units make the conversion code simpler/faster, because
// the wheel can be divided into six sections of 256 values each, very
// easy to handle on an 8-bit microcontroller.  Math is math, and the
// rendering code elsehwere in this file was written to be aware of these
// units.  Saturation and value (brightness) range from 0 to 255.
long hsv2rgb(long h, byte s, byte v) {
  byte r, g, b, lo;
  int  s1;
  long v1;

  // Hue
  h %= 1536;           // -1535 to +1535
  if(h < 0) h += 1536; //     0 to +1535
  lo = h & 255;        // Low byte  = primary/secondary color mix
  switch(h >> 8) {     // High byte = sextant of colorwheel
    case 0 : r = 255     ; g =  lo     ; b =   0     ; break; // R to Y
    case 1 : r = 255 - lo; g = 255     ; b =   0     ; break; // Y to G
    case 2 : r =   0     ; g = 255     ; b =  lo     ; break; // G to C
    case 3 : r =   0     ; g = 255 - lo; b = 255     ; break; // C to B
    case 4 : r =  lo     ; g =   0     ; b = 255     ; break; // B to M
    default: r = 255     ; g =   0     ; b = 255 - lo; break; // M to R
  }

  // Saturation: add 1 so range is 1 to 256, allowig a quick shift operation
  // on the result rather than a costly divide, while the type upgrade to int
  // avoids repeated type conversions in both directions.
  s1 = s + 1;
  r = 255 - (((255 - r) * s1) >> 8);
  g = 255 - (((255 - g) * s1) >> 8);
  b = 255 - (((255 - b) * s1) >> 8);

  // Value (brightness) and 24-bit color concat merged: similar to above, add
  // 1 to allow shifts, and upgrade to long makes other conversions implicit.
  v1 = v + 1;
  return (((r * v1) & 0xff00) << 8) |
          ((g * v1) & 0xff00)       |
         ( (b * v1)           >> 8);
}

char fixSin(int angle) {
  angle %= 720;               // -719 to +719
  if(angle < 0) angle += 720; //    0 to +719
  return (angle <= 360) ?
     pgm_read_byte(&sineTable[(angle <= 180) ?
       angle          : // Quadrant 1
      (360 - angle)]) : // Quadrant 2
    -pgm_read_byte(&sineTable[(angle <= 540) ?
      (angle - 360)   : // Quadrant 3
      (720 - angle)]) ; // Quadrant 4
}

char fixCos(int angle) {
  angle %= 720;               // -719 to +719
  if(angle < 0) angle += 720; //    0 to +719
  return (angle <= 360) ?
    ((angle <= 180) ?  pgm_read_byte(&sineTable[180 - angle])  : // Quad 1
                      -pgm_read_byte(&sineTable[angle - 180])) : // Quad 2
    ((angle <= 540) ? -pgm_read_byte(&sineTable[540 - angle])  : // Quad 3
                       pgm_read_byte(&sineTable[angle - 540])) ; // Quad 4
}

// Set range of pixels
void setRangeColor(byte idx, byte imgData[2][numPixels * 3],
                   const uint8_t *range, uint8_t r, uint8_t g, uint8_t b) {
  int i, begin = pgm_read_byte(&range[0]), end = pgm_read_byte(&range[1]), length = end - begin;
  byte *ptr = &imgData[idx][begin*3];
  for(i=0; i<=length; i++) {
    *ptr++ = r; *ptr++ = g; *ptr++ = b;
  }
}

// Set group of pixels
void setGroupColor(byte idx, byte imgData[2][numPixels * 3],
                   const uint8_t *group, int length, uint8_t r, uint8_t g, uint8_t b) {
  int i;
  byte *ptr;
  for(i=0; i<length; i++) {
    ptr = &imgData[idx][pgm_read_byte(&group[i])*3];
    *ptr++ = r; *ptr++ = g; *ptr++ = b;
  }
}

// Set range of pixels (alpha)
void setRangeAlpha(byte buffer[numPixels], const uint8_t *range, uint8_t a) {
  int i, begin = pgm_read_byte(&range[0]), end = pgm_read_byte(&range[1]);
  for(i=begin; i<=end; i++)
    buffer[i] = a;
}

// Set group of pixels (alpha)
void setGroupAlpha(byte buffer[numPixels], const uint8_t *group, int length,
									 uint8_t a) {
  int i;
  for(i=0; i<length; i++)
    buffer[pgm_read_byte(&group[i])] = a;
}

#endif
