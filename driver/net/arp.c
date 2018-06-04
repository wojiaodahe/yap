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
void arp_request(void) //����ARP�������ݰ�
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

U8 arp_process(void)//ARP���պ������ɹ�����1�����򷵻�0
{
	//���ж�ARP���ݰ�������,��������,���账��
	if( packet_len < 28 )//ARP���ݳ���Ϊ28�ֽ�Ϊ��Ч����
	{
		return 0;
	}
	switch ( HON( ARPBUF->opcode ) )
	{
	   case 1    : //����ARP����
	         if( ARPBUF->dipaddr[0] == ip_addr[0] &&
	             ARPBUF->dipaddr[1] == ip_addr[1] &&
	             ARPBUF->dipaddr[2] == ip_addr[2] &&
	             ARPBUF->dipaddr[3] == ip_addr[3] )//�ж��Ƿ����Լ���IP���Ƿ����Լ�ѯ��MAC��ַ
	         { 
	             ARPBUF->opcode = HON( 2 );//����ΪARPӦ��
	             memcpy(ARPBUF->dmac, ARPBUF->smac, 6);
	             memcpy(ARPBUF->ethhdr.d_mac, ARPBUF->smac, 6);
	             memcpy(ARPBUF->smac, mac_addr, 6);
	             memcpy(ARPBUF->ethhdr.s_mac, mac_addr, 6);
	             memcpy(ARPBUF->dipaddr, ARPBUF->sipaddr, 4);
	             memcpy(ARPBUF->sipaddr, ip_addr, 4);
	             ARPBUF->ethhdr.type = HON( 0x0806 );
	             packet_len = 42;
	             DM9000_sendPcket( Buffer, packet_len );//����ARP���ݰ�
	             return 1;
	         }
	         else
	         {
//       	 Printf("������ARP����\n");
	             return 0;
	         }
	         
	         break;
	   case 2    : //����ARPӦ��
 	 		 Printf("����ARPӦ��\n");
	         if( ARPBUF->dipaddr[0] == ip_addr[0] &&
	             ARPBUF->dipaddr[1] == ip_addr[1] &&
	             ARPBUF->dipaddr[2] == ip_addr[2] &&
	             ARPBUF->dipaddr[3] == ip_addr[3] )//�ٴ��ж�IP���Ƿ��Ǹ��Լ���Ӧ��
	         {
//       	  Printf("�Լ���ARPӦ��\n");
				  memcpy(host_ip_addr, ARPBUF->sipaddr, 4);
		          memcpy(host_mac_addr, ARPBUF->smac, 6);//���������MAC��ַ
		          return 1;
	         }
	         else
	         {
//       	 Printf("������ARPӦ��\n");
	             return 0;
	         }
	         break;
	default     ://����ARPЭ��
// 			 Printf("����ARPЭ��\n");
	         return 0;
	}
}
