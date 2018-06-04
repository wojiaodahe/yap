#include "dm9000.h"
#include "s3c24xx.h"
#include "kernel.h"


#define DM_ADD (*((volatile unsigned short *) 0x20000300))

#define DM_CMD (*((volatile unsigned short *) 0x20000304))




U8 Buffer[1000];	//������һ��1000�ֽڵĽ��շ��ͻ�����
//Ŀ������
U8 host_mac_addr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
U8 host_ip_addr[4]  = {10, 8, 8, 1};
//������
U8 mac_addr[6] 	    = {0, 8, 4, 5, 6, 7};
U8 ip_addr[4] 		= {10, 8, 8, 24};

U16 packet_len;			//���ա��������ݰ��ĳ��ȣ����ֽ�Ϊ��λ


U8 arpsendbuf[60]={
	0xff,0xff,0xff,0xff,0xff,0xff,	//��̫��Ŀ���ַ
	0x02,0x03,0x04,0x05,0x06,0x07,	//��̫��ԭ��ַ
	0x08,0x06,						//֡���ͣ�	ARP֡
	0x00,0x01,						//Ӳ�����ͣ���̫��
	0x08,0x00,						//Э�����ͣ�IPЭ��
	0x06,							//Ӳ����ַ���ȣ�6�ֽ�
	0x04,							//Э���ַ���ȣ�4�ֽ�
	0x00,0x00,						//������ ARP����
	0x02,0x03,0x04,0x05,0x06,0x07,	//���Ͷ�Ӳ����ַ
	192, 168, 1, 230,				//���Ͷ�Э���ַ
	0x00,0x00,0x00,0x00,0x00,0x00,	//���ն�Ӳ����ַ
	192, 168, 1, 100				//���ն�Э���ַ
};


#define DM9KS_ID	0x90000A46
#define DM9KS_VID_L	0x28
#define DM9KS_VID_H	0x29
#define DM9KS_PID_L	0x2A
#define DM9KS_PID_H	0x2B

extern int arp_process(void);
void udelay(U32 t)
{
	U32 i;
	for(;t>0;t--)
	{
		for(i=0;i<100;i++){}
	}
}
void Test_DM9000AE()
{
	int i;
	U32 id_val;
	//	rBWSCON = rBWSCON & ~(0xf<<16)|(0xd<<16);
	//	rBANKCON4 = (1 << 13)|(1<<11)|(6<<8)|(1<<6)|(1<<4)|(0<<2)|0;
	//
	i = GPACON;
    GPACON |= 0x7fffff;
	BWSCON = (BWSCON & ~(0x03 << 16)) | (0x01 << 16);
	BANKCON4 = (1 << 13) | (2 << 11) | (7 << 8) | (2 << 6) | (1 << 4) | (1 << 2) | 0;

	id_val = 0;
	DM_ADD = DM9KS_VID_L;
	id_val = (U8)DM_CMD;
	DM_ADD = DM9KS_VID_H;
	id_val |= (U8)DM_CMD<<8;
	DM_ADD = DM9KS_PID_L;udelay(2);
	id_val |= (U8)DM_CMD<<16;
	DM_ADD = DM9KS_PID_H;udelay(2);
	id_val |= (U8)DM_CMD<<24;
	Printf("DM9000AE ChipId is %x\r\n", id_val);
	if(id_val == DM9KS_ID)
	{
		Printf("DM9000 ID ��ȷ\r\n");
	}
	else
	{
		Printf("DM9000 ID ����\r\n");
	}

}

static void __irq Eint7_ISR(void)
{

	U32 len,i;
	Printf("Eint7�жϷ���\r\n");	
	len = receivepacket(Buffer);
	Printf("�������ݳ���:%d <--->",len);
	for(i=0;i<len;i++)
	{
		Printf("%d ",Buffer[i]);
		if((i%5)==0) Printf("\r\n");
	}
	Printf("\r\n���ݽ���\r\n");
	//ClearPending(BIT_EINT4_7);
	//rEINTPEND |= 1<<7;
}
/**/
static void int_issue(void)
{
	U32 i;
	i = receivepacket(Buffer);
	if(i != 0)
	{
		i = arp_process();
		if(i == 1)
		{
			Print_HostMAC();
		}
	}
	//ClearPending(BIT_EINT4_7);
	//EINTPEND |= 1 << 7;
}

