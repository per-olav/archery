#include "Clock.h"
#include <Arduino.h>

Clock::Clock() {
  initiate();
}

void Clock::initiate() {
  t = millis();
  tPrev = t;
  dt = 0;
  startTime = t;
  timeRunningSequence = 0;
  timeRunningSeconds = 0;
  timeRunningSecondsPrev = 0;
}

void Clock::update(bool isActive) {
  tPrev = t;
  t = millis();
  dt = t - tPrev;
  if (isActive) {
    timeRunningSequence += dt;
  }
  timeRunningSecondsPrev = timeRunningSeconds;
  timeRunningSeconds = timeRunningSequence / 1000;
}

unsigned long Clock::getT() {
  return t;
}

unsigned long Clock::getTimeRunningSequence() {
  return timeRunningSequence;
}

int Clock::getTimeRunningSeconds() {
  return timeRunningSeconds;
}

bool Clock::secondCounterChanged() {
  return timeRunningSeconds != timeRunningSecondsPrev;
}
