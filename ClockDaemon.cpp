#include <mysql/mysql.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include "ClockDaemon.h"
#include "utils/m_log.h"
extern "C"
{
	#include "utils/setproctitle.h"
}

TDebugLog g_log;

ClockDaemon::ClockDaemon()
{
	bmp085 = new BMP085(__BMP085_ULTRAHIGHRES);
	dht = new CDHTReader(7, DHT_TYPE_DHT22);
	lcd = new LCD4bit;
}

ClockDaemon::~ClockDaemon()
{
	delete bmp085;
	delete dht;
	delete lcd;
}

double ClockDaemon::GetCPUTemp()
{
	FILE *cpu_temp_file = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	if (cpu_temp_file != NULL)
	{
		int temp;
		fscanf(cpu_temp_file, "%d", &temp);
		fclose(cpu_temp_file);
		return (double)temp / 1000.0;
	}
	else
		return -99.0;
}

void ClockDaemon::UpdateTemperature()
{
	MYSQL mysql;

	char value = 1;
	double humidity = 0.0, dht_temp = 0.0;
	double temperature = bmp085->ReadTemperature();
	double pressure = bmp085->ReadPressure();
	double cpu_temp = GetCPUTemp();
	for (int retry = 0; retry < 10; retry++)
	{
		int ret;
		if ((ret = dht->read(dht_temp, humidity)) == 0)
			break;
		WRN("Read DHT failed %d, retry: %d\n", ret, retry);
	}
	
	INF("CPU Temp:   %0.2f C\n", cpu_temp);
	INF("Temperture: %0.2f C\n", temperature);
	INF("Pressure:   %0.2f hPa\n", pressure);
	INF("Humidity:   %0.2f %\n", humidity);
	INF("DHT_Temp:   %0.2f C\n", dht_temp);

	mysql_init(&mysql);
	mysql_options(&mysql, MYSQL_OPT_RECONNECT, (char *)&value);
	if (mysql_real_connect(&mysql, "192.168.225.107", "root", "1048576", "blogpi", 3306, NULL, 0) != NULL)
	{
		char sql[256];
		snprintf(sql, sizeof(sql), 
			"insert into module_report(update_date, report_id, value) values (now(), '1', '%f')", 
			temperature);
		if (mysql_query(&mysql, sql))
			CRT("Insert db failed: %s!\n", mysql_error(&mysql));
		snprintf(sql, sizeof(sql), 
			"insert into module_report(update_date, report_id, value) values (now(), '2', '%f')", 
			pressure);
		if (mysql_query(&mysql, sql))
			CRT("Insert db failed: %s!\n", mysql_error(&mysql));
		snprintf(sql, sizeof(sql), 
			"insert into module_report(update_date, report_id, value) values (now(), '3', '%f')", 
			cpu_temp);
		if (mysql_query(&mysql, sql))
			CRT("Insert db failed: %s!\n", mysql_error(&mysql));
		snprintf(sql, sizeof(sql), 
			"insert into module_report(update_date, report_id, value) values (now(), '5', '%f')", 
			humidity);
		if (mysql_query(&mysql, sql))
			CRT("Insert db failed: %s!\n", mysql_error(&mysql));
		snprintf(sql, sizeof(sql), 
			"insert into module_report(update_date, report_id, value) values (now(), '6', '%f')", 
			dht_temp);
		if (mysql_query(&mysql, sql))
			CRT("Insert db failed: %s!\n", mysql_error(&mysql));
	}	
	INF("DB Update Finished!\n");
	mysql_close(&mysql);
}

int ClockDaemon::Start()
{
	time_t last_update = 0;
	int reinit_time = 0;
	daemon(1, 0);
	LOG_OPEN(7, "log", "piClock", 10000000, 5, 0x08991000);
	setproctitle("piClock", "daemon");

	lcd->Init();
	dht->init();
	while (1)
	{
		int i, len;
		char Buf[50], Last[50] = { 0 };   
		time_t t = time(NULL);

		strftime(Buf, 50, "%Y-%m-%d %H:%M", localtime(&t));

		len = strlen(Buf);
		for (i = 0; i < len; i++)
			if (Buf[i] != Last[i])
			{
				Last[i] = Buf[i];
				lcd->PrintChar(0, i, Buf[i]); 
			}

		DBG("screen output1: %s\n", Buf);

		snprintf(Buf, 50, "T:%0.1f P:%0.2f", 
			bmp085->ReadTemperature(), 
			bmp085->ReadPressure());
			
		if (++reinit_time > 100)
		{
			lcd->Init();
			reinit_time = 0;
		}
		lcd->Print(1, 0, Buf);
		DBG("screen output2: %s\n", Buf);

		if (t >= last_update + UPDATE_STEPS)
		{
			last_update = t - (t % UPDATE_STEPS);
			UpdateTemperature();
		}

		while (time(NULL) / 60 <= t / 60) // sleep until next minute coming
		{
			sleep(1);
		}
	}
	return 0;
}

