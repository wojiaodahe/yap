#include "platform.h"

struct resource test_resource[2];
struct platform_device test_dev;

struct resource test_resource1[2];
struct platform_device test_dev1;

int test_probe(struct platform_device *pdev)
{
    int i;
    struct resource *res;

    res = platform_get_resource(pdev, IORESOURCE_IO, 0);
    printk("io start : %d end: %d\n", res->start, res->end);

    res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    printk("irq num: %d\n", res->start);

    printk("------------\n");
    return 0;
}

int test_remove(struct platform_device *pdev)
{
    printk("+++++++++++\n");
    return 0;
}
struct platform_driver test_drv;

int test_platform()
{

	test_drv.probe = test_probe;
	test_drv.remove = test_remove;
	test_drv.id_table = 0;
	test_drv.driver.name = "test";


	test_resource[0].start = 100;
	test_resource[0].end = 200;
	test_resource[0].name = "Mem";
	test_resource[0].flag = IORESOURCE_IO;
	test_resource[1].start = 21;
	test_resource[1].end = 21;
	test_resource[1].name = "irq";
	test_resource[1].flag = IORESOURCE_IRQ;

	test_dev.name = "test";
	test_dev.num_resource = 2;
	test_dev.resource = test_resource;



	test_resource1[0].start = 500;
	test_resource1[0].end = 800;
	test_resource1[0].name = "Mem";
	test_resource1[0].flag = IORESOURCE_IO;
	test_resource1[1].start = 22;
	test_resource1[1].end = 22;
	test_resource1[1].name = "irq";
	test_resource1[1].flag = IORESOURCE_IRQ;

	test_dev1.name = "test";
	test_dev1.num_resource = 2;
	test_dev1.resource = test_resource1;

    platform_device_register(&test_dev);
    platform_device_register(&test_dev1);
    platform_driver_register(&test_drv);

    platform_device_unregister(&test_dev);
    platform_device_unregister(&test_dev1);
    platform_driver_unregister(&test_drv);

    return 0;
}
