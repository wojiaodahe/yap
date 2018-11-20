/*
 * net_dev.c
 *
 *  Created on: 2018Äê6ÔÂ28ÈÕ
 *      Author: crane
 */

#include "netdevice.h"


static struct list_head net_device_head;

int register_netdev(struct net_device *dev)
{
	list_add(&dev->dev_list, &net_device_head);
	return 0;
}

struct net_device *return_ndev()
{
	struct net_device *ndev;
	struct list_head *list;

	if (list_empty(&net_device_head))
		return 0;

	list = net_device_head.next;
	ndev = container_of(list, struct net_device, dev_list);
	return ndev;
}

void net_core_init()
{
	arp_send_q_init();
	INIT_LIST_HEAD(&net_device_head);
}
