#ifndef _DHT_READER_H_
#define _DHT_READER_H_

#include <wiringPi.h>

#define DHT_TIME_LIMIT 100000

#define DHT_TYPE_DHT11 0
#define DHT_TYPE_DHT22 1

#define DHT_ERROR_TIMEOUT		-12001
#define DHT_CHECKSUM_ERROR		-12002

class CDHTReader
{
private:
	int _pin;
	int _rnum;
	int _raw[100];
	int _type;
	unsigned int _data[5];

private:
	int read_byte(unsigned int &data);
	long long get_time();

public:
	CDHTReader(int pin, int type = 0);

	int init();
	int read(double &temp, double &humi);

};

#endif
