// -*- mode: C++ -*-
// For board "Arduino Mega ADK"

#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <math.h>
#include "LedMatrix.h"
#include "PushButton.h"
#include "Sound.h"
#include "Clock.h"
#include "Message.h"

#define LED_CONTROL_PIN 2
#define SERIAL_RX_PIN 10
#define SERIAL_TX_PIN 11
#define SOUND_PIN 23
#define START_BUTTON_PIN 24
#define STOP_BUTTON_PIN 25
#define SWITCH_AB_CD_ABC_BUTTON_PIN 26
#define SINGLE_DOUBLE_MODE_BUTTON_PIN 32
#define INCREASE_TIME_BUTTON_PIN 30
#define DECREASE_TIME_BUTTON_PIN 28

#define SOUND_ON          LOW
#define SOUND_OFF         HIGH

#define SIZE_X  32
#define SIZE_Y  8
#define NUM_LEDS SIZE_X*SIZE_Y

#define PERIODIC_PULSE_INTERVAL 1000

enum State {
  INACTIVE, RED_1, GREEN_1, YELLOW_1, RED_2, GREEN_2, YELLOW_2
};

State _state = INACTIVE;
State _prevState = INACTIVE;

enum SignalEvent {
  NO_SIGNAL, START, STOP
} _signalEvent;

enum TimeEvent {
  NO_TIME_EVENT, WAIT_END, GREEN_TIME_END, SHOOT_TIME_END
} _timeEvent;

enum DiscreteEvent {
  NO_EVENT, START_EVENT, STOP_EVENT, SWITCH_ABCD_EVENT,
  SINGLE_DOUBLE_EVENT, INCREASE_TIME_EVENT, DECREASE_TIME_EVENT
} _discreteEvent;

enum Abcd {
  AB = 0, CD = 1, NO_ABCD = 2
};

enum Abcd _abcd;
enum Abcd _prevAbcd;

enum RoundType {
  SINGLE, DOUBLE
} _roundType;

bool _isCounting;

struct DisplaySettings {
  int ledIntensity = 96;
  int waitNumberHue = 200;
  int shootNumberHue = 200;
  int abcdHue = 170;
  float numbersXPos = 10.0;
  int hueRed = 0;
  int hueGreen = 120;
  int hueYellow = 60;
} _displaySettings;

int _greenTimeSeconds = 90;
int _waitTimeSeconds = 10;
int _yellowTimeSeconds = 30;

SoftwareSerial _serial1(SERIAL_RX_PIN, SERIAL_TX_PIN);

int _bytesReceivedCounter = 0;
int _numOfBytesReceived = 0;

CRGB _leds[NUM_LEDS];
LEDMatrix* _ledMatrixP;

PushButton* _startButtonP;
PushButton* _stopButtonP;
PushButton* _switchAB_CD_ABC_ButtonP;
PushButton* _singleDoubleModeButtonP;
PushButton* _increaseTimeButtonP;
PushButton* _decreaseTimeButtonP;

Sound* _soundP;
Clock* _clockP;
Message* _messageP;

bool _settingsChanged = false;

void setup() {
  Serial.begin(115200);
  _serial1.begin(9600);
  Serial.println("Start ArcheryWifiLedMatrix");

  _startButtonP = new PushButton(START_BUTTON_PIN, 200);
  _stopButtonP = new PushButton(STOP_BUTTON_PIN, 200);
  _switchAB_CD_ABC_ButtonP = new PushButton(SWITCH_AB_CD_ABC_BUTTON_PIN, 200);
  _singleDoubleModeButtonP = new PushButton(SINGLE_DOUBLE_MODE_BUTTON_PIN, 200);
  _increaseTimeButtonP = new PushButton(INCREASE_TIME_BUTTON_PIN, 200);
  _decreaseTimeButtonP = new PushButton(DECREASE_TIME_BUTTON_PIN, 200);

  _clockP = new Clock();
  _soundP = new Sound(SOUND_PIN, SOUND_OFF, SOUND_ON, 800);
  _messageP = new Message(&_serial1);

  delay(500);

  FastLED.addLeds<NEOPIXEL, LED_CONTROL_PIN>(_leds, NUM_LEDS, 0);
  FastLED.clear();
  FastLED.show();

  _ledMatrixP = new LEDMatrix(SIZE_X, SIZE_Y, _leds);
  _ledMatrixP->setScroll(false);

  _state = INACTIVE;
  _roundType = DOUBLE;
  _abcd = AB;

  showCountdown();
  showABCDStatus();
  showStatusLEDs();
  showColorStatus();

  // Silent mode if stop button is kept down
  _soundP->setEnabled(!_stopButtonP -> isPushed(millis()));
}

