#include "Message.h"
#include <SoftwareSerial.h>
#include <math.h>
#include <Arduino.h>

Message::Message(SoftwareSerial* serialP) {
  this->serialP = serialP;
  messageBuffer[0] = '\0';
  numOfBytesReceived = 0;
  bytesReceivedCounter = 0;
  messageComplete = false;
  bytesReceivedCounter = 0;
  timeLastReceived = 0;
}

bool Message::receiveByteSerialCom() {
  if (bytesReceivedCounter == 0) {
    numOfBytesReceived = 0;
  }
  bool messageComplete = false;
  if (serialP->available()) {
    uint8_t c = (uint8_t)serialP->read();
    unsigned long t = millis();

    Serial.println(t - timeLastReceived);
    if (t - timeLastReceived > 500) {
      bytesReceivedCounter = 0;
      numOfBytesReceived = 0;
      for (int i = 0; i < MAX_MESSAGE_LENGTH + 1; i++) {
        messageBuffer[i] = '\0';
      }
    }
    timeLastReceived = t;

    ++bytesReceivedCounter;
    if (bytesReceivedCounter > MAX_MESSAGE_LENGTH) {
      bytesReceivedCounter = MAX_MESSAGE_LENGTH;
    }
    numOfBytesReceived = bytesReceivedCounter;
    messageBuffer[bytesReceivedCounter - 1] = c;
    messageBuffer[bytesReceivedCounter] = '\0';
    messageComplete = (c == '\0' || c == '\n');
    bytesReceivedCounter = messageComplete ? 0 : bytesReceivedCounter;
    Serial.println((char*)messageBuffer);
    Serial.println(bytesReceivedCounter);
  }
  return messageComplete;
}

uint8_t* Message::getMessage() {
  return messageBuffer;
}
