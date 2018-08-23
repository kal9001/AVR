#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "main.h"

void initOutput();

void drawSquare();
void drawMessageBuffer(uint8_t bufferNumber);
void drawOffsetLine(Line line, Coordinate offset);
void drawPoint(Coordinate coord);
void drawLine(Line line);
void drawArray(Line* lines, uint8_t size);



#endif /* OUTPUT_H_ */
