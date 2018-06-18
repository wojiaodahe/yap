#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"

struct ethernetif
{
  struct eth_addr *ethaddr;
  int unused;
};

struct netif netif;
unsigned char macaddress[6] = {0x00, 0x1c, 0x82, 0x00, 0x33, 0x1f};

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    DM9000_sendPcket(p->payload, p->len);
    return ERR_OK;
}

void low_level_init(struct netif *netif)
{

    unsigned char mac[] = {0x00, 0x1c, 0x32, 0x00, 0x33, 0x1f};
#if 1
    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    /* set MAC hardware address */
    netif->hwaddr[0] =  macaddress[0];
    netif->hwaddr[1] =  macaddress[1];
    netif->hwaddr[2] =  macaddress[2];
    netif->hwaddr[3] =  macaddress[3];
    netif->hwaddr[4] =  macaddress[4];
    netif->hwaddr[5] =  macaddress[5];

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
#endif
    
    DM9000_init(netif);


}

err_t ethernetif_init(struct netif *netif)
{
    struct ethernetif *ethernetif;
#if LWIP_NETIF_HOSTNAME
      netif->hostname = "TINYOS";
#endif 

    NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 100000000);
    
    netif->state = ethernetif;
    netif->name[0] = 't';
    netif->name[1] = 'i';

    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
    
    /* initialize the hardware */
    low_level_init(netif);

	return ERR_OK;
}

void LWIP_INIT(void)
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;	
	

	mem_init();
	memp_init();
	
#if LWIP_DHCP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else
	IP4_ADDR(&ipaddr, 192, 168, 1, 66);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&gw, 192, 168, 1, 1);
#endif

	//set_mac();
	netif_add(&netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, &ethernet_input);
	netif_set_default(&netif);
#if LWIP_DHCP
#endif
	netif_set_up(&netif);
}
