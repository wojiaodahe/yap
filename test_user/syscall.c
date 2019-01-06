#include "syscall.h"

void sleep(unsigned int time)
{
	const char argc = 2;
	unsigned int argv[argc];
	
	argv[0] = SYSTEM_CALL_SSLEEP;
	argv[1] = time;
	user_syscall(argc, argv);
}

void msleep(unsigned int time)
{
	const char argc = 2;
	unsigned int argv[argc];

	argv[0] = SYSTEM_CALL_MSLEEP;
	argv[1] = time;
	user_syscall(argc, argv);
}


