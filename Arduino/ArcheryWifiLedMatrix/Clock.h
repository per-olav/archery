#ifndef CLOCK_H
#define CLOCK_H

class Clock {
	private:
		unsigned long dt;
		unsigned long tPrev;
		unsigned long t;
		unsigned long startTime;
		unsigned long timeRunningSequence;
		int timeRunningSeconds;
		int timeRunningSecondsPrev;
	public:
		Clock();
    void initiate();
    void update(bool isActive);
    unsigned long getT();
    unsigned long getTimeRunningSequence();
    int getTimeRunningSeconds();
    bool secondCounterChanged();
};

#endif
