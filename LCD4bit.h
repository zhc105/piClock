#ifndef LCD4BIT_H
#define LCD4BIT_H

#include <wiringPi.h>

class LCD4bit
{
private:
	int pin_rs;
	int pin_e;
	int pin_data[4];
	int pin_led;
	
private:
	void write4bits(int data, int rs);
	void write8bits(int data, int rs);
	void PulseEnable();

public:
	LCD4bit();
	LCD4bit(int rs, int e, int data[4], int led);

	int Init();
	void Clear();
	void Print(int line, int offset, const char *msg);
	void PrintChar(int line, int offset, char c);
	void Backlight(int value);

};

#endif
