#include "dm9000.h"
#include "s3c24xx.h"
#include "kernel.h"
#include "common.h"
#include "netdevice.h"
#include "socket.h"

#define DM_ADD (*((volatile unsigned short *) 0x20000300))
#define DM_CMD (*((volatile unsigned short *) 0x20000304))


#define DM9KS_ID	0x90000A46
#define DM9KS_VID_L	0x28
#define DM9KS_VID_H	0x29
#define DM9KS_PID_L	0x2A
#define DM9KS_PID_H	0x2B


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
	printk("DM9000AE ChipId is %x\r\n", id_val);
	if(id_val == DM9KS_ID)
	{
		printk("DM9000 ID 正确\r\n");
	}
	else
	{
		printk("DM9000 ID 不对\r\n");
	}

}

//向DM9000寄存器写数据
void dm9000_reg_write(U16 reg, U16 data)
{  
	udelay(20);		//之前定义的微妙级延时函数，这里延时20us
	DM_ADD = reg;	//将寄存器地址写到INDEX端口
	udelay(20);
	DM_CMD = data;	//将数据写到DATA端口，即写进寄存器

}

//从DM9000寄存器读数据
unsigned short dm9000_reg_read(U16 reg)
{
	udelay(20);
	DM_ADD = reg;
	udelay(20);
	return DM_CMD;//将数据从寄存器中读出	
}
//向DM9000PHY寄存器写数据
void dm9000_reg_writePHY(U16 reg, U16 data)
{
	//寄存器地址写到EPAR/PHY_AR（0CH）寄存器中,
	//注意将寄存器地址的第6位置1（地址与0x40或运算即可），
	///以表明写的是PHY地址，而不是EEPROM地址
	dm9000_reg_write(DM9000_EPAR, reg|0x40);
	//将数据高字节写到PHY_DRH（0EH）寄存器中
	dm9000_reg_write(DM9000_EPDRH, (data>>8)&0xff);
	//将数据低字节写到PHY_DRL（0DH）寄存器中
	dm9000_reg_write(DM9000_EPDRL, data&0xff);
	//发送PHY写命令(0x0a）到EPCR/PHY_CR（0BH）寄存器中	
	dm9000_reg_write(DM9000_EPCR, 0x0a);
	//延时20us，发送命令0x08到EPCR/PHY_CR（0BH）寄存器中，清除PHY读操作
	udelay(20);
	dm9000_reg_write(DM9000_EPCR, 0x08);
}
//从DM9000PHY寄存器读数据
U16 dm9000_reg_readPHY(U16 reg)
{
	U16 data;
	//寄存器地址写到EPAR/PHY_AR（0CH）寄存器中,
	//注意将寄存器地址的第6位置1（地址与0x40或运算即可），
	///以表明写的是PHY地址，而不是EEPROM地址
	dm9000_reg_write(DM9000_EPAR, reg|0x40);
	//发送PHY读命令(0x0c）到EPCR/PHY_CR（0BH）寄存器中	
	dm9000_reg_write(DM9000_EPCR, 0x0c);
	//将数据高字节从PHY_DRH（0EH）寄存器中读出
	data = dm9000_reg_read(DM9000_EPDRH);
	//将数据低字节从PHY_DRL（0DH）寄存器中读出
	data = (data<<8) | dm9000_reg_read(DM9000_EPDRL);
	//延时20us，发送命令0x08到EPCR/PHY_CR（0BH）寄存器中，清除PHY读操作
	udelay(20);
	dm9000_reg_write(DM9000_EPCR, 0x08);
	return data;
}

unsigned char Buffer[2000];
static void int_issue(void *priv)
{
	U32 len;
    U8 status; 
    struct sk_buff *skb;

    if (!skb)
    {
    	printk("No More Sk_Buff\n");
    	panic();
    }

    status = dm9000_reg_read(DM9000_ISR);
    dm9000_reg_write(DM9000_ISR, status);	//清除接收中断标志位
   
   // printk("s: %x\n", status);
    if (status & 0x01)
	{
        do 
        {
            len= receivepacket(Buffer);
            if (len != 0)
            {
            	 skb = alloc_skbuff(len);
            	 if (!skb)
            	 {
            		 printk("No More SK_Buff\n");
            		 panic();
            	 }
            	 skb->ndev = priv;
            	 skb->data_len = len;
        		 memcpy(skb->data_buf, Buffer, len);
        		 netif_rx(skb);
            }
        }
        while (len> 0);
	}
	//ClearPending(BIT_EINT4_7);
	//EINTPEND |= 1 << 7;
}

