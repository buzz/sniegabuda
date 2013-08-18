#ifndef GEO_h
#define GEO_h

#include <pgmspace.h>

// Lines
static const uint8_t PROGMEM firstCircle[] = {0, 29};
static const uint8_t PROGMEM secondCircle[] = {30, 59};
static const uint8_t PROGMEM thirdCircle[] = {60,84};
static const uint8_t PROGMEM fourthCircle[] = {85,99};
static const uint8_t PROGMEM fifthCircle[] = {100,104};

// Groups
static const uint8_t PROGMEM starSmall[] = {87,90,93,96,99,100,101,102,103,104};
static const uint8_t starSmallLength = 10;

static const uint8_t PROGMEM starMiddle[] = {
  33,39,45,51,57,62,63,64,67,68,69,72,73,74,77,78,79,82,83,84,85,86,87,88,89,90,
	91,92,93,94,95,96,97,98,99,100,101,102,103,104};
static const uint8_t starMiddleLength = 40;

static const uint8_t PROGMEM Line1[] =
{0,29,30,59,60,84,85,99,100,104,103,93,92,72,71,43,42,11,10};
static const uint8_t Line1Length = 19;

#endif
