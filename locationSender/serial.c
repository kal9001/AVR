/*\8
 * locationSender_C.c
 *
 * Created: 03/08/2018 13:46:37
 * Author : KAL90
\*/
/*\
 *   32k ROM, 1k EEPROM, 2k RAM, 8Mhz (internal).
 *
 *                     ATmega328P
 *                     .---_---.
 *          RESET/PC6 -|1    28|- PC5 - LCD_E
 *				 rx - PD0 -|2    27|- PC4 - LCD_RS
 *				 tx - PD1 -|3    26|- PC3 - LCD_D7
 *						PD2 -|4    25|- PC2 - LCD_D6
 *						PD3 -|5    24|- PC1 - LCD_D5
 *                PD4 -|6    23|- PC0 - LCD_D4
 *                VCC -|7    22|- GND
 *                GND -|8    21|- AREF
 *                PB6 -|9    20|- AVCC
 *                PB7 -|10   19|- PB5
 *                PD5 -|11   18|- PB4 
 *						PD6 -|12   17|- PB3
 *                PD7 -|13   16|- PB2
 *    ext clock - PB0 -|14   15|- PB1
 *                     `-------`
 *
\*/

#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>

#include "serial.h"
#include "lcd.h"
#include "systemTimer.h"

uint8_t screenRefreshFlag = 0;

int main(void)
{	
	OSCCAL = 0x92;
	DDRD = (1<<PORTD2) | (1<<PORTD3) | (1<<PORTD4);

	PORTD |= (1<<PORTD2);
	PORTD |= (1<<PORTD3);
	PORTD |= (1<<PORTD4);

	initSerial();
	initLCD();
	initSystemTimer(&screenRefreshFlag);
	
	sei();

   while (1) 
   {
		executeCommand();
					
		if(screenRefreshFlag != 0) { 
			updateDisplay();
			screenRefreshFlag = 0;
		}
   }
}

