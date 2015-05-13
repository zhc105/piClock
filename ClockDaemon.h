#ifndef CLOCKDAEMON_H
#define CLOCKDAEMON_H

#include "BMP085.h"
#include "DHTReader.h"
#include "LCD4bit.h"

#define UPDATE_STEPS 1800

class ClockDaemon
{
private:
	BMP085 *bmp085;
	CDHTReader *dht;
	LCD4bit *lcd;

private:
	double GetCPUTemp();
	void UpdateTemperature();

public:
	ClockDaemon();
	~ClockDaemon();

	int Start();

};

#endif
