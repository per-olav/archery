#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

class PushButton {
	private:
		int pin;
		int minIntervalMs;
		unsigned long timeLastDown;
    bool prevDown;
    bool lastingDown;
    bool pushedFlank;
		
	public:
		PushButton(int pin, int minIntervalMs);
		bool isPushed(unsigned long timeNow);
};

#endif
