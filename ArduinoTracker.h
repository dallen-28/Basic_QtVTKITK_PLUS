#include "SerialPort.h"

#define MAX_DATA_LENGTH 1000

class ArduinoTracker
{
private:
    byte incomingData[MAX_DATA_LENGTH];
    byte incomingDataTemp[MAX_DATA_LENGTH];
    char* portName;
public:
    SerialPort *arduino;
    double *orientation;
    void ReceiveData();
    void DecodeData();
    ArduinoTracker(char*);
    ~ArduinoTracker();


};