#include "config.h"
const unsigned int SYSTEM_TOTAL_MEMORY_START	= PHYSICAL_MEM_ADDR;
const unsigned int SYSTEM_TOTAL_MEMORY_SIZE	    = MEM_MAP_SIZE;

const unsigned int IRQ_MODE_STACK				= 0x33f00000;
const unsigned int FIQ_MODE_STACK				= 0x33e00000;
const unsigned int SVC_MODE_STACK				= 0x33d00000;
const unsigned int SYS_MODE_STACK				= 0x33c00000;

const unsigned int MMU_TLB_BASE					= TLB_BASE;

const unsigned int KMALLOC_ADDR_START			= 0x31000000;
const unsigned int KMALLOC_MEM_SIZE				= USER_PROGRAM_SPACE_START - KMALLOC_ADDR_START;
