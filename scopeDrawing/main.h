#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

typedef struct {
	uint8_t x, y;
} Coordinate;

typedef struct {
	Coordinate start, end;
} Line;

uint8_t displayBuffer;
uint8_t targetBuffer;
uint8_t textBuffer[9][16];

enum modes{TEXT_SINGLE, TEXT_MULTI , SQUARE, CIRCLE, HELLO, SWITCH} systemMode;

#endif /* MAIN_H_ */