void loop() {
  _prevState = _state;
  _clockP->update(_state != INACTIVE);

  bool gotNewMessage = _messageP->receiveByteSerialCom();
  if (gotNewMessage) {
    Serial.println("New message received: ");
    Serial.println((char*) _messageP->getMessage());
  }

  findTimeEvent();
  changeStateByTimeEvent();

  findDiscreteEvent(gotNewMessage);
  changeStateByDiscreteEvent();

  if (_clockP->secondCounterChanged()) {
    showCountdown();
  }
  if (_abcd != _prevAbcd) {
    showABCDStatus();
    showStatusLEDs();
  }
  if (_state != _prevState || _settingsChanged) {
    showColorStatus();
    showABCDStatus();
    showCountdown();
    showStatusLEDs();
  }

  _soundP->update(_clockP->getT());

  _prevAbcd = _abcd;
  _prevState = _state;
  _settingsChanged = false;
}

void showColorStatus() {
  switch (_state) {
    case INACTIVE:
      redLightOn();
      break;
    case RED_1:
    case RED_2:
      redLightOn();
      break;
    case YELLOW_1:
    case YELLOW_2:
      yellowLightOn();
      break;
    case GREEN_1:
    case GREEN_2:
      greenLightOn();
      break;
  }
}


void showABCDStatus() {
  if (_abcd == AB) {
    char a[2];
    a[0] = 28;
    a[1] = '\0';
    char b[2];
    b[0] = 29;
    b[1] = '\0';
    _ledMatrixP->showTextHSV(a, 0.0, _displaySettings.abcdHue, 255, _displaySettings.ledIntensity);
    _ledMatrixP->updateText();
    _ledMatrixP->showTextHSV(b, 5.0, _displaySettings.abcdHue, 255, _displaySettings.ledIntensity);
    _ledMatrixP->updateText();
  } else if (_abcd == CD) {
    char c[2];
    c[0] = 30;
    c[1] = '\0';
    char d[2];
    d[0] = 31;
    d[1] = '\0';
    _ledMatrixP->showTextHSV(c, 0.0, _displaySettings.abcdHue, 255, _displaySettings.ledIntensity);
    _ledMatrixP->updateText();
    _ledMatrixP->showTextHSV(d, 5.0, _displaySettings.abcdHue, 255, _displaySettings.ledIntensity);
    _ledMatrixP->updateText();
  } else {
    _ledMatrixP -> drawRectangleHSV(0, 0, 10, 8, 0, 0, 0);
  }
  FastLED.show();
}

void showStatusLEDs() {
  // Clear lower status row
  _ledMatrixP->drawRectangleHSV(0, 7, 28, 1, 0, 0, 0);
  if (_roundType == SINGLE) {
    _ledMatrixP->drawRectangleHSV(0, 7, 1, 1, 0, 255, _displaySettings.ledIntensity / 2);
  } else {
    _ledMatrixP->drawRectangleHSV(0, 7, 2, 1, 0, 255, _displaySettings.ledIntensity / 2);
  }
  FastLED.show();
}

void showCountdown() {
  int counterSeconds;
  switch (_state) {
    case INACTIVE:
      counterSeconds = _greenTimeSeconds + _yellowTimeSeconds;
      break;
    case RED_1:
      counterSeconds = findWaitCounter();
      break;
    case GREEN_1:
    case YELLOW_1:
      counterSeconds = findShootCounter();
      break;
    case RED_2:
      counterSeconds = findWaitCounter();
      break;
    case GREEN_2:
    case YELLOW_2:
      counterSeconds = findShootCounter();
      break;
  }

  if (counterSeconds >= 0 && counterSeconds <= 240) {
    showRightAdjNumber(counterSeconds, 3, _displaySettings.numbersXPos, _displaySettings.shootNumberHue, 0, _displaySettings.ledIntensity);
    FastLED.show();
  }
}

