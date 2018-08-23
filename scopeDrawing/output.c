#include <avr/io.h>

#define F_CPU 20000000

#include <util/delay.h>
#include "serial.h"


#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>

#include "output.h"
#include "charMap.h"

const uint8_t DAC_CS = PORTB2;

void sendCoordinate(Coordinate c);

uint8_t outputData[4] = { 0, 0, 0, 0};
uint8_t byteCount;

/************************************************************************/
/* initializes the SPI hardware, and other output variables.            */
/************************************************************************/
void initOutput() {
	//set SS, MOST and SCK as outputs.
	DDRB |= (1<<PORTB2) | (1<<PORTB3) | (1<<PORTB5);
	//set SS high.
	PORTB |= (1<<DAC_CS);

	//enable SPI, SPI-interrupt, set to MSB first, and set SPI-master.
	SPCR |= (1<<SPIE) | (1<<SPE) | (1<<MSTR);
	SPSR |= (1<<SPI2X);
}

void drawScopeCharacter(uint8_t character, Coordinate location) {
	for(int i = 0; i < scopeChars[character].size; i++) {
		drawOffsetLine(scopeChars[character].lines[i], location);
	}
}

void drawSquare() {
	drawLine((Line){{ 64,  64}, {  64, 192}});
	drawLine((Line){{ 64, 192}, { 192, 192}});
	drawLine((Line){{192, 192}, { 192,  64}});
	drawLine((Line){{192,  64}, {  64,  64}});
}

void drawMessageBuffer(uint8_t bufferNumber) {
	uint8_t charIndex = 0;
	Coordinate offset;

	uint8_t row = 2;
	uint8_t col = 0;

	for(;;) {
		offset.x = 16 + (col * 48);
		offset.y = 16 + (row * 80);
		drawScopeCharacter(textBuffer[bufferNumber][charIndex], offset);
		col++;
		if(col > 4) {
			if(row == 0) { break; }
			col = 0; row--;
		}
		charIndex++;
	}
}

void drawOffsetLine(Line lineToDraw, Coordinate offset) {
	lineToDraw.start.x += offset.x;
	lineToDraw.start.y += offset.y;
	lineToDraw.end.x += offset.x;
	lineToDraw.end.y += offset.y;
	drawLine(lineToDraw);
}

void drawLine(Line LineToDraw) {
	uint8_t differenceX = abs(LineToDraw.end.x - LineToDraw.start.x);
	uint8_t differenceY = abs(LineToDraw.end.y - LineToDraw.start.y);

	int8_t changeX = LineToDraw.start.x < LineToDraw.end.x ? 1 : -1;
	int8_t changeY = LineToDraw.start.y < LineToDraw.end.y ? 1 : -1;

	int16_t error = ( differenceX > differenceY ? differenceX : -differenceY) / 2;
	int16_t error2;

	for(;;) {
		sendCoordinate(LineToDraw.start);

		if(LineToDraw.start.x == LineToDraw.end.x && LineToDraw.start.y == LineToDraw.end.y) return;

		error2 = error;
		if(error2 > -differenceX) { error -= differenceY; LineToDraw.start.x += changeX;	}
		if(error2 <  differenceY) { error += differenceX; LineToDraw.start.y += changeY;	}
	}
}

void drawArray(Line* lines, uint8_t size) {

}

/************************************************************************/
/* Populates the outputData variable with Coordinate data, and triggers */
/* the SPI peripheral to begin transmission.                            */
/************************************************************************/
void sendCoordinate(Coordinate c) {
	//data frame for MCP4822 is:
	// channel select // unused // gain // shutdown // MSB first data 12bits //
	
	//while(SPCR & (1<<SPIE)) {};

	PORTB &= ~(1<<DAC_CS); //set SS low	
	SPDR = (0x90 + (c.x>>4));
	_delay_us(1);

	SPDR = (c.x<<4);
	_delay_us(1);
	PORTB |= (1<<DAC_CS);

	PORTB &= ~(1<<DAC_CS);
	SPDR = (0x10 + (c.y>>4));
	_delay_us(1);

	SPDR = (c.y<<4);
	_delay_us(1);
	PORTB |= (1<<DAC_CS);
	
	//SPCR |= (1<<SPIE); //enable SPI transaction complete interrupt.
	

	//fill SPI register.
	//SPDR = outputData[byteCount];
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
ISR(SPI_STC_vect) {
	switch(byteCount) {
	case 0: //finished sending 1st byte
		byteCount++;
		SPDR = outputData[byteCount];
		return;

	case 1: //finished sending 2nd byte
		PORTB |= (1<<DAC_CS); //set SS high.
		byteCount++;
		PORTB &= ~(1<<DAC_CS); //set SS low.
		SPDR = outputData[byteCount];
		return;

	case 2: //finished sending 3rd byte
		byteCount++;
		SPDR = outputData[byteCount];
		return;

	case 3://finished sending 4th byte
		SPCR &= ~(1<<SPIE); //turn off SPI interrupt after the last byte.
		PORTB |= (1<<DAC_CS); //set SS high.
		return;
	}
}
