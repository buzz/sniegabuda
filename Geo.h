#ifndef GEO_h
#define GEO_h

#include <pgmspace.h>

// Rings
static const uint8_t PROGMEM ring1[] = {0, 29};
static const uint8_t PROGMEM ring2[] = {30, 59};
static const uint8_t PROGMEM ring3[] = {60,84};
static const uint8_t PROGMEM ring4[] = {85,99};
static const uint8_t PROGMEM ring5[] = {100,104};

// Stars
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

// Pentagons
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

// Lines
static const uint8_t PROGMEM line1[] = {12,13,14,15,16};
static const uint8_t line1Length = 5;

static const uint8_t PROGMEM line2[] = {10,11,42,43,44,45,46,47,48,17,18};
static const uint8_t lineLength = 11;

static const uint8_t PROGMEM line3[] = {8,9,40,41,70,71,72,73,74,75,76,49,50,19,20};
static const uint8_t lineLength = 15;

static const uint8_t PROGMEM line4[] = {6,7,38,29,68,69,91,92,93,94,95,77,78,51,52,21,22};
static const uint8_t lineLength = 17;

static const uint8_t PROGMEM line5[] = {4,5,36,37,66,67,89,90,102,103,104,96,97,79,80,53,53,23,24};
static const uint8_t lineLength = 19;

static const uint8_t PROGMEM line6[] = {2,3,34,35,65,64,88,87,101,100,99,98,82,81,55,56,25,26};
static const uint8_t lineLength = 18;

static const uint8_t PROGMEM line7[] = {0,1,32,33,63,62,86,85,84,83,57,58,27,28};
static const uint8_t lineLength = 14;

static const uint8_t PROGMEM line8[] = {31,61,30,29,59,60};
static const uint8_t lineLength = 6;

#endif
