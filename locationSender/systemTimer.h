/*
 * serial.h
 *
 * Created: 03/08/2018 14:04:49
 *  Author: KAL90
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

void initSerial();

void sendCommand(const char* message);
void sendCommand_P(const char* message);

void executeCommand();

#endif /* SERIAL_H_ */