void IOSetInit(void)
{
#if 0
	rGPFCON = (rGPFCON & (~(0x03<<14))) | (0x02<<14);	//GPF7����ΪEINT7
	rEXTINT0 = (rEXTINT0 & (~(0x07<<28))) | (0x01<<28);//�ߵ�ƽ����
	rEINTMASK = rEINTMASK & (~(0x01<<7));//ʹ���ж�
	ClearPending(BIT_EINT4_7);
	pISR_EINT4_7 = (U32)int_issue;//(U32)Eint7_ISR;
	rINTMSK = rINTMSK & (~(BIT_EINT4_7));
#else
	//EINT7
	GPFCON  &= ~(3 << 14);
	GPFCON  |= 2 << 14; //Set GPF7 To EINT7 Mode

	EXTINT0 = (EXTINT0 & (~(0x07 << 28))) | (0x01 << 28);//??EINT7??????
	EINTMASK &= ~(1 << 7);//??EINT7
	SRCPND = SRCPND | (0x1 << 4);
	INTPND = INTPND | (0x1 << 4);
	put_irq_handler(DM9000_IRQ, int_issue);
	INTMSK &= ~(1 << 4);
#endif
}

//��DM9000�Ĵ���д����
void dm9000_reg_write(U16 reg, U16 data)
{  
	udelay(20);		//֮ǰ�����΢���ʱ������������ʱ20us
	DM_ADD = reg;	//���Ĵ�����ַд��INDEX�˿�
	udelay(20);
	DM_CMD = data;	//������д��DATA�˿ڣ���д���Ĵ���

}