void findTimeEvent() {
  if (_state == INACTIVE || !_clockP->secondCounterChanged()) {
    _timeEvent = NO_TIME_EVENT;
  }

  if (_clockP->getTimeRunningSeconds() == _waitTimeSeconds) {
    _timeEvent = WAIT_END;
  } else if (_clockP->getTimeRunningSeconds() == _waitTimeSeconds + _greenTimeSeconds) {
    _timeEvent = GREEN_TIME_END;
  } else if (_clockP->getTimeRunningSeconds() == _waitTimeSeconds + _greenTimeSeconds + _yellowTimeSeconds) {
    _timeEvent = SHOOT_TIME_END;
  } else {
    _timeEvent = NO_TIME_EVENT;
  }
}

void changeStateByTimeEvent() {
  if (_timeEvent == NO_TIME_EVENT) {
    return;
  }

  switch (_state) {
    case INACTIVE:
      break;
    case RED_1:
      if (_timeEvent == WAIT_END) {
        _state = GREEN_1;
        _soundP->start(1, _clockP->getT());
      }
      break;
    case GREEN_1:
      if (_timeEvent == GREEN_TIME_END) {
        _state = YELLOW_1;
      }
      break;
    case YELLOW_1:
      if (_timeEvent == SHOOT_TIME_END) {
        if (_roundType == DOUBLE) {
          _state = RED_2;
          if (_abcd == AB) {
            _abcd = CD;
          } else {
            _abcd = AB;
          }
          _soundP->start(2, _clockP->getT());
        } else {
          _state = INACTIVE;
          _soundP->start(3, _clockP->getT());
        }
      }
      break;
    case RED_2:
      if (_timeEvent == WAIT_END) {
        _state = GREEN_2;
        _soundP->start(1, _clockP->getT());
      }
      break;
    case GREEN_2:
      if (_timeEvent == GREEN_TIME_END) {
        _state = YELLOW_2;
      }
      break;
    case YELLOW_2:
      if (_timeEvent == SHOOT_TIME_END) {
        _state = INACTIVE;
        _soundP->start(3, _clockP->getT());
      }
  }

  if (_timeEvent == SHOOT_TIME_END) {
    _clockP->initiate();
  }
}

void findDiscreteEvent(bool gotMessage) {
  _discreteEvent = NO_EVENT;
  if (_startButtonP->isPushed(_clockP->getT())) {
    _discreteEvent = START_EVENT;
  }
  if (_stopButtonP->isPushed(_clockP->getT())) {
    Serial.println("STOP *****");
    _discreteEvent = STOP_EVENT;
  }
  if (_switchAB_CD_ABC_ButtonP->isPushed(_clockP->getT())) {
    _discreteEvent = SWITCH_ABCD_EVENT;
  }
  if (_singleDoubleModeButtonP->isPushed(_clockP->getT())) {
    _discreteEvent = SINGLE_DOUBLE_EVENT;
  }
  if (_increaseTimeButtonP->isPushed(_clockP->getT())) {
    _discreteEvent = INCREASE_TIME_EVENT;
  }
  if (_decreaseTimeButtonP->isPushed(_clockP->getT())) {
    _discreteEvent = DECREASE_TIME_EVENT;
  }

  if (!gotMessage) {
    return;
  }
  char* msg = (char*)(_messageP->getMessage());

  char messageType = msg[0];
  switch (messageType) {
    case 'T':
      if (_state == INACTIVE) {
        setGreenTime(msg);
      }
      _settingsChanged = true;
      break;
    case 'S':
      _discreteEvent = START_EVENT;
      break;
    case 'Z':
      //startSoundSignal(msg);
      break;
    case 'I':
      _discreteEvent = STOP_EVENT;
      break;
  }
}

