

#ifndef INCLUDE_NETDEVICE_H_
#define INCLUDE_NETDEVICE_H_

#include "list.h"
#include "device.h"
#include "socket.h"

#define IF_NAME_SIZE	16

enum netdev_queue_state_t
{
	__QUEUE_STATE_XOFF,
	__QUEUE_STATE_FROZEN,
};


#define NETDEV_TX_OK        0
#define NETDEV_TX_BUSY      1
#define NETDEV_TX_LOCKED    -1

struct net_device_stats
{
	unsigned long	rx_packets;		/* total packets received	*/
	unsigned long	tx_packets;		/* total packets transmitted	*/
	unsigned long	rx_bytes;		/* total bytes received 	*/
	unsigned long	tx_bytes;		/* total bytes transmitted	*/
	unsigned long	rx_errors;		/* bad packets received		*/
	unsigned long	tx_errors;		/* packet transmit problems	*/
	unsigned long	rx_dropped;		/* no space in linux buffers	*/
	unsigned long	tx_dropped;		/* no space available in linux	*/
	unsigned long	multicast;		/* multicast packets received	*/
	unsigned long	collisions;

	/* detailed rx_errors: */
	unsigned long	rx_length_errors;
	unsigned long	rx_over_errors;		/* receiver ring buff overflow	*/
	unsigned long	rx_crc_errors;		/* recved pkt with crc error	*/
	unsigned long	rx_frame_errors;	/* recv'd frame alignment error */
	unsigned long	rx_fifo_errors;		/* recv'r fifo overrun		*/
	unsigned long	rx_missed_errors;	/* receiver missed packet	*/

	/* detailed tx_errors */
	unsigned long	tx_aborted_errors;
	unsigned long	tx_carrier_errors;
	unsigned long	tx_fifo_errors;
	unsigned long	tx_heartbeat_errors;
	unsigned long	tx_window_errors;

	/* for cslip etc */
	unsigned long	rx_compressed;
	unsigned long	tx_compressed;
};

struct net_device
{
	char 					name[IF_NAME_SIZE];
	unsigned long			base_addr;	/* device I/O address	*/
	unsigned int			irq;		/* device IRQ number	*/
	unsigned char		 	dma;
	unsigned long 			state;
	unsigned char 			macaddr[6];
	unsigned int 			ip;
	unsigned int 			gw;
	unsigned int 			netmask;
	unsigned short 			mtu;
    void                    *priv;
	struct device			dev;
	struct list_head 		dev_list;
    struct list_head        tx_queue;
	struct net_device_stats	stats;
	int	 (*init)(struct net_device *dev);
	void (*uninit)(struct net_device *dev);
	int	 (*open)(struct net_device *dev);
	int	 (*stop)(struct net_device *dev);
	int	 (*hard_start_xmit)(struct sk_buff *skb, struct net_device *ndev);
	int	 (*set_mac_address)(struct net_device *dev, void *addr);
};

#define to_net_dev(d)               container_of(d, struct net_device, dev)
#define net_device_set_priv(ndev, priv)   ndev->priv = priv
#define net_device_get_priv(ndev)         ndev->priv


extern int register_netdev(struct net_device *dev);
extern void net_device_core_init(void);
extern void netif_wake_queue(struct net_device *ndev);
extern int netif_tx_queue(struct net_device *ndev, struct sk_buff *skb);
extern void netif_stop_queue(struct net_device *ndev);
extern void netif_rx(struct sk_buff *skb);

#endif /* INCLUDE_NETDEVICE_H_ */