//��DM9000�Ĵ���������
U8 dm9000_reg_read(U16 reg)
{
	udelay(20);
	DM_ADD = reg;
	udelay(20);
	return DM_CMD;//�����ݴӼĴ����ж���	
}
//��DM9000PHY�Ĵ���д����
void dm9000_reg_writePHY(U16 reg, U16 data)
{
	//�Ĵ�����ַд��EPAR/PHY_AR��0CH���Ĵ�����,
	//ע�⽫�Ĵ�����ַ�ĵ�6λ��1����ַ��0x40�����㼴�ɣ���
	///�Ա���д����PHY��ַ��������EEPROM��ַ
	dm9000_reg_write(DM9000_EPAR, reg|0x40);
	//�����ݸ��ֽ�д��PHY_DRH��0EH���Ĵ�����
	dm9000_reg_write(DM9000_EPDRH, (data>>8)&0xff);
	//�����ݵ��ֽ�д��PHY_DRL��0DH���Ĵ�����
	dm9000_reg_write(DM9000_EPDRL, data&0xff);
	//����PHYд����(0x0a����EPCR/PHY_CR��0BH���Ĵ�����	
	dm9000_reg_write(DM9000_EPCR, 0x0a);
	//��ʱ20us����������0x08��EPCR/PHY_CR��0BH���Ĵ����У����PHY������
	udelay(20);
	dm9000_reg_write(DM9000_EPCR, 0x08);
}
//��DM9000PHY�Ĵ���������
U16 dm9000_reg_readPHY(U16 reg)
{
	U16 data;
	//�Ĵ�����ַд��EPAR/PHY_AR��0CH���Ĵ�����,
	//ע�⽫�Ĵ�����ַ�ĵ�6λ��1����ַ��0x40�����㼴�ɣ���
	///�Ա���д����PHY��ַ��������EEPROM��ַ
	dm9000_reg_write(DM9000_EPAR, reg|0x40);
	//����PHY������(0x0c����EPCR/PHY_CR��0BH���Ĵ�����	
	dm9000_reg_write(DM9000_EPCR, 0x0c);
	//�����ݸ��ֽڴ�PHY_DRH��0EH���Ĵ����ж���
	data = dm9000_reg_read(DM9000_EPDRH);
	//�����ݵ��ֽڴ�PHY_DRL��0DH���Ĵ����ж���
	data = (data<<8) | dm9000_reg_read(DM9000_EPDRL);
	//��ʱ20us����������0x08��EPCR/PHY_CR��0BH���Ĵ����У����PHY������
	udelay(20);
	dm9000_reg_write(DM9000_EPCR, 0x08);
	return data;
}
void DM9000_init(void)
{
	U32 i;
	Test_DM9000AE();
	IOSetInit();
	//��ʼ�����ò���: 0
	dm9000_reg_write(DM9000_GPCR, 0x01);	//���� GPCR(1EH) bit[0]=1��ʹDM9000��GPIO0Ϊ�����
	dm9000_reg_write(DM9000_GPR,  0x00);	//GPR bit[0]=0 ʹDM9000��GPIO3���Ϊ���Լ����ڲ�PHY��
	udelay(5000);							//��ʱ2ms���ϵȴ�PHY�ϵ硣
	///��ʼ�����ò���: 1
	//	dm9000_reg_write(DM9000_NCR,  0x80);	//����NCR[7]=1,�����ⲿPHY
	//	udelay(5000);							//��ʱ2ms���ϵȴ�PHY�ϵ�
	//��ʼ�����ò���: 2
	dm9000_reg_write(DM9000_NCR,  0x03);	//�����λ
	udelay(30);								//��ʱ20us���ϵȴ������λ���
	dm9000_reg_write(DM9000_NCR,  0x00);	//��λ��ɣ�������������ģʽ��
	dm9000_reg_write(DM9000_NCR,  0x03);	//�ڶ��������λ��Ϊ��ȷ�������λ��ȫ�ɹ����˲����Ǳ�Ҫ�ġ�
	udelay(30);
	dm9000_reg_write(DM9000_NCR,  0x00);
	//��ʼ�����ò���: 3
	dm9000_reg_write(DM9000_NSR,  0x2c);	//�������״̬��־λ
	dm9000_reg_write(DM9000_ISR,  0x3f);	//��������жϱ�־λ
	//��ʼ�����ò���: 4
	dm9000_reg_write(DM9000_RCR,  0x39);	//���տ���
	dm9000_reg_write(DM9000_TCR,  0x00);	//���Ϳ���
	dm9000_reg_write(DM9000_BPTR, 0x3f);	//��ѹ����
	dm9000_reg_write(DM9000_FCTR, 0x3a);	//����FIFO����3k 8k
	dm9000_reg_write(DM9000_FCR,  0xff);	//����/�������
	dm9000_reg_write(DM9000_SMCR, 0x00);	//����ģʽ
	//��ʼ�����ò���: 5
	for(i=0; i<6; i++)
		dm9000_reg_write(DM9000_PAR + i, mac_addr[i]);//mac_addr[]�Լ�����һ�°ɣ�6���ֽڵ�MAC��ַ

	//��ʼ�����ò���: 6
	dm9000_reg_write(DM9000_NSR,  0x2c);	//�������״̬��־λ
	dm9000_reg_write(DM9000_ISR,  0x3f);	//��������жϱ�־λ
	//��ʼ�����ò���: 7
	dm9000_reg_write(DM9000_IMR, 0x81);		//�ж�ʹ��	
	PrintfDM9000Reg();

}
void DM9000_sendPcket(U8 *datas, U32 length)
{
	U32 len,i;
	U8 tmp;

	Printf("��������\r\n");

	dm9000_reg_write(DM9000_IMR,0x80);		//�Ƚ�ֹ�����жϣ���ֹ�ڷ�������ʱ���жϸ���	
	len = length;							//�ѷ��ͳ���д��
	dm9000_reg_write(DM9000_TXPLH, (len>>8) & 0x0ff);
	dm9000_reg_write(DM9000_TXPLL, len & 0x0ff);
	DM_ADD = DM9000_MWCMD;					//�洢������ַ�Զ����ӵĶ���������
	for(i=0; i<len; i+=2)					//16 bit mode
	{
		udelay(2);
		DM_CMD = datas[i] | (datas[i+1]<<8);
	}
	dm9000_reg_write(DM9000_TCR, 0x01);		//�������ݵ���̫����

	while(1)//�ȴ����ݷ������
	{
		U8 data;
		data = dm9000_reg_read(DM9000_TCR);//DM9000_NSR
		if((data&0x01) == 0x00) break;
	}
	tmp = dm9000_reg_read(DM9000_NSR);

	if((tmp & 0x0f) == 0x04)
	{
		if((dm9000_reg_read(DM9000_TSR1)&0xfc) == 0x00)
			Printf("TSR1�ɹ�\r\n");
		else
			Printf("TSR1ʧ��\r\n");   	
	}
	else
	{
		if((dm9000_reg_read(DM9000_TSR2)&0xfc) == 0x00)
			Printf("TSR2�ɹ�\r\n");
		else
			Printf("TSR2ʧ��\r\n");
	}
	dm9000_reg_write(DM9000_NSR, 0x2c);		//���״̬�Ĵ��������ڷ�������û�������жϣ���˲��ش����жϱ�־λ
	dm9000_reg_write(DM9000_IMR, 0x81);		//DM9000�����Ľ����ж�ʹ��
	Printf("�����������\r\n");
}
U32 receivepacket(U8 *datas)
{
	U16 i,tmp,status,len;
	U8 ready;
	//	U32 st;
	ready = 0;								//ϣ����ȡ��"01H"
	status = 0;								//���ݰ�״̬
	len = 0; 								//���ݰ�����
	if(dm9000_reg_read(DM9000_ISR) & 0x01)			
	{
		dm9000_reg_write(DM9000_ISR, 0x01);	//��������жϱ�־λ
	}
    //printk("%x\n", dm9000_reg_read(DM9000_ISR));

	ready = dm9000_reg_read(DM9000_MRCMDX); // ��һ�ζ�ȡ��һ���ȡ������ 00H
	//	Printf("ready1 = %x\r\n",ready);
	if((ready & 0x0ff) != 0x01)
	{
		ready = dm9000_reg_read(DM9000_MRCMDX); // �ڶ��ζ�ȡ�����ܻ�ȡ������
		//        Printf("ready2 = %x\r\n",ready);
		if((ready & 0x01) != 0x01)
		{
			/*
			   if((ready & 0x01) != 0x00) 		//���ڶ��ζ�ȡ���Ĳ��� 01H �� 00H �����ʾû�г�ʼ���ɹ�
			   {
			   dm9000_reg_write(DM9000_IMR, 0x80);//���������ж�
			   DM9000_init();				//���³�ʼ��
			   dm9000_reg_write(DM9000_IMR, 0x81);//�������ж�
			   }
			   return 0;
			   */
		}
	}

	status = dm9000_reg_read(DM9000_MRCMD);
	//  DM_ADD = DM9000_MRCMD;
	//  st = DM_CMD;
	//  status = st; 

	//  len = 64;// DM_CMD;
	len =  DM_CMD;
	//  Printf("st=%x status=%x  len= %x\r\n",st,status,len);
	if( (len < 1522))//!(status & 0xbf) &&
	{
		for(i=0; i<len; i+=2)// 16 bit mode
		{
			udelay(20);
			tmp = DM_CMD;
			datas[i] = tmp & 0x0ff;
			datas[i + 1] = (tmp >> 8) & 0x0ff;
		}
	}
	else			return 0;
	if(len > 1000) 	return 0;
	//  if( (HON( ETHBUF->type ) != ETHTYPE_ARP) && (HON( ETHBUF->type ) != ETHTYPE_IP) )
	//		return 0;

	packet_len = len;	
	//  Printf("len = %d\n", len);
	return len;
}



