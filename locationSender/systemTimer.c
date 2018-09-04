/*
 * lcd.c
 *
 * Created: 13/08/2018 15:43:47
 *  Author: KAL90
 */ 

 #include <avr/io.h>
 #include "lcd.h"
 #include "hd44780.h"
 #include "hd44780_settings.h"

 #include "serial.h"

 void initLCD() {
	lcd_init();
 }

 void updateDisplay() {
				 
	lcd_goto(0);
	lcdLine1[16] = '\0';
	lcd_puts(lcdLine1);

	lcd_goto(40);
	lcdLine2[16] = '\0';
	lcd_puts(lcdLine2);
 }
