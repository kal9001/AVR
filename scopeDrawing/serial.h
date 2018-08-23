#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>

uint8_t commandWaiting;

void initSerial(void);

void sendMessage_P(const char* message);
void sendMessage(const char* message);
void readCommand(void);

#endif /* SERIAL_H_ */
