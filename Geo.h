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

static const uint8_t PROGMEM starBig[] = {
	1,2,3,7,8,9,13,14,15,19,20,21,25,26,27,31,32,33,34,35,37,38,39,40,41,43,44,45,
	46,47,49,50,51,52,53,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,
	74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,
	100,101,102,103,104
};
static const uint8_t starBigLength = 85;

static const uint8_t PROGMEM pentCenter[] = {100,101,102,103,104};
static const uint8_t pentCenterLength = 5;

static const uint8_t PROGMEM pent1[] = {41,42,43,70,71};
static const uint8_t pent1Length = 5;

static const uint8_t PROGMEM pent2[] = {35,36,37,65,66};
static const uint8_t pent2Length = 5;

static const uint8_t PROGMEM pent3[] = {30,31,59,60,61};
static const uint8_t pent3Length = 5;

static const uint8_t PROGMEM pent4[] = {53,54,55,80,81};
static const uint8_t pent4Length = 5;

static const uint8_t PROGMEM pent5[] = {47,48,49,75,76};
static const uint8_t pent5Length = 5;

#endif