int DM9000_sendPcket(struct sk_buff *skb);
struct net_device dm9000_dev =
{
	.ip 			 = 0xc0a80150,
	.netmask		 = 0xffffff00,
	.gw  			 = 0xc0a80101,
	.mtu			 = 1514,
	.hard_start_xmit = DM9000_sendPcket,
};

void IOSetInit(void)
{
	//EINT7
	GPFCON  &= ~(3 << 14);
	GPFCON  |= 2 << 14; //Set GPF7 To EINT7 Mode

	EXTINT0 = (EXTINT0 & (~(0x07 << 28))) | (0x01 << 28);//??EINT7??????
	EINTMASK &= ~(1 << 7);//??EINT7
	SRCPND = SRCPND | (0x1 << 4);
	INTPND = INTPND | (0x1 << 4);
	put_irq_handler(DM9000_IRQ, int_issue, &dm9000_dev);
	INTMSK &= ~(1 << 4);
}

void DM9000_init(char *mac_addr)
{
	U32 i;
	Test_DM9000AE();
	IOSetInit();
	//初始化设置步骤: 0
	dm9000_reg_write(DM9000_GPCR, 0x01);	//设置 GPCR(1EH) bit[0]=1，使DM9000的GPIO0为输出。
	dm9000_reg_write(DM9000_GPR,  0x00);	//GPR bit[0]=0 使DM9000的GPIO3输出为低以激活内部PHY。
	udelay(5000);							//延时2ms以上等待PHY上电。
	///初始化设置步骤: 1
	//	dm9000_reg_write(DM9000_NCR,  0x80);	//设置NCR[7]=1,激活外部PHY
	//	udelay(5000);							//延时2ms以上等待PHY上电
	//初始化设置步骤: 2
	dm9000_reg_write(DM9000_NCR,  0x03);	//软件复位
	udelay(30);								//延时20us以上等待软件复位完成
	dm9000_reg_write(DM9000_NCR,  0x00);	//复位完成，设置正常工作模式。
	dm9000_reg_write(DM9000_NCR,  0x03);	//第二次软件复位，为了确保软件复位完全成功。此步骤是必要的。
	udelay(30);
	dm9000_reg_write(DM9000_NCR,  0x00);
	//初始化设置步骤: 3
	dm9000_reg_write(DM9000_NSR,  0x2c);	//清除各种状态标志位
	dm9000_reg_write(DM9000_ISR,  0x3f);	//清除所有中断标志位
	//初始化设置步骤: 4
	dm9000_reg_write(DM9000_RCR,  0x39);	//接收控制
	dm9000_reg_write(DM9000_TCR,  0x00);	//发送控制
	dm9000_reg_write(DM9000_BPTR, 0x3f);	//背压门限
	dm9000_reg_write(DM9000_FCTR, 0x3a);	//接收FIFO门限3k 8k
	dm9000_reg_write(DM9000_FCR,  0xff);	//接收/发送溢出
	dm9000_reg_write(DM9000_SMCR, 0x00);	//特殊模式
	//初始化设置步骤: 5
	for(i = 0; i < 6; i++)
		dm9000_reg_write(DM9000_PAR + i, mac_addr[i]);//mac_addr[]自己定义一下吧，6个字节的MAC地址

	//初始化设置步骤: 6
	dm9000_reg_write(DM9000_NSR,  0x2c);	//清除各种状态标志位
	dm9000_reg_write(DM9000_ISR,  0x3f);	//清除所有中断标志位
	//初始化设置步骤: 7
	dm9000_reg_write(DM9000_IMR, 0x81);		//中断使能	

}
int DM9000_sendPcket(struct sk_buff *skb)
{
	U32 len,i;
	U8 tmp;
	U8 *datas;
	U32 length;

	datas  = skb->data_buf;
	length = skb->data_len;

	dm9000_reg_write(DM9000_IMR,0x80);		//先禁止网卡中断，防止在发送数据时被中断干扰	
	len = length;							//把发送长度写入
	dm9000_reg_write(DM9000_TXPLH, (len>>8) & 0x0ff);
	dm9000_reg_write(DM9000_TXPLL, len & 0x0ff);
	DM_ADD = DM9000_MWCMD;					//存储器读地址自动增加的读数据命令
	for(i=0; i<len; i+=2)					//16 bit mode
	{
		udelay(2);
		DM_CMD = datas[i] | (datas[i+1]<<8);
	}
	dm9000_reg_write(DM9000_TCR, 0x01);		//发送数据到以太网上

	while(1)//等待数据发送完成
	{
		U8 data;
		data = dm9000_reg_read(DM9000_TCR);//DM9000_NSR
		if((data&0x01) == 0x00) 
            break;
	}
	tmp = dm9000_reg_read(DM9000_NSR);

	if((tmp & 0x0f) == 0x04)
	{
		if((dm9000_reg_read(DM9000_TSR1)&0xfc) == 0x00)
        {
		
        }
		else
			printk("TSR1 Send Failed\n");
	}
	else
	{
		if((dm9000_reg_read(DM9000_TSR2)&0xfc) == 0x00)
        {
		
        }
		else
			printk("TSR2 Send Failed\n");
	}
	dm9000_reg_write(DM9000_NSR, 0x2c);		//清除状态寄存器，由于发送数据没有设置中断，因此不必处理中断标志位
	dm9000_reg_write(DM9000_IMR, 0x81);		//DM9000网卡的接收中断使能

	free_skbuff(skb);

	return 0;
}

