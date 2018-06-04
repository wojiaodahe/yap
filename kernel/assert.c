#include "head.h"
void assertion_failure(char *exp, char *file, char *base_file, int line)
{

	printk("assert(%s) failed: file: %s, base_file: %s, line: %d\r\n",
			exp, file, base_file, line);

	/**
	 * If assertion fails in a TASK, the system will halt before
	 * printl() returns. If it happens in a USER PROC, printl() will
	 * return like a common routine and arrive here. 
	 * @see sys_printx()
	 * 
	 * We use a forever loop to prevent the proc from going on:
	 */
	//spin("assertion_failure()");

}
