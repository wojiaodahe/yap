#include "printf.h"

int main(int argc, const char *argv[])
{
    
    while (1)
    {
		printf("test printf %d %x %c %s", 10, 0xaa, 'p', "test string\n");

        sleep(1);
    }
    return 0;
}

