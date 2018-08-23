#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <string.h>

#include "main.h"
#include "serial.h"

void removeCommand(void);
void insertCommand_P(const char* message);

void modeCommands();
void textCommands();
void shapeCommands();
void numberWords(uint8_t number);

typedef struct {
	char buffer[256];
	uint8_t head; //Offset in the buffer for any new incoming character to be inserted.
	uint8_t tail; //Offset in the buffer marks the oldest 'unused' character.
	uint8_t peek; //an offset to "peek" at a value without moving the tail.
} Buffer;

Buffer tx, rx;

/************************************************************************/
/* Initializes the serial hardware, tx.buffer and rx.buffer.            */
/************************************************************************/
void initSerial() {
	displayBuffer = 0;
	targetBuffer = 0;
	commandWaiting = 0;

	const char tmp[16] = {'H'-65, 'E'-65, 'L'-65, 'L'-65, 'O'-65, 26, 26, 26, 26, 26, 'W'-65, 'O'-65, 'R'-65, 'L'-65, 'D'-65, '\0' };
	memcpy(textBuffer[8], tmp, 16);

	tx.head = 0; tx.tail = 0 - 1;
	tx.peek = 0; //tx buffer doesn't actually use this (yet?)
	rx.head = 0; rx.tail = 0 - 1;
	rx.peek = rx.tail;

	memset(tx.buffer, 'X', 256);
	memset(rx.buffer, 'Y', 256);

	//insertCommand_P(PSTR("?tb1"));
	//insertCommand_P(PSTR("?twBUFFER      ONE"));

	//for debugging
   insertCommand_P(PSTR("?a"));
	insertCommand_P(PSTR("?h"));

	//set baud rate for 20Mhz
	UBRR0H = 0; 
	UBRR0L = 129;

	//set 8-bit data
	UCSR0C |= (3<<UCSZ00);

	//enable tx/rx pins, and rx interrupt. tx interrupt it enabled on demand
	UCSR0B |= (1<<TXEN0) | (1<<RXEN0) | (1<<RXCIE0);
}

/************************************************************************/
/* inserts a C-string type message to the tx.buffer.                    */
/* Replaces the null terminator with a newline character.               */
/************************************************************************/
void sendMessage_P(const char* message) {
	while(pgm_read_byte(message) != '\0') {
		tx.buffer[tx.head] = pgm_read_byte(message++);
		tx.head++;
	}
	tx.buffer[tx.head] = '\n';
	tx.head++;

	if(!(UCSR0B & (1<<TXCIE0))) {
		//seed the TX register
		UCSR0B |= (1<<TXCIE0);
		tx.tail++;
		UDR0 = tx.buffer[tx.tail];
	}
}

void sendMessage(const char* message) {
	while(*message != '\0') {
		tx.buffer[tx.head] = *message++;
		tx.head++;
	}
	tx.buffer[tx.head] = '\n';
	tx.head++;

	if(!(UCSR0B & (1<<TXCIE0))) {
		//seed the TX register
		UCSR0B |= (1<<TXCIE0);
		tx.tail++;
		UDR0 = tx.buffer[tx.tail];
	}
}

/************************************************************************/
/* Removes the oldest command from the rx.buffer.                       */
/* Used after a command is executed to wipe it from the buffer.         */
/************************************************************************/
void removeCommand() {
	while(rx.buffer[rx.tail + 1] != '\0') {
		rx.tail++;
		rx.buffer[rx.tail] = '\0';
	}
	rx.tail++; //to skip the null terminator of the previous command.
	commandWaiting--;
	rx.peek = rx.tail;
	return;
}

/************************************************************************/
/* Inserts a command to the rx.buffer.                                  */
/************************************************************************/
void insertCommand_P(const char* message) {
	while(pgm_read_byte(message) != '\0') {
		rx.buffer[rx.head] = pgm_read_byte(message++);
		rx.head++;
	}
	rx.buffer[rx.head] = '\0';
	rx.head++;
	commandWaiting++;
}

/************************************************************************/
/* Reads and decodes commands from the rx.buffer.                       */
/************************************************************************/
void readCommand() {
	if(commandWaiting == 0) return;

	rx.peek++;//1
	if(rx.buffer[rx.peek] == '?') {
		rx.peek++;//2
		switch(rx.buffer[rx.peek]) {
			case '\0': case ' ':
				sendMessage_P(PSTR("Okay")); break;

			case 'a': sendMessage_P(PSTR("ScopeDrawing 0.2, kal9001, 11/06/18.")); break;
			case 'h': sendMessage_P(PSTR("h-help, a-about, m-mode, t-text, s-shape.")); break;
			case 'm':  modeCommands(); break;
			case 't':  textCommands(); break;
			case 's': shapeCommands();	break;
			default:  sendMessage_P(PSTR("Command not understood")); break;
		}
	} else {
		sendMessage_P(PSTR("Command invalid."));
	}
	removeCommand();
}

