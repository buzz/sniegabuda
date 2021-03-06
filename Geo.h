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
static const uint8_t line2Length = 11;

static const uint8_t PROGMEM line3[] = {8,9,40,41,70,71,72,73,74,75,76,49,50,19,20};
static const uint8_t line3Length = 15;

static const uint8_t PROGMEM line4[] = {6,7,38,39,68,69,91,92,93,94,95,77,78,51,52,21,22};
static const uint8_t line4Length = 17;

static const uint8_t PROGMEM line5[] = {4,5,36,37,66,67,89,90,102,103,104,96,97,79,80,53,54,23,24};
static const uint8_t line5Length = 19;

static const uint8_t PROGMEM line6[] = {2,3,34,35,65,64,88,87,101,100,99,98,82,81,55,56,25,26};
static const uint8_t line6Length = 18;

static const uint8_t PROGMEM line7[] = {0,1,32,33,63,62,86,85,84,83,57,58,27,28};
static const uint8_t line7Length = 14;

static const uint8_t PROGMEM line8[] = {31,61,30,29,59,60};
static const uint8_t line8Length = 6;

// Leafs
static const uint8_t PROGMEM leaf1[] = {29,30,31,59,84,60,61,62,99,85,86,87,100,101,104,103,102,93};
static const uint8_t leaf1Length = 18;

static const uint8_t PROGMEM leaf2[] = {23,53,54,55,79,80,81,82,96,97,98,99,104,100,103,102,101,90};
static const uint8_t leaf2Length = 18;

static const uint8_t PROGMEM leaf3[] = {17,49,48,47,77,76,75,74,96,95,94,93,104,103,100,101,102,87};
static const uint8_t leaf3Length = 18;

static const uint8_t PROGMEM leaf4[] = {11,43,42,41,72,71,70,69,93,92,91,90,103,102,104,100,101,99};
static const uint8_t leaf4Length = 18;

static const uint8_t PROGMEM leaf5[] = {96,103,104,100,102,101,90,89,88,87,67,66,65,64,37,36,35,5};
static const uint8_t leaf5Length = 18;



#endif
