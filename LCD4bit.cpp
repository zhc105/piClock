#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "LCD4bit.h"

LCD4bit::LCD4bit()
{
	pin_data[0] = 0;
	pin_data[1] = 1;
	pin_data[2] = 2;
	pin_data[3] = 3;
	pin_e = 5;
	pin_rs = 6;
	pin_led = 7;	
}

LCD4bit::LCD4bit(int rs, int e, int data[4], int led)
{
	pin_rs = rs;
	pin_e = e;
	pin_led = led;
	memcpy(pin_data, data, sizeof(int) * 4);
}

int LCD4bit::Init()
{
	/* initalize pin */
	if ( -1 == wiringPiSetup() )
	{
		printf("Setup wiringPi failed!");
		return 1;
	}

	pinMode(pin_e, OUTPUT);
	pinMode(pin_rs, OUTPUT);
	pinMode(pin_led, OUTPUT);
	for (int i = 0; i < 4; i++)
		pinMode(pin_data[i], OUTPUT);

	/* initialize LCD1602 */
	digitalWrite(pin_e, 1);		
	write4bits(0x3, 0);
	usleep(50);
	write4bits(0x3, 0);
	usleep(50);
	write4bits(0x3, 0);
	usleep(50);
	write4bits(0x2, 0);	// 4-bit mode
	usleep(50);
	write8bits(0x6, 0);
	usleep(50);
	write8bits(0x0C, 0);	// start LCD
	usleep(50);
	Clear();		// clear screen

	return 0;
}

void LCD4bit::Clear()
{
	write8bits(0x01, 0);
	usleep(2000);
}

void LCD4bit::Print(int line, int offset, const char *msg)
{
	if (offset > 15 || line > 1)
		return;
	char buf[17];
	memset(buf, 0x20, sizeof(buf));
	int len = strnlen(msg, 16 - offset);
	int cmd = 0x80 + line * 0x40 + offset;
	write8bits(cmd, 0);
	usleep(50);

	memcpy(buf, msg, len);
	for (int i = 0; i < 16; i++)
	{
		write8bits(buf[i], 1);
		usleep(50);
	}
}

void LCD4bit::PrintChar(int line, int offset, char c)
{
	if (offset > 15 || line > 1)
		return;
	int cmd = 0x80 + line * 0x40 + offset;
	write8bits(cmd, 0);
	usleep(50);
	write8bits(c, 1);
	usleep(50);
}

void LCD4bit::PulseEnable()
{
	digitalWrite(pin_e, 0);
	usleep(1);
	digitalWrite(pin_e, 1);
	usleep(1);
}

void LCD4bit::write4bits(int data, int rs)
{
	for (int i = 0; i < 4; i++)
	{
		digitalWrite(pin_data[i], data & 1);
		data >>= 1;
	}
	digitalWrite(pin_rs, rs);
	PulseEnable();
}

void LCD4bit::write8bits(int data, int rs)
{
	write4bits(data >> 4, rs);
	write4bits(data & 0xF, rs);
}

void LCD4bit::Backlight(int value)
{
	digitalWrite(pin_led, value);
}