void PrintfDM9000Reg(void)
{
	/*	U16 data,i;
		for(i=0x0;i<0x10;i++)
		{
		data = dm9000_reg_read(i);
		Printf("RET_0x%02x = %x\r\n",i,data);
		}
		Printf("\r\n");
		for(i=0x10;i<0x16;i++)
		{
		data = dm9000_reg_read(i);
		Printf("RET_0x%02x = %02x\r\n",i,data);
		}

		i=0x16;
		data = dm9000_reg_read(i);
		Printf("RET_0x%2x = %x\r\n",i,data);

		for(i=0x1e;i<0x26;i++)
		{
		data = dm9000_reg_read(i);
		Printf("RET_0x%2x = %x\r\n",i,data);
		}

		for(i=0x28;i<0x2c;i++)
		{
		data = dm9000_reg_read(i);
		Printf("RET_0x%2x = 0x%x\r\n",i,data);
		}

		i=0x2f;
		data = dm9000_reg_read(i);
		Printf("RET_0x%2x = %x\r\n",i,data);

		for(i=0xf0;i<0xff;i++)
		{
		data = dm9000_reg_read(i);
		Printf("RET_0x%2x = %x\r\n",i,data);
		}
		data = dm9000_reg_read(i);
		Printf("RET_0x%2x = %x\r\n",i,data);

		data = dm9000_reg_read(0xfe);
		Printf("������IOģʽ:%x\r\n",((U8)data)>>6);

		data = dm9000_reg_read(0x01);
		if(((U8)data)>>1)
		Printf("10M��̫��\r\n");				
		else
		Printf("100M��̫��\r\n");

		data = dm9000_reg_read(0x02);
		Printf("TCR��02H�������Ϳ��ƼĴ���:%x\r\n",((U8)data));

		TestDm9000();
		*/
	U8 val, i;
	U16 data;
	Printf("Registers:\n");
	val = dm9000_reg_read(DM9000_NCR);
	Printf("NCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_NSR);
	Printf("NSR = %02x(%03d)    ", val, val);
	val = dm9000_reg_read(DM9000_TCR);
	Printf("TCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_RCR);
	Printf("RCR = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_RSR);
	Printf("RSR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_ROCR);
	Printf("ROCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_BPTR);
	Printf("BPTR = %02x(%03d)  ", val, val);
	val = dm9000_reg_read(DM9000_FCTR);
	Printf("FCTR = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_FCR);
	Printf("FCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_GPCR);
	Printf("GPCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_GPR);
	Printf("GPR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_WCR);
	Printf("WCR = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_TRPAL);
	Printf("TRPAL = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_TRPAH);
	Printf("TRPAH = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_RWPAL);
	Printf("RWPAL = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_RWPAH);
	Printf("RWPAH = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_VIDL);
	Printf("VIDL  = %02x(%03d)   ", val, val);	
	val = dm9000_reg_read(DM9000_VIDH);
	Printf("VIDH  = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_PIDL);
	Printf("PIDL  = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_PIDH);
	Printf("PIDH  = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_CHIPR);
	Printf("CHIPR = %02x(%03d)   ", val, val);	
	val = dm9000_reg_read(DM9000_SMCR);
	Printf("SMCR  = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_TXPLL);
	Printf("TXPLL = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_TXPLH);
	Printf("TXPLH = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_ISR);
	Printf("ISR   = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_IMR);
	Printf("IMR   = %02x(%03d) \n", val, val);

	Printf("MAC = ");
	for(i = 0; i < 5; i++)
	{
		val = dm9000_reg_read(DM9000_PAR + i);
		Printf(" %02x(%03d): ", val, val);
	}
	val = dm9000_reg_read(DM9000_PAR + i);
	Printf(" %02x(%03d)\n ", val, val);
	///////////////////////////////

	Printf("PHY Registers:\n");
	data = dm9000_reg_readPHY(DM9000_BMCR);
	Printf(" BMCR   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_BMSR);
	Printf("BMSR   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_PHYID1);
	Printf("PHYID1 = %04x(%05d)\n ", data, data);

	data = dm9000_reg_readPHY(DM9000_PHYID2);
	Printf("PHYID2 = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_ANAR);
	Printf("ANAR   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_DSCR);
	Printf("DSCR   = %04x(%05d)\n ", data, data);

	data = dm9000_reg_readPHY(DM9000_ANLPAR);
	Printf("ANLPAR = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_ANER);
	Printf("ANER   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_DSCSR);
	Printf("DSCSR  = %04x(%05d)\n ", data, data);

	data = dm9000_reg_readPHY(DM9000_10BTCSR);
	Printf("BTCSR  = %04x(%05d)\n ", data, data);

	TestDm9000();
}

void TestDm9000(void)
{
	//DM9000_sendPcket(arpsendbuf,60);
	arp_request();
}
void Print_HostMAC(void)
{
	U8 i;
	Printf("Host IP is:\n");
	for(i = 0; i < 4; i++)
		Printf("%02x(%03d) ", host_ip_addr[i], host_ip_addr[i]);

	Printf("\nHost MAC is:\n");
	for(i = 0; i < 6; i++)
		Printf("%02x(%03d) ", host_mac_addr[i], host_mac_addr[i]);
	Printf("\n");
}
void testNetwork(void)
{
	U8 dat;
	while(1)
	{
		dat =  dm9000_reg_read(DM9000_NSR);
		if(dat&0x40)
			break;
	}
}

/*
   9
   8 4 5 6 7
   0 19 211 53 228
   253 8 6 0 1
   8 0 6 4 0
   2 0 19 211 53
   228 253 192 168 1
   100 9 8 4 5
   6 7 192 168 1
   230 0 0 0 0
   0 0 0 0 0
   0 0 0 0 0
   0 0 0 0 54
   218 187 81

*/
