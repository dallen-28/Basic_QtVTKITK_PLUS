/*
* Author: Manash Kumar Mandal
* Modified Library introduced in Arduino Playground which does not work
* This works perfectly
* LICENSE: MIT
*/


#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;

class SerialPort
{
private:
    HANDLE handler;
    bool connected;
    COMSTAT status;
    DWORD errors;

protected:
    static const int MAX_DATA_LENGTH1 = 255;
    static const int ARDUINO_WAIT_TIME = 2000;

public:
    SerialPort(char *portName);
    ~SerialPort();

    int readSerialPort(byte *buffer, unsigned int buf_size);
    bool writeSerialPort(char *buffer, unsigned int buf_size);
    bool isConnected();
};

#endif // SERIALPORT_H