void changeStateByDiscreteEvent() {
  bool startEvent = _discreteEvent == START_EVENT;
  bool stopEvent = _discreteEvent == STOP_EVENT;
  if (stopEvent) {
    Serial.println("Stop event");
  }
  switch (_state) {
    case INACTIVE:
      if (startEvent) {
        _state = RED_1;
        _soundP->start(2, _clockP->getT());
      } else if (_discreteEvent == SWITCH_ABCD_EVENT) {
        rotateAbcd();
      } else if (_discreteEvent == SINGLE_DOUBLE_EVENT) {
        switchSingleDouble();
        _settingsChanged = true;
      } else if (_discreteEvent == INCREASE_TIME_EVENT) {
        increaseGreenTime();
        _settingsChanged = true;
      } else if (_discreteEvent == DECREASE_TIME_EVENT) {
        decreaseGreenTime();
        _settingsChanged = true;
      }
      break;
    case RED_1:
      break;
    case GREEN_1:
    case YELLOW_1:
      if (stopEvent) {
        if (_roundType == SINGLE) {
          _state = INACTIVE;
          _soundP->start(3, _clockP->getT());
          _clockP->initiate();
        } else {
          _state = RED_2;
          _soundP->start(2, _clockP->getT());
          _clockP->initiate();
          if (_abcd == AB) {
            _abcd = CD;
          } else {
            _abcd = AB;
          }
        }
      }
      break;
    case RED_2:
      break;
    case GREEN_2:
    case YELLOW_2:
      if (stopEvent) {
        _state = INACTIVE;
        _soundP->start(3, _clockP->getT());
        _clockP->initiate();
      }
  }
}

int findWaitCounter() {
  int t = _clockP->getTimeRunningSequence() / 1000;
  return _waitTimeSeconds - t;
}

int findShootCounter() {
  int t = _clockP->getTimeRunningSequence() / 1000;
  return _waitTimeSeconds + _greenTimeSeconds + _yellowTimeSeconds - t;
}

char text[10];
void showRightAdjNumber(int number, int numPlaces, float numbersXPos, int hue, int sat, int val) {
  number = min(pow(10, numPlaces) - 1, number);

  for (int i = 0; i < numPlaces; i++) {
    text[i] = ' ';
  }
  text[numPlaces] = '\0';

  int numDigits = (int)log10(number) + 1;
  int numLeadingBlanks = numPlaces - numDigits;
  numLeadingBlanks = max(0, numLeadingBlanks);
  numLeadingBlanks = min(numPlaces - 1, numLeadingBlanks);

  sprintf(&text[numLeadingBlanks], "%d", number);

  _ledMatrixP->showTextHSV(text, numbersXPos, hue, sat, val);
  _ledMatrixP->updateText();
  FastLED.show();
}

void greenLightOn() {
  _ledMatrixP -> drawRectangleHSV(28, 0, 4, 8, 0, 0, 0);
  _ledMatrixP -> drawRectangleHSV(28, 0, 4, 8, _displaySettings.hueGreen, 255, _displaySettings.ledIntensity);
  FastLED.show();
}

void yellowLightOn() {
  _ledMatrixP -> drawRectangleHSV(28, 0, 4, 8, 0, 0, 0);
  _ledMatrixP -> drawRectangleHSV(28, 0, 4, 8, _displaySettings.hueYellow, 255, _displaySettings.ledIntensity);
  FastLED.show();
}

void redLightOn() {
  _ledMatrixP -> drawRectangleHSV(28, 0, 4, 8, 0, 0, 0);
  _ledMatrixP -> drawRectangleHSV(28, 0, 4, 8, _displaySettings.hueRed, 255, _displaySettings.ledIntensity);
  FastLED.show();
}


void setGreenTime(char* msg) {
  _greenTimeSeconds = atoi(msg + 2) - 30;
}

void increaseGreenTime() {
  _greenTimeSeconds = min(240 - 30, _greenTimeSeconds + 40);
}

void decreaseGreenTime() {
  _greenTimeSeconds = max(40 - 30, _greenTimeSeconds - 40);
}

void rotateAbcd() {
  _abcd = (_abcd + 1) % 3;
}

void switchSingleDouble() {
  if (_roundType == SINGLE) {
    _roundType = DOUBLE;
  } else {
    _roundType = SINGLE;
  }
  _settingsChanged = true;
}
