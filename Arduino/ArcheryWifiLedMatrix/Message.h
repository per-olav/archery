#ifndef MESSAGE_H
#define MESSAGE_H
#include <SoftwareSerial.h>

#define MAX_MESSAGE_LENGTH  32

class Message {
  private:
    SoftwareSerial* serialP;
    uint8_t messageBuffer[MAX_MESSAGE_LENGTH + 1];
    int bytesReceivedCounter;
    int numOfBytesReceived;
    bool messageComplete;
    unsigned long timeLastReceived;
  public:
    Message(SoftwareSerial* serialP);
    bool receiveByteSerialCom();
    uint8_t* getMessage();
};

#endif
