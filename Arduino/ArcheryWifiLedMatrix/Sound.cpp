#include "Sound.h"
#include <Arduino.h>

Sound::Sound(int pin, int offValue, int onValue, unsigned long duration) {
  this->pin = pin;
  this->onValue = onValue;
  this->offValue = offValue;
  this->duration = duration;

  sounding = false;
  enabled = true;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, offValue);
}

void Sound::start(int noSignals, unsigned long currentTime) {
  sounding = true;
  this->noSignals = noSignals;
  startTime = currentTime;
}

void Sound::update(unsigned long currentTime) {
  sounding = sounding
             && (currentTime - startTime)
             < duration * (unsigned long)(2 * noSignals - 1);
  bool soundNow = sounding
                  && ((currentTime - startTime) / duration) % 2 == 0;
  if (enabled) {
    digitalWrite(pin, soundNow ? onValue : offValue);
  }
}

void Sound::setEnabled(bool enabled) {
  this->enabled = enabled;
}
