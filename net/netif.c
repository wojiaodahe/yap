#include "ip.h"
#include "arp.h"
#include "eth.h"
#include "netdevice.h"
#include "kernel.h"
#include "socket.h"
#include "inet.h"
#include "printk.h"
#include "syslib.h"
#include "wait.h"
#include "bitops.h"

static wait_queue_t rx_wq;
static struct list_head netif_rx_queue;

static void __netif_schedule(struct net_device *ndev)
{
    int ret;
    struct list_head *list;
    struct sk_buff   *skb;

    if (list_empty(&ndev->tx_queue))
        return;

    if (test_bit(__QUEUE_STATE_XOFF, ndev->state))
        return;

    list = ndev->tx_queue.next;
    skb = list_entry(list, struct sk_buff, list);
    
    ret = ndev->hard_start_xmit(skb, ndev);

    switch (ret)
    {
    case NETDEV_TX_OK:
        /*
         * list_del(&skb->list)
         * */
        break;
    case NETDEV_TX_BUSY:
        break;
    case NETDEV_TX_LOCKED:
        break;
    }
}

void netif_start_queue()
{
    
}

void netif_stop_queue(struct net_device *ndev)
{
    set_bit(__QUEUE_STATE_XOFF, &ndev->state);
}

void netif_wake_queue(struct net_device *ndev)
{
    clear_bit(__QUEUE_STATE_XOFF, &ndev->state);
    __netif_schedule(ndev);
}

int netif_tx_queue(struct net_device *ndev, struct sk_buff *skb)
{
    /**/
    if (test_bit(__QUEUE_STATE_XOFF, ndev->state))
        list_add_tail(&skb->list, &ndev->tx_queue);
    else
        return ndev->hard_start_xmit(skb, ndev);

    return 0;
}

static int netif_rx_thread(void *arg)
{
    struct sk_buff   *skb;
    struct list_head *list;
    
    while (1)
    {
        wait_event(&rx_wq, (!list_empty(&netif_rx_queue)));
        
        list = netif_rx_queue.next;
        while (list != &netif_rx_queue)
        {
            skb = list_entry(list, struct sk_buff, list);
            
            enter_critical();
            list = list->next;
            list_del(&skb->list);
            exit_critical();
           
            eth_recv(skb);
        }
    }
}

void netif_rx(struct sk_buff *skb)
{
    check_addr(skb);
    check_addr(skb->data_buf);
    list_add_tail(&skb->list, &netif_rx_queue);
    wake_up(&rx_wq);
}

void netif_init(void)
{
    INIT_LIST_HEAD(&netif_rx_queue);  
	if (kernel_thread(netif_rx_thread, (void *)0, OS_NETIF_PROCESS_PID))
    {
        printk("%s failed\n", __func__);
        panic();
    }
}
