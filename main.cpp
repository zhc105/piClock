#include "ClockDaemon.h"
extern "C"
{
	#include "utils/setproctitle.h"
}

int main(int argc, char *argv[])
{
	ClockDaemon daemon;
	initproctitle(argc, argv);

	return daemon.Start();
}
