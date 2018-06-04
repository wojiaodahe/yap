#include "config.h"

#define UPSCB_NO_USED					0
#define UPSCB_IS_USED					1

typedef struct user_program_space_control_brock
{
	unsigned int flag;
	unsigned long addr;
}upscb;

static struct user_program_space_control_brock UPSCB[MAX_USER_PROGRAM_NUM];

unsigned long get_user_program_space()
{
	int i;

	for (i = 0; i < MAX_USER_PROGRAM_NUM; i++)
	{
		if (UPSCB[i].flag == UPSCB_NO_USED)
		{
			UPSCB[i].flag = UPSCB_IS_USED;
			return UPSCB[i].addr;
		}
	}

	return 0;
}

void put_user_program_space(unsigned long addr)
{
	int i;

	for (i = 0; i < MAX_USER_PROGRAM_NUM; i++)
	{
		if ((addr & 0xfff00000) == UPSCB[i].addr)
		{
			UPSCB[i].flag = UPSCB_NO_USED;
			return;
		}
	}
}

void init_user_program_space()
{
	int i;
	unsigned long tmp;

	tmp = USER_PROGRAM_SPACE_START;
	for (i = 0; i < MAX_USER_PROGRAM_NUM; i++)
	{
		UPSCB[i].flag = UPSCB_NO_USED;
		UPSCB[i].addr = tmp;
		tmp += USER_PROGRAM_SPACE_UNIT_SIZE;
	}
}


