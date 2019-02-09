#ifndef SOUND_H
#define SOUND_H

class Sound {
  private:
    int pin;
    bool enabled;
    int noSignals;
    unsigned long duration;
    bool sounding;
    unsigned long startTime = 0;
    int offValue;
    int onValue;
  public:
    Sound(int pin, int offValue, int onValue, unsigned long duration);
    void start(int noSignals, unsigned long currentTime);
    void update(unsigned long currentTime);
    void setEnabled(bool enable);
};

#endif
