// -*- mode: C++ -*-
#include <Arduino.h>
#include "PushButton.h"

PushButton::PushButton(int pin, int minIntervalMs){
	this->pin = pin;
	this->minIntervalMs = minIntervalMs;
	pinMode(pin, INPUT_PULLUP);
  lastingDown = false;
  timeLastDown = (unsigned long)0;
}

bool PushButton::isPushed(unsigned long timeNow){
	bool downNow = digitalRead(pin) == LOW;
  if (downNow){
    timeLastDown = timeNow;
  }
  lastingDown = timeNow - timeLastDown < minIntervalMs && timeNow - timeLastDown > 0;
  if (lastingDown){
    //Serial.println("lastingDown");
    //Serial.println(pin);
  }
  
  pushedFlank = lastingDown && !prevDown;
  if (pushedFlank){
    Serial.println("pushedFlank");
    Serial.println(pin);
  }
  prevDown = lastingDown;
  return pushedFlank;
}
