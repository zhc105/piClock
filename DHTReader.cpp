#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>
#include "DHTReader.h"

CDHTReader::CDHTReader(int pin, int type)
{
	_pin = pin;
	_type = type;
	_rnum = 0;
}

int CDHTReader::read_byte(unsigned int &data)
{
	int timeout, i, interval;
	long long tstart, tend;

	for (timeout = 0; digitalRead(_pin) == 1;timeout++)
	{
		delayMicroseconds(1);
		if (timeout >= DHT_TIME_LIMIT)
			return DHT_ERROR_TIMEOUT;
	}

	data = 0;
	_rnum = 0;
	for (i = 0; i < 8; i++)
	{
		tstart = get_time();
		for (timeout = 0; digitalRead(_pin) == 0;timeout++)
		{
			delayMicroseconds(1);
			if (timeout >= DHT_TIME_LIMIT)
				return DHT_ERROR_TIMEOUT;
		}
		tend = get_time();
		_raw[_rnum++] = (int)(tend - tstart);

		tstart = get_time();
		for (timeout = 0; digitalRead(_pin) == 1;timeout++)
		{
			delayMicroseconds(1);
			if (timeout >= DHT_TIME_LIMIT)
				return DHT_ERROR_TIMEOUT;
		}
		tend = get_time();
		_raw[_rnum++] = (int)(tend - tstart);
		interval = (int)(tend - tstart);
		data <<= 1;
		if (interval >= 50)
			data |= 1;
	}

	return 0;
}

long long CDHTReader::get_time()
{
	struct timeval tval;
	long ret;

	gettimeofday(&tval, NULL);
	ret =  tval.tv_sec;
	ret *= 1000000;
	ret += tval.tv_usec;
	return ret;
}

int CDHTReader::init()
{
	/* initalize pin */
	if ( -1 == wiringPiSetup() )
	{
		fprintf(stderr, "Setup wiringPi failed!");
		return 1;
	}
}

int CDHTReader::read(double &temp, double &humi)
{
	int i, timeout, waittime, ret = 0;

	// wait until DHT initialize
	pinMode(_pin, INPUT);
	timeout = 0;
	for(waittime = 0; waittime < 100; waittime++)
	{
		if (digitalRead(_pin) == 0)
			waittime = 0;
		if (++timeout >= 1000)
		{
			ret = DHT_ERROR_TIMEOUT;
			goto error_out;
		}
		delay(4);
	}

	/* send start signal */
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, 0);
	delay(20);

	/* wait for DHT start */
	pinMode(_pin, INPUT);
	for (timeout = 0; digitalRead(_pin) == 1; timeout++)
	{
		delayMicroseconds(1);
		if (timeout >= DHT_TIME_LIMIT)
		{
			ret = DHT_ERROR_TIMEOUT;
			goto error_out;
		}
	}
	for (timeout = 0; digitalRead(_pin) == 0; timeout++)
	{
		delayMicroseconds(1);
		if (timeout >= DHT_TIME_LIMIT)
		{
			ret = DHT_ERROR_TIMEOUT;
			goto error_out;
		}
	}

	for (i = 0; i < 5; i++)
		if (read_byte(_data[i]))
		{
			ret = DHT_ERROR_TIMEOUT;
			goto error_out;
		}
	if (_data[4] != ((_data[0] + _data[1] + _data[2] + _data[3]) & 0xFF))
	{
		ret = DHT_CHECKSUM_ERROR;
		goto error_out;
	}


	if (_type == DHT_TYPE_DHT11)
	{
		humi = (double) _data[0];
		temp = (double) _data[2];
	}
	else if (_type == DHT_TYPE_DHT22)
	{
		humi = (_data[0] << 8) + _data[1];
		humi /= 10;
		temp = ((_data[2] & 0x7F) << 8) + _data[3];
		temp /= 10;
		if (_data[2] & 0x80)
			temp = -temp;
	}

error_out:

	pinMode(_pin, INPUT);
	return ret;
}