/************************************************************************/
/* Mode command handler.                                                */
/************************************************************************/
void modeCommands(char* command) {
	rx.peek++;//3
	switch(rx.buffer[rx.peek]) {
		case '\0': case ' ':
			sendMessage_P(PSTR("System mode:"));
			switch(systemMode) {
				case TEXT_SINGLE: sendMessage_P(PSTR("Text-Single")); break;
				case TEXT_MULTI:	sendMessage_P(PSTR("Text-Multi"));	break;
				case HELLO:			sendMessage_P(PSTR("Hello"));			break;
				case SQUARE:		sendMessage_P(PSTR("Square"));		break;
				case CIRCLE:		sendMessage_P(PSTR("Circle"));		break;
				case SWITCH:		sendMessage_P(PSTR("Switch"));		break;
			}
			break;
		case 't': systemMode = TEXT_SINGLE; break;
		case 'm': systemMode = TEXT_MULTI;	break;
		case 'e': systemMode = HELLO;			break;
		case 's': systemMode = SQUARE;		break;
		case 'c': systemMode = CIRCLE;		break;
		case 'w': systemMode = SWITCH;		break;
		case 'h': sendMessage_P(PSTR("none-mode, t-text, e-hello, s-square, c-circle, w-switch, h-help"));
	}
}

/************************************************************************/
/* Text command hander.                                                 */
/************************************************************************/
void textCommands(char* command) {
	char tmpString[16] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0' };
	uint8_t index = 0;
	char character;

	rx.peek++;//3
	switch(rx.buffer[rx.peek]) {

		case '\0': case ' ': //Return text in select buffer
			while(index <= 15) {
				tmpString[index] = (textBuffer[targetBuffer][index] + 65);
				index++;
			}
			sendMessage(tmpString);
			break;

		case 'w': //Text input to select buffer
			rx.peek++;//4
			for(int i = 0; i < 15; i++) {
				character = rx.buffer[rx.peek];
				if(character == '\0') {
					textBuffer[targetBuffer][i] = 26;
				} else if(character == ' ') {
					textBuffer[targetBuffer][i] = 26;
					rx.peek++;//{5:15}
				} else if(character >= 'A' && character <= 'Z') {
					textBuffer[targetBuffer][i] = (character -= 'A');
					rx.peek++;//{5:15}
				} else if(character >= 'a' && character <= 'z') {
					textBuffer[targetBuffer][i] = (character -= 'a');
					rx.peek++;//{5:15}
				}
			}
			break;

		case 's': //return value of return buffer
			sendMessage_P(PSTR("Select buffer for IO is #:"));
			numberWords(targetBuffer);
			break;

		case 'i': //return value of display buffer
			sendMessage_P(PSTR("Display buffer for IO is #:"));
			numberWords(displayBuffer);
			break;

		case 'b': //select buffer for IO commands
			rx.peek++;//4
			if(rx.buffer[rx.peek] >= '1' && rx.buffer[rx.peek] <= '8') {
				targetBuffer = rx.buffer[rx.peek] -= '1';
			}
			break;

		case 'd': //display buffer
			rx.peek++;//4
			if(rx.buffer[rx.peek] >= '1' && rx.buffer[rx.peek] <= '8') {
				targetBuffer = rx.buffer[rx.peek] -= '1';
			}
			break;
		
		case 'h':
			sendMessage_P(PSTR("none-read text, w-write text, s-read select, i-read display, b-select, d-display, h-help."));
			break;

		default:
			sendMessage_P(PSTR("Unrecognized command."));
			break;
	}
}

/************************************************************************/
/* Shape command hander.                                                */
/************************************************************************/
void shapeCommands() {

}

void numberWords(uint8_t number) {
	switch(number) {
		case 0: sendMessage_P(PSTR("One")); break;
		case 1: sendMessage_P(PSTR("Two")); break;
		case 2: sendMessage_P(PSTR("Three")); break;
		case 3: sendMessage_P(PSTR("Four")); break;
		case 4: sendMessage_P(PSTR("Five")); break;
		case 5: sendMessage_P(PSTR("Six")); break;
		case 6: sendMessage_P(PSTR("Seven")); break;
		case 7: sendMessage_P(PSTR("Eight")); break;
	}
}

/************************************************************************/
/* Receive interrupt service routine.                                   */
/* Replaces incoming newline characters with null terminators.				*/
/* increments commandWaiting if a complete line is received             */
/************************************************************************/
ISR(USART_RX_vect) {
	rx.buffer[rx.head] = UDR0;
	if(rx.buffer[rx.head] == '\r') {
		rx.buffer[rx.head] = '\0';
		commandWaiting++;
	}
	rx.head++;
	return;
}

/************************************************************************/
/* Transmit interrupt service routine.                                  */
/************************************************************************/
ISR(USART_TX_vect) {
	//disable TX interrupt if the transmit buffer is empty.
	if(tx.tail + 1 == tx.head) {
		UCSR0B &= ~(1<<TXCIE0 );
	} else {
		//jump forward and send the next char.
		tx.tail++;
		UDR0 = tx.buffer[tx.tail];		
	}
	return;
}

ISR(USART_UDRE_vect) {} //Don't think I need to use this at the moment