U32 receivepacket(U8 *datas)
{
	U16 i,tmp,status,len;
    unsigned char GoodPacket;
	U8 ready;
	ready = 0;								//希望读取到"01H"
	status = 0;								//数据包状态
	len = 0; 								//数据包长度
    
        ready = dm9000_reg_read(DM9000_MRCMDX); // 第一次读取，一般读取到的是 00H
        if((ready & 0x0ff) != 0x01)
        {
            ready = dm9000_reg_read(DM9000_MRCMDX); // 第二次读取，总能获取到数据
            if((ready & 0x01) != 0x01)
            {
                /*
                   if((ready & 0x01) != 0x00) 		//若第二次读取到的不是 01H 或 00H ，则表示没有初始化成功
                   {
                   dm9000_reg_write(DM9000_IMR, 0x80);//屏蔽网卡中断
                   DM9000_init();				//重新初始化
                   dm9000_reg_write(DM9000_IMR, 0x81);//打开网卡中断
                   }
                   return 0;
                   */
                return 0;
            }
        }

        GoodPacket = 1;
        status = dm9000_reg_read(DM9000_MRCMD);
        len =  DM_CMD;

        status = status >> 8;
        if (status & 0xbf)
        {
            GoodPacket = 0;
            if (status & 0x01) 
                printk("<RX FIFO error>\n");
            if (status & 0x02) 
                printk("<RX CRC error>\n");
            if (status & 0x80) 
                printk("<RX Length error>\n");
            if (status & 0x08)
                printk("<Physical Layer error>\n");
            for (i = 0; i < len; i += 2)
                len = DM_CMD;
            return len;
        }

        if( (len <= 1522))//!(status & 0xbf) &&
        {
            for(i=0; i<len; i+=2)// 16 bit mode
            {
                udelay(20);
                tmp = DM_CMD;
                datas[i] = tmp & 0x0ff;
                datas[i + 1] = (tmp >> 8) & 0x0ff;
            }
        }
        else
            return 0;
#if 0
        printk("recv len: %d\n", len);
        for (i = 0; i < len; i++)
            printk("%x ", datas[i]);
        printk("\n");	
#endif
        return len;
}



