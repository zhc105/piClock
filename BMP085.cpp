#include <wiringPiI2C.h>
#include <unistd.h>
#include <cstdio>
#include <stdexcept>
#include "BMP085.h"

BMP085::BMP085()
{
	if ( (fd = wiringPiI2CSetup(BMP085DEVID)) == -1 )
	{
		printf("E: wiringPiI2CSetup failed! (Did you forget sudo?)\n");
		throw "wiringPiI2CSetup failed!";
	}
	ReadCalibration();
	mode = 1;
}

BMP085::BMP085(INT mode)
{
	if ( (fd = wiringPiI2CSetup(BMP085DEVID)) == -1 )
	{
		printf("E: wiringPiI2CSetup failed! (Did you forget sudo?)\n");
		throw "wiringPiI2CSetup failed!";
	}
	ReadCalibration();
	this->mode = mode;
}

INT BMP085::ReadI16(UINT address)
{
        CHAR msb = wiringPiI2CReadReg8(fd, address);
	UCHAR lsb = wiringPiI2CReadReg8(fd, address + 1);
	return ((INT) msb << 8) | lsb;
}

INT BMP085::ReadI16U(UINT address)
{
        UCHAR msb = wiringPiI2CReadReg8(fd, address);
	UCHAR lsb = wiringPiI2CReadReg8(fd, address + 1);
	return ((INT) msb << 8) | lsb;
}

INT BMP085::ReadRawTemp()
{
	wiringPiI2CWriteReg8(fd, 0xF4, 0x2E);
        usleep(5000);
	return ReadI16U(0xF6);
}

INT BMP085::ReadRawPressure()
{
	wiringPiI2CWriteReg8(fd, 0xF4, __BMP085_READPRESSURECMD + (mode << 6));
	if (mode == __BMP085_ULTRALOWPOWER)
		usleep(6000);
	else if (mode == __BMP085_HIGHRES)
		usleep(15000);
	else if (mode == __BMP085_ULTRAHIGHRES)
		usleep(27000);
	else
		usleep(9000);

	UINT msb  = wiringPiI2CReadReg8(fd, 0xF6);
	UINT lsb  = wiringPiI2CReadReg8(fd, 0xF7);
	UINT xlsb = wiringPiI2CReadReg8(fd, 0xF8);

	INT raw = ((msb << 16) + (lsb << 8) + xlsb) >> (8 - mode);
	return raw;
}

double BMP085::ReadTemperature()
{
        INT ut;
        LONG x1, x2, b5, b6;

        ut = ReadRawTemp();
        //printf("UT  = %d\n", ut);

        x1 = (((LONG) ut - (LONG) ac6) * (LONG) ac5) >> 15;
        x2 = ((LONG) mc << 11) / (x1 + md);
        b5 = x1 + x2;

	double temp = (double)((b5 + 8) >> 4) / 10.0;
	return temp;
}

double BMP085::ReadPressure()
{
	LONG x1, x2, x3, b3, b5, b6, p;
	ULONG b4, b7;

	INT ut = ReadRawTemp();
	INT up = ReadRawPressure();
	
	/* True Temperature Calculations */
	x1 = ((ut - ac6) * ac5) >> 15;
	x2 = (mc << 11) / (x1 + md);
	b5 = x1 + x2;

	/* Pressure Calculations */
	b6 = b5 - 4000;
	x1 = (b2 * (b6 * b6) >> 12) >> 11; 
	x2 = (ac2 * b6) >> 11;
	x3 = x1 + x2;
	b3 = (((ac1 * 4 + x3) << mode) + 2) / 4;

	x1 = (ac3 * b6) >> 13;
	x2 = (b1 * ((b6 * b6) >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (ac4 * (x3 + 32768)) >> 15;
	b7 = (up - b3) * (50000 >> mode);

	if (b7 < 0x80000000)
		p = (b7 * 2) / b4;
	else
		p = (b7 / b4) * 2;
	
	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;

	p = p + ((x1 + x2 + 3791) >> 4);

	return ((double)p / 100.0);
}

void BMP085::ReadCalibration()
{
	ac1 = ReadI16(0xAA);
        ac2 = ReadI16(0xAC);
        ac3 = ReadI16(0xAE);
        ac4 = ReadI16U(0xB0);
        ac5 = ReadI16U(0xB2);
        ac6 = ReadI16U(0xB4);
        b1 = ReadI16(0xB6);
        b2 = ReadI16(0xB8);
        mb = ReadI16(0xBA);
        mc = ReadI16(0xBC);
        md = ReadI16(0xBE);
        /*
        printf("AC1 = %d\n", ac1);
        printf("AC2 = %d\n", ac2);
        printf("AC3 = %d\n", ac3);
        printf("AC4 = %d\n", ac4);
        printf("AC5 = %d\n", ac5);
        printf("AC6 = %d\n", ac6);
        printf("B1  = %d\n", b1);
        printf("B2  = %d\n", b2);
        printf("MB  = %d\n", mb);
        printf("MC  = %d\n", mc);
        printf("MD  = %d\n", md);
        */
}
