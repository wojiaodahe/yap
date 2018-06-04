#include "dm9000.h"
#include <string.h>

extern U8 Buffer[1000];	
extern U8 host_mac_addr[6];
extern U8 mac_addr[6] ;
extern U8 ip_addr[4];
extern U8 host_ip_addr[4];
extern U16 packet_len;


ARP_HDR * ARPBUF;
#define HON(n) ((((U16)((n) & 0xff)) << 8) | (((n) & 0xff00) >> 8))
void arp_request(void) //发送ARP请求数据包
{
	ARPBUF = (ARP_HDR*)&Buffer;
	memcpy(ARPBUF->ethhdr.d_mac, host_mac_addr, 6);
	memcpy(ARPBUF->ethhdr.s_mac, mac_addr, 6);
	ARPBUF->ethhdr.type = HON( 0x0806 );	
	ARPBUF->hwtype = HON( 1 );
	ARPBUF->protocol = HON( 0x0800 );
	ARPBUF->hwlen = 6;
	ARPBUF->protolen = 4;
	ARPBUF->opcode = HON(1);//
	memcpy(ARPBUF->smac, mac_addr, 6);
	memcpy(ARPBUF->sipaddr, ip_addr, 4);
	memcpy(ARPBUF->dipaddr, host_ip_addr, 4);
	packet_len = 42;//14+28=42	
	DM9000_sendPcket(Buffer, packet_len );
}

U8 arp_process(void)//ARP接收函数，成功返回1，否则返回0
{
	//简单判断ARP数据包有无损坏,有损坏则丢弃,不予处理
	if( packet_len < 28 )//ARP数据长度为28字节为无效数据
	{
		return 0;
	}
	switch ( HON( ARPBUF->opcode ) )
	{
	   case 1    : //处理ARP请求
	         if( ARPBUF->dipaddr[0] == ip_addr[0] &&
	             ARPBUF->dipaddr[1] == ip_addr[1] &&
	             ARPBUF->dipaddr[2] == ip_addr[2] &&
	             ARPBUF->dipaddr[3] == ip_addr[3] )//判断是否是自己的IP，是否向自己询问MAC地址
	         { 
	             ARPBUF->opcode = HON( 2 );//设置为ARP应答
	             memcpy(ARPBUF->dmac, ARPBUF->smac, 6);
	             memcpy(ARPBUF->ethhdr.d_mac, ARPBUF->smac, 6);
	             memcpy(ARPBUF->smac, mac_addr, 6);
	             memcpy(ARPBUF->ethhdr.s_mac, mac_addr, 6);
	             memcpy(ARPBUF->dipaddr, ARPBUF->sipaddr, 4);
	             memcpy(ARPBUF->sipaddr, ip_addr, 4);
	             ARPBUF->ethhdr.type = HON( 0x0806 );
	             packet_len = 42;
	             DM9000_sendPcket( Buffer, packet_len );//发送ARP数据包
	             return 1;
	         }
	         else
	         {
//       	 Printf("其他的ARP请求\n");
	             return 0;
	         }
	         
	         break;
	   case 2    : //处理ARP应答
 	 		 Printf("处理ARP应答\n");
	         if( ARPBUF->dipaddr[0] == ip_addr[0] &&
	             ARPBUF->dipaddr[1] == ip_addr[1] &&
	             ARPBUF->dipaddr[2] == ip_addr[2] &&
	             ARPBUF->dipaddr[3] == ip_addr[3] )//再次判断IP，是否是给自己的应答
	         {
//       	  Printf("自己的ARP应答\n");
				  memcpy(host_ip_addr, ARPBUF->sipaddr, 4);
		          memcpy(host_mac_addr, ARPBUF->smac, 6);//保存服务器MAC地址
		          return 1;
	         }
	         else
	         {
//       	 Printf("其他的ARP应答\n");
	             return 0;
	         }
	         break;
	default     ://不是ARP协议
// 			 Printf("不是ARP协议\n");
	         return 0;
	}
}
