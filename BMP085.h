#ifndef BMP085_H
#define BMP085_H

#define BMP085DEVID 0x77

/* Operating Modes */
#define  __BMP085_ULTRALOWPOWER 0
#define  __BMP085_STANDARD      1
#define  __BMP085_HIGHRES       2
#define  __BMP085_ULTRAHIGHRES  3
#define __BMP085_READPRESSURECMD 0x34

typedef signed   int   INT;
typedef unsigned int   UINT;
typedef signed   long  LONG;
typedef unsigned long  ULONG;
typedef signed   short SHORT;
typedef unsigned short USHORT;
typedef signed   char  CHAR;
typedef unsigned char  UCHAR;

class BMP085
{
private:
	INT  mode;
	INT  fd;
	INT  ac1, ac2, ac3;
	UINT ac4, ac5, ac6;
	INT  b1, b2, mb, mc, md;

private:
	INT ReadI16(UINT address);
	INT ReadI16U(UINT address);
	INT ReadRawTemp();
	INT ReadRawPressure();
	void ReadCalibration();
	

public:
	BMP085();
	BMP085(INT mode);
	double ReadTemperature();	//unit: Celsius
	double ReadPressure();		//unit: hPa

};

#endif