void printkDM9000Reg(void)
{
	/*	U16 data,i;
		for(i=0x0;i<0x10;i++)
		{
		data = dm9000_reg_read(i);
		printk("RET_0x%02x = %x\r\n",i,data);
		}
		printk("\r\n");
		for(i=0x10;i<0x16;i++)
		{
		data = dm9000_reg_read(i);
		printk("RET_0x%02x = %02x\r\n",i,data);
		}

		i=0x16;
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = %x\r\n",i,data);

		for(i=0x1e;i<0x26;i++)
		{
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = %x\r\n",i,data);
		}

		for(i=0x28;i<0x2c;i++)
		{
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = 0x%x\r\n",i,data);
		}

		i=0x2f;
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = %x\r\n",i,data);

		for(i=0xf0;i<0xff;i++)
		{
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = %x\r\n",i,data);
		}
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = %x\r\n",i,data);

		data = dm9000_reg_read(0xfe);
		printk("处理器IO模式:%x\r\n",((U8)data)>>6);

		data = dm9000_reg_read(0x01);
		if(((U8)data)>>1)
		printk("10M以太网\r\n");
		else
		printk("100M以太网\r\n");

		data = dm9000_reg_read(0x02);
		printk("TCR（02H）：发送控制寄存器:%x\r\n",((U8)data));

		TestDm9000();
		*/
	U8 val, i;
	U16 data;
	printk("Registers:\n");
	val = dm9000_reg_read(DM9000_NCR);
	printk("NCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_NSR);
	printk("NSR = %02x(%03d)    ", val, val);
	val = dm9000_reg_read(DM9000_TCR);
	printk("TCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_RCR);
	printk("RCR = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_RSR);
	printk("RSR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_ROCR);
	printk("ROCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_BPTR);
	printk("BPTR = %02x(%03d)  ", val, val);
	val = dm9000_reg_read(DM9000_FCTR);
	printk("FCTR = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_FCR);
	printk("FCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_GPCR);
	printk("GPCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_GPR);
	printk("GPR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_WCR);
	printk("WCR = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_TRPAL);
	printk("TRPAL = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_TRPAH);
	printk("TRPAH = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_RWPAL);
	printk("RWPAL = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_RWPAH);
	printk("RWPAH = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_VIDL);
	printk("VIDL  = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_VIDH);
	printk("VIDH  = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_PIDL);
	printk("PIDL  = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_PIDH);
	printk("PIDH  = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_CHIPR);
	printk("CHIPR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_SMCR);
	printk("SMCR  = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_TXPLL);
	printk("TXPLL = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_TXPLH);
	printk("TXPLH = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_ISR);
	printk("ISR   = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_IMR);
	printk("IMR   = %02x(%03d) \n", val, val);

	printk("MAC = ");
	for(i = 0; i < 5; i++)
	{
		val = dm9000_reg_read(DM9000_PAR + i);
		printk(" %02x(%03d): ", val, val);
	}
	val = dm9000_reg_read(DM9000_PAR + i);
	printk(" %02x(%03d)\n ", val, val);
	///////////////////////////////

	printk("PHY Registers:\n");
	data = dm9000_reg_readPHY(DM9000_BMCR);
	printk(" BMCR   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_BMSR);
	printk("BMSR   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_PHYID1);
	printk("PHYID1 = %04x(%05d)\n ", data, data);

	data = dm9000_reg_readPHY(DM9000_PHYID2);
	printk("PHYID2 = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_ANAR);
	printk("ANAR   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_DSCR);
	printk("DSCR   = %04x(%05d)\n ", data, data);

	data = dm9000_reg_readPHY(DM9000_ANLPAR);
	printk("ANLPAR = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_ANER);
	printk("ANER   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_DSCSR);
	printk("DSCSR  = %04x(%05d)\n ", data, data);

	data = dm9000_reg_readPHY(DM9000_10BTCSR);
	printk("BTCSR  = %04x(%05d)\n ", data, data);

	//TestDm9000();
}

void TestDm9000(void)
{
	//DM9000_sendPcket(arpsendbuf,60);
	//arp_request();
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

int dm9000_module_init()
{
	char mac[6] = {0x00, 0x1c, 0x82, 0x00, 0x33, 0x1f};
	DM9000_init(mac);
	memcpy(dm9000_dev.macaddr, mac, 6);
	register_netdev(&dm9000_dev);
	return 0;
}

void dm9000_mdule_exit()
{
}







