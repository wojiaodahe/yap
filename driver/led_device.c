#include "platform.h"
#include "s3c24xx.h"
#include "fs.h"

struct resource 	led_resource0[2];
struct platform_device led_dev0;

struct resource 	led_resource1[2];
struct platform_device led_dev1;

struct resource 	led_resource2[2];
struct platform_device led_dev2;

struct resource 	led_resource3[2];
struct platform_device led_dev3;


int led_device_init()
{
	int ret;

	led_resource0[0].start = 0x56000050;
	led_resource0[0].end   = 0x56000054;
	led_resource0[0].name  = "GPIOF";
	led_resource0[0].flag  = IORESOURCE_MEM;

	led_resource0[1].start = 5;
	led_resource0[1].end   = 5;
	led_resource0[1].name  = "LEDIO_REGION";
	led_resource0[1].flag  = IORESOURCE_IO;


	led_dev0.name = "led";
	led_dev0.num_resource = 2;
	led_dev0.resource = led_resource0;

    ret = platform_device_register(&led_dev0);
    if (ret < 0)
    	return ret;
#if 1
////////////////////////////////////////////////////////////
	led_resource1[0].start = 0x56000050;
	led_resource1[0].end   = 0x56000054;
	led_resource1[0].name  = "GPIOF";
	led_resource1[0].flag  = IORESOURCE_MEM;

	led_resource1[1].start = 4;
	led_resource1[1].end   = 4;
	led_resource1[1].name  = "LEDIO_REGION";
	led_resource1[1].flag  = IORESOURCE_IO;


	led_dev1.name = "led";
	led_dev1.num_resource = 2;
	led_dev1.resource = led_resource1;

    ret = platform_device_register(&led_dev1);
    if (ret < 0)
    	return ret;

////////////////////////////////////////////////////////////
	led_resource2[0].start = 0x56000050;
	led_resource2[0].end   = 0x56000054;
	led_resource2[0].name  = "GPIOF";
	led_resource2[0].flag  = IORESOURCE_MEM;

	led_resource2[1].start = 6;
	led_resource2[1].end   = 6;
	led_resource2[1].name  = "LEDIO_REGION";
	led_resource2[1].flag  = IORESOURCE_IO;


	led_dev2.name = "led";
	led_dev2.num_resource = 2;
	led_dev2.resource = led_resource2;

    ret = platform_device_register(&led_dev2);
    if (ret < 0)
    	return ret;

#if 0
////////////////////////////////////////////////////////////
	led_resource3[0].start = 0x56000010;
	led_resource3[0].end   = 0x56000014;
	led_resource3[0].name  = "GPIOB";
	led_resource3[0].flag  = IORESOURCE_MEM;

	led_resource3[1].start = 8;
	led_resource3[1].end   = 8;
	led_resource3[1].name  = "LEDIO_REGION";
	led_resource3[1].flag  = IORESOURCE_IO;


	led_dev3.name = "led";
	led_dev3.num_resource = 2;
	led_dev3.resource = led_resource3;

    ret = platform_device_register(&led_dev3);
    if (ret < 0)
    	return ret;
#endif
#endif
	return 0;
}
