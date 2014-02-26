#include "ClockDaemon.h"

int main(int argc, char *argv[])
{
	ClockDaemon daemon;
	return daemon.Start();
}
