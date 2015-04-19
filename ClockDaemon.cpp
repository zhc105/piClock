#include <mysql/mysql.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include "ClockDaemon.h"

ClockDaemon::ClockDaemon()
{
	bmp085 = new BMP085(__BMP085_ULTRAHIGHRES);
	lcd = new LCD4bit;
}

ClockDaemon::~ClockDaemon()
{
	delete bmp085;
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

	double temperature = bmp085->ReadTemperature();
	double pressure = bmp085->ReadPressure();
	double cpu_temp = GetCPUTemp();
	
	//printf("CPU Temp:   %0.2f C\n", cpu_temp);
	//printf("Temperture: %0.2f C\n", temperature);
	//printf("Pressure:   %0.2f hPa\n", pressure);

	mysql_init(&mysql);
	if (mysql_real_connect(&mysql, "192.168.225.107", "root", "1048576", "blogpi", 3306, NULL, 0) != NULL)
	{
		char sql[256];
		snprintf(sql, sizeof(sql), 
			"insert into module_report(update_date, report_id, value) values (now(), '1', '%f')", 
			temperature);
		if (mysql_query(&mysql, sql))
			printf("Update Failed!\n");
		snprintf(sql, sizeof(sql), 
			"insert into module_report(update_date, report_id, value) values (now(), '2', '%f')", 
			pressure);
		if (mysql_query(&mysql, sql))
			printf("Update Failed!\n");
		snprintf(sql, sizeof(sql), 
			"insert into module_report(update_date, report_id, value) values (now(), '3', '%f')", 
			cpu_temp);
		if (mysql_query(&mysql, sql))
			printf("Update Failed!\n");
	}	
	printf("Update Finished!\n");
	mysql_close(&mysql);
}

int ClockDaemon::Start()
{
	time_t last_update = 0;
	daemon(1, 0);
	lcd->Init();
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
		snprintf(Buf, 50, "T:%0.1f P:%0.2f", 
			bmp085->ReadTemperature(), 
			bmp085->ReadPressure());
		lcd->Print(1, 0, Buf);
		//printf("%s\n", Buf);

		if (t >= last_update + UPDATE_STEPS)
		{
			last_update = t - (t % UPDATE_STEPS);
			UpdateTemperature();
		}

		sleep(60);
	}
	return 0;
}

