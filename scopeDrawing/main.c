/*\
 *   32k ROM, 1k EEPROM, 2k RAM, 20Mhz.
 *
 *               ATmega328P
 *               .---_---.
 *    RESET/PC6 -|1    28|- PC5 - button 1
 *     rx - PD0 -|2    27|- PC4 - button 2
 *     tx - PD1 -|3    26|- PC3
 *          PD2 -|4    25|- PC2 - rightKnob
 *          PD3 -|5    24|- PC1 - centerKnob
 *          PD4 -|6    23|- PC0 - leftKnob
 *          VCC -|7    22|- GND
 *          GND -|8    21|- AREF
 *   XTAL - PB6 -|9    20|- AVCC
 *   XTAL - PB7 -|10   19|- PB5 - SCK
 *          PD5 -|11   18|- PB4 - MISO 
 *          PD6 -|12   17|- PB3 - MOSI
 *          PD7 -|13   16|- PB2 - DAC CS
 *          PB0 -|14   15|- PB1
 *               `-------`
 *
\*/

#define F_CPU 20000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "main.h"
#include "serial.h"
#include "output.h"

int main(void) {
	systemMode = HELLO;
	initSerial();
	//initInput();
	initOutput();
	sei(); //enable global interrupts
	
   while(1) {

		while(commandWaiting) {
			readCommand();
		}
		
		switch(systemMode) {
		case SWITCH:
			break;

		case TEXT_SINGLE:
			drawMessageBuffer(displayBuffer);
			break;

		case TEXT_MULTI:
			break;

		case SQUARE:
			drawSquare();
			break;

		case CIRCLE:
			break;

		case HELLO:
			drawMessageBuffer(3);
			break;
		}
   }
}
