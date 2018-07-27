#include "ArduinoTracker.h"

ArduinoTracker::ArduinoTracker(char* portName)
{
    this->orientation = new double[4];
    this->portName = portName;
    this->arduino = new SerialPort(this->portName);

}

void ArduinoTracker::ReceiveData()
{
    double usLength = 0;
    double usRxLength = 0;

    usLength = arduino->readSerialPort(incomingData, MAX_DATA_LENGTH);

    //usLength = (UInt16)spSerialPort.Read(RxBuffer, usRxLength, 700);
    usRxLength += usLength;
    while (usRxLength >= 11)
    {
        /*memcpy_s(incomingDataTemp, 1000, incomingData, 1000);
        if (!((incomingDataTemp[0] == 0x55) & ((incomingDataTemp[1] & 0x50) == 0x50)))
        {
            for (int i = 1; i < usRxLength; i++) incomingData[i - 1] = incomingData[i];
            usRxLength--;
            continue;
        }
        if (((incomingDataTemp[0] + incomingDataTemp[1] + incomingDataTemp[2] + incomingDataTemp[3] + incomingDataTemp[4] + incomingDataTemp[5] + incomingDataTemp[6] + incomingDataTemp[7] + incomingDataTemp[8] + incomingDataTemp[9]) & 0xff) == incomingDataTemp[10])
        {
            this->DecodeData();
        }*/
        this->DecodeData();
        for (int i = 11; i < usRxLength; i++)
        {
            incomingData[i - 11] = incomingData[i];
        }
        usRxLength -= 11;
    }
    this->DecodeData();
    Sleep(10);
}

void ArduinoTracker::DecodeData()
{
    // If Incoming Data is indicating Angles
    if (incomingData[1] == 0x53)
    {
        orientation[0] = (short)(((short)(incomingData[3]) << 8) + (char)(incomingData[2]));
        orientation[1] = (short)(((short)(incomingData[5]) << 8) + (char)(incomingData[4]));
        orientation[2] = (short)(((short)(incomingData[7]) << 8) + (char)(incomingData[6]));

        // Convert to Degrees
        orientation[0] = orientation[0] / 32768.0 * 180;
        orientation[1] = orientation[1] / 32768.0 * 180;
        orientation[2] = orientation[2] / 32768.0 * 180;
        printf("%f %f %f\n", orientation[0], orientation[1], orientation[2]);
    }
}

ArduinoTracker::~ArduinoTracker()
{
    delete[] this->orientation;
}
