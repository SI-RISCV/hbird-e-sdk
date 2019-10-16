/**
  ******************************************************************************
  * @file   W5100.c
    *             本文件包括5个部分：
  *  	1. W5100初始化
  *  	16. W5100的Socket初始化
  *  	3. Socket连接
  *  	  
  *  	   Socket设置为TCP客户端模式，则调用Socket_Connect()函数，
  *  	                                  每调用一次Socket_Connect(s)函数，产生一次连接，
  *  	                                  如果连接不成功，则产生超时中断，然后可以再调用该函数进行连接。
  *  	   
  *  	4. Socket数据接收和发送
  *  	5. W5100中断处理
  *
  * 
  *  置W5100为客户端模式的调用过程：W5100_Init()-->Socket_Init(s)-->Socket_Connect(s)，设置过程即完成，并与远程服务器连接。
  *  
  *  注意spi的双向连接。
  *
  *  W5100产生的连接成功、终止连接、接收数据、发送数据、超时等事件，都可以从中断状态中获得。
  ******************************************************************************
  */

 #include "W5100.h"				/* 定义W5100的寄存器地址、状态 */

typedef uint8_t SOCKET;


/* 端口数据缓冲区 */
uint8_t Rx_Buffer[1024];			/* 端口接收数据缓冲区 */
uint8_t Tx_Buffer[1024];			/* 端口发送数据缓冲区 */


/* 网络参数寄存器 */
uint8_t Gateway_IP[4]={192,168,0,1};	     		/* Gateway IP Address */  
uint8_t Sub_Mask[4]={255,255,255,0};				/* Subnet Mask */
uint8_t Phy_Addr[6]={0x00,0x08,0xDC,0x01,0x02,0x23};  			/* Physical Address */
uint8_t IP_Addr[4]={192,168,0,177};				/* Loacal IP Address */
uint8_t IP_Add_read[4]={0};			

uint8_t S0_Port[2]={0x13,0x88}; 			/* Socket0 Port number 5000 */
uint8_t S0_DIP[4]={192,168,0,100};				/* Socket0 Destination IP Address */
uint8_t S0_DPort[2]={0x13,0x88}; 				/* Socket0 Destination Port number 5000*/


uint8_t S0_State=0;				/* Socket0 state recorder */

uint8_t S0_Data;			/* Socket0 receive data and transmit OK */

uint8_t W5100_Interrupt;

/* UDP Destionation IP address and Port number */
uint8_t UDP_DIPR[4];
uint8_t UDP_DPORT[2];



#define SPI_CS_0  GPIO_REG(GPIO_OUTPUT_VAL) &=~(0x1 << SPI2_CS_GPIO_OFFSET)
#define SPI_CS_1  GPIO_REG(GPIO_OUTPUT_VAL) |=(0x1 << SPI2_CS_GPIO_OFFSET)


//#define SPI_SI  GPIO_REG(GPIO_INPUT_VAL) &(0x01<<SPI2_MOSI_GPIO_OFFSET)

void Delay(unsigned int  t)
{
			volatile unsigned int  repeatCount = 0;
      repeatCount=t;
  while (repeatCount--) ;
}


uint8_t SPI_Send_ReadByte(uint8_t dat)   //SPI send&read数据
{	
    SPI2_REG(SPI_REG_TXFIFO) = dat;
		Delay(50);// Prevent packet loss of the first frame
		return SPI2_REG(SPI_REG_RXFIFO)&0xff;
}

uint8_t Read_W5100(uint16_t addr)
{
	uint8_t i=0;
 
	/* 置W5100的CS为低电平 */
  SPI_CS_0; 

	Delay(50);
	/* 发送读命令 */
	SPI_Send_ReadByte(0x0f);

	/* 发送地址 */
	SPI_Send_ReadByte(addr>> 8);
	SPI_Send_ReadByte(addr& 0xFF);

	/* 读取数据 */
	i=SPI_Send_ReadByte(0);

	Delay(50);
	/* 置W5100的CS为高电平 */
	SPI_CS_1;
	return i;
}

void Write_W5100(uint16_t addr, uint8_t dat)
{

	/* 置W5100的CS为低电平 */
	SPI_CS_0;
  Delay(50);
	/* 发送写命令 */
	SPI_Send_ReadByte(0xf0);

	/* 发送地址 */
	SPI_Send_ReadByte(addr>> 8);
	SPI_Send_ReadByte(addr& 0xFF);

	/* 写入数据 */
	SPI_Send_ReadByte(dat);;

  Delay(50);
	/* 置W5100的CS为高电平 */
  SPI_CS_1;
}


void W5100_Init(void)
{
	uint8_t i;

	Write_W5100(W5100_MODE,MODE_RST);		/*软复位W5100*/

	 Delay(1500);						/*延时，自己定义该函数*/

	/*设置网关(Gateway)的IP地址，4字节 */
	/*使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet*/
	for(i=0;i<4;i++)
		Write_W5100(W5100_GAR+i,Gateway_IP[i]);			/*Gateway_IP为4字节uint8_t数组,自己定义*/
    
	/*设置子网掩码(MASK)值，4字节。子网掩码用于子网运算*/
	for(i=0;i<4;i++)
		Write_W5100(W5100_SUBR+i,Sub_Mask[i]);			/*SUB_MASK为4字节uint8_t数组,自己定义*/

	/*设置物理地址，6字节，用于唯一标识网络设备的物理地址值
	该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
	如果自己定义物理地址，注意第一个字节必须为偶数*/
	for(i=0;i<6;i++)
		Write_W5100(W5100_SHAR+i,Phy_Addr[i]);			/*PHY_ADDR6字节uint8_t数组,自己定义*/

	/*设置本机的IP地址，4个字节
	注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关*/
	for(i=0;i<4;i++)
		Write_W5100(W5100_SIPR+i,IP_Addr[i]);			/*IP_ADDR为4字节uint8_t数组,自己定义*/
			
	/*设置发送缓冲区和接收缓冲区的大小，参考W5100数据手册*/
	Write_W5100(W5100_RMSR,0x55);		/*Socket Rx memory size=2k*/
	Write_W5100(W5100_TMSR,0x55);		/*Socket Tx mempry size=2k*/

	/* 设置重试时间，默认为2000(200ms) */
	Write_W5100(W5100_RTR,0x07);
	Write_W5100(W5100_RTR+1,0xd0);

	/* 设置重试次数，默认为8次 */
	Write_W5100(W5100_RCR,8);

	/* 启动中断，参考W5100数据手册确定自己需要的中断类型
	IMR_CONFLICT是IP地址冲突异常中断
	IMR_UNREACH是UDP通信时，地址无法到达的异常中断
	其它是Socket事件中断，根据需要添加 */
	Write_W5100(W5100_IMR,(IMR_CONFLICT|IMR_UNREACH|IMR_S0_INT));

		printf("W5100 init finish\n");
}

uint8_t Detect_Gateway(void)
{
	uint8_t i;

	Write_W5100((W5100_S0_MR),S_MR_TCP);		/*设置socket0为TCP模式*/

	Write_W5100((W5100_S0_CR),S_CR_OPEN);		/*打开socket0*/

	if(Read_W5100(W5100_S0_SSR)!=S_SSR_INIT)
	{
		Write_W5100((W5100_S0_CR),S_CR_CLOSE);	/*打开不成功，关闭Socket，然后返回*/
		return FALSE;
	}

	/*检查网关及获取网关的物理地址*/
	for(i=0;i<4;i++)
		Write_W5100((W5100_S0_DIPR+i),IP_Addr[i]+1);	/*向目的地址寄存器写入与本机IP不同的IP值*/

	Write_W5100((W5100_S0_CR),S_CR_CONNECT);		/*打开socket0的TCP连接*/

	Delay(20);						/* 延时20ms */

	i=Read_W5100(W5100_S0_DHAR);			/*读取目的主机的物理地址，该地址就是网关地址*/

	Write_W5100((W5100_S0_CR),S_CR_CLOSE);			/*关闭socket0*/

	if(i==0xff)
	{
		/**********没有找到网关服务器，或没有与网关服务器成功连接***********/
		/**********              自己添加处理代码                ***********/
		return FALSE;
	}
	return TRUE;
}

void Socket_Init(SOCKET s)
{
	unsigned int i;

	/*设置分片长度，参考W5100数据手册，该值可以不修改*/
	Write_W5100((W5100_S0_MSS+s*0x100),0x00);		/*最大分片字节数=16*/
	Write_W5100((W5100_S0_MSS+s*0x100+1),0x10);

	/* Set Socket Port number */
	switch(s)
	{
		case 0:
			Write_W5100(W5100_S0_PORT,S0_Port[0]);	/* Set Local Socket Port number */
			Write_W5100(W5100_S0_PORT+1,S0_Port[1]);

			Write_W5100(W5100_S0_DPORT,S0_DPort[0]);	/* Set Destination port number */
			Write_W5100(W5100_S0_DPORT+1,S0_DPort[1]);

			for(i=0;i<4;i++)
				Write_W5100(W5100_S0_DIPR+i,S0_DIP[i]);	/* Set Destination IP Address */
			break;
		case 1:
			break;
		case 2:

			break;
		case 3:

			break;
		default:
			break;
	}
}

/**
 * @brief  设置Socket为客户端与远程服务器连接
 *当本机Socket工作在客户端模式时，引用该程序，与远程服务器建立连接
 *
 *如果启动连接后出现超时中断，则与服务器连接失败，需要重新调用该程序连接
 *该程序每调用一次，就与服务器产生一次连接
 **/

uint8_t Socket_Connect(SOCKET s)
{
	Write_W5100((W5100_S0_MR+s*0x100), S_MR_TCP);		/*设置socket为TCP模式 */
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_OPEN);		/*打开Socket*/


	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_INIT)
	{
		uint8_t value = Read_W5100(W5100_S0_SSR+s*0x100);

		Write_W5100(W5100_S0_CR+s*0x100,S_CR_CLOSE);	/*打开不成功，关闭Socket，然后返回*/
		printf("Socket_Connect fail\n");
		return FALSE;
	}

	Write_W5100((W5100_S0_CR+s*0x100),S_CR_CONNECT);		/*设置Socket为Connect模式*/

	return TRUE;

	/*至此完成了Socket的打开连接工作，至于它是否与远程服务器建立连接，则需要等待Socket中断，
	以判断Socket的连接是否成功。参考W5100数据手册的Socket中断状态*/
}

/**
 * @brief  设置Socket作为服务器等待远程主机的连接
 *当本机Socket工作在服务器模式时，引用该程序，等等远程主机的连接
 *
 *该程序只调用一次，就使W5100设置为服务器模式
 * @return 如果设置成功则返回true, 否则返回false
 **/

uint8_t Socket_Listen(SOCKET s)
{
	Write_W5100((W5100_S0_MR+s*0x100), S_MR_TCP);		/*设置socket为TCP模式 */
        Write_W5100(W5100_S0_PORT,S0_Port[0]);	/* Set source Socket Port number */
	Write_W5100(W5100_S0_PORT+1,S0_Port[1]);
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_OPEN);		/*打开Socket*/

	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_INIT)
	{
		Write_W5100((W5100_S0_CR+s*0x100),S_CR_CLOSE);	/*打开不成功，关闭Socket，然后返回*/
		return FALSE;
	}

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_LISTEN);		/*设置Socket为侦听模式*/

	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_LISTEN)
	{
		Write_W5100((W5100_S0_CR+s*0x100), S_CR_CLOSE);		/*设置不成功，关闭Socket，然后返回*/
		return FALSE;
	}

	return TRUE;

	/*至此完成了Socket的打开和设置侦听工作，至于远程客户端是否与它建立连接，则需要等待Socket中断，
	以判断Socket的连接是否成功。参考W5100数据手册的Socket中断状态
	在服务器侦听模式不需要设置目的IP和目的端口号*/
}

/**
 * @brief  	设置Socket为UDP模式
 *如果Socket工作在UDP模式，引用该程序。在UDP模式下，Socket通信不需要建立连接
 *该程序只调用一次，就使W5100设置为UDP模式
 * @return 如果设置成功则返回true, 否则返回false
 **/


uint8_t Socket_UDP(SOCKET s)
{
	Write_W5100((W5100_S0_MR+s*0x100), S_MR_UDP);		/*设置Socket为UDP模式*/
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_OPEN);		/*打开Socket*/

	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_UDP)
	{
		Write_W5100((W5100_S0_CR+s*0x100), S_CR_CLOSE);	/*打开不成功，关闭Socket，然后返回*/
		return FALSE;
	}
	else
		return TRUE;

	/*至此完成了Socket的打开和UDP模式设置，在这种模式下它不需要与远程主机建立连接
	因为Socket不需要建立连接，所以在发送数据前都可以设置目的主机IP和目的Socket的端口号
	如果目的主机IP和目的Socket的端口号是固定的，在运行过程中没有改变，那么也可以在这里设置*/
}

/**
 * @brief  	 处理Socket接收和发送的数据
 *如果Socket产生接收数据的中断，则引用该程序进行处理
 *该程序将Socket的接收到的数据缓存到Rx_buffer数组中，并返回接收的数据字节数
  * @return 数据长度
 **/

unsigned short S_rx_process(SOCKET s)
{
	unsigned short i,j;
	unsigned short rx_size,rx_offset;
	

	/*读取接收数据的字节数*/
	rx_size=Read_W5100(W5100_S0_RX_RSR+s*0x100);
	rx_size*=256;
	rx_size+=Read_W5100(W5100_S0_RX_RSR+s*0x100+1);

	/*读取接收缓冲区的偏移量*/
	rx_offset=Read_W5100(W5100_S0_RX_RR+s*0x100);
	rx_offset*=256;
	rx_offset+=Read_W5100(W5100_S0_RX_RR+s*0x100+1);


	i=rx_offset/S_RX_SIZE;				/*计算实际的物理偏移量，S0_RX_SIZE需要在前面#define中定义*/
								/*注意S_RX_SIZE的值在W5100_Init()函数的W5100_RMSR中确定*/
	rx_offset=rx_offset-i*S_RX_SIZE;

	j=W5100_RX+s*S_RX_SIZE+rx_offset;		/*实际物理地址为W5100_RX+rx_offset*/

	for(i=0;i<rx_size;i++)
	{
		if(rx_offset>=S_RX_SIZE)
		{
			j=W5100_RX+s*S_RX_SIZE;
			rx_offset=0;
		}
		Rx_Buffer[i]=Read_W5100(j);		/*将数据缓存到Rx_buffer数组中*/

		j++;
		rx_offset++;
	}

	/*计算下一次偏移量*/
	rx_offset=Read_W5100(W5100_S0_RX_RR+s*0x100);
	rx_offset*=256;
	rx_offset+=Read_W5100(W5100_S0_RX_RR+s*0x100+1);

	rx_offset+=rx_size;
	Write_W5100((W5100_S0_RX_RR+s*0x100), (rx_offset/256));
	Write_W5100((W5100_S0_RX_RR+s*0x100+1), rx_offset);

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_RECV);			/*设置RECV命令，等等下一次接收*/

	return rx_size;								/*返回接收的数据字节数*/
}

/**
 * @brief  	如果要通过Socket发送数据，则引用该程序
 *要发送的数据缓存在Tx_buffer中, size则是要发送的字节长度
 * @author  stmsky
 * @param[in] socket nummber
 * @param[out] none
 * @return
 * @note
 **/

uint8_t S_tx_process(SOCKET s, unsigned int size)
{
	unsigned short i,j;
	unsigned short tx_free_size,tx_offset;

	/*如果是UDP模式,可以在此设置目的主机的IP和端口号*/
	if((Read_W5100(W5100_S0_MR+s*0x100)&0x0f)==0x02)
	{
		for(i=0;i<4;i++)			/* 设置目的主机IP*/
			Write_W5100((W5100_S0_DIPR+s*0x100+i), UDP_DIPR[i]);

		Write_W5100((W5100_S0_DPORT+s*0x100), UDP_DPORT[0]);
		Write_W5100((W5100_S0_DPORT+s*0x100+1), UDP_DPORT[1]);
	}

	/*读取缓冲区剩余的长度*/
	tx_free_size=Read_W5100(W5100_S0_TX_FSR+s*0x100);
	tx_free_size*=256;
	tx_free_size+=Read_W5100(W5100_S0_TX_FSR+s*0x100+1);
	if(tx_free_size<size)						/*如果剩余的字节长度小于发送字节长度,则返回*/
		return FALSE;

	/*读取发送缓冲区的偏移量*/
	tx_offset=Read_W5100(W5100_S0_TX_WR+s*0x100);
	tx_offset*=256;
	tx_offset+=Read_W5100(W5100_S0_TX_WR+s*0x100+1);

	i=tx_offset/S_TX_SIZE;					/*计算实际的物理偏移量，S0_TX_SIZE需要在前面#define中定义*/
									/*注意S0_TX_SIZE的值在W5100_Init()函数的W5100_TMSR中确定*/
	tx_offset=tx_offset-i*S_TX_SIZE;
	j=W5100_TX+s*S_TX_SIZE+tx_offset;		/*实际物理地址为W5100_TX+tx_offset*/

	for(i=0;i<size;i++)
	{
		if(tx_offset>=S_TX_SIZE)
		{
			j=W5100_TX+s*S_TX_SIZE;
			tx_offset=0;
		}
		Write_W5100(j,Tx_Buffer[i]);						/*将Tx_buffer缓冲区中的数据写入到发送缓冲区*/
		j++;
		tx_offset++;
	}

	/*计算下一次的偏移量*/
	tx_offset=Read_W5100(W5100_S0_TX_WR+s*0x100);
	tx_offset*=256;
	tx_offset+=Read_W5100(W5100_S0_TX_WR+s*0x100+1);


	tx_offset+=size;
	Write_W5100((W5100_S0_TX_WR+s*0x100),(tx_offset/256));
	Write_W5100((W5100_S0_TX_WR+s*0x100+1),tx_offset);

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_SEND);			/*设置SEND命令,启动发送*/

	return TRUE;								/*返回成功*/
}

/**
 * @brief  	W5100中断处理程序框架
 * @note
 **/
void vPrintString(uint8_t *ucpString)
{
   unsigned int  i=0;
   while(ucpString[i]!=0x00)
   	{
   	   Tx_Buffer[i]=ucpString[i];
   	   i++;
   	}
   S_tx_process(0,i);
}
void W5100_Interrupt_Process(void)
{
	uint8_t i,j;
	unsigned int  count=0;
	W5100_Interrupt=0;

	i=Read_W5100(W5100_IR);

	Write_W5100(W5100_IR, (i&0xf0));					/*回写清除中断标志*/
    if((i & IR_CONFLICT) == IR_CONFLICT)	 	/*IP地址冲突异常处理，自己添加代码*/
	{
     printf("IP CONFLECT\n");
	}

	if((i & IR_UNREACH) == IR_UNREACH)			/*UDP模式下地址无法到达异常处理，自己添加代码*/
	{
		 printf("IP UNREACH\n");
	}

	/* Socket事件处理 */
	if((i & IR_S0_INT) == IR_S0_INT)
	{
		j=Read_W5100(W5100_S0_IR);
		Write_W5100(W5100_S0_IR, j);		/* 回写清中断标志 */

		if(j&S_IR_CON)				/* 在TCP模式下,Socket0成功连接 */
		{
			S0_State|=S_CONN;
			vPrintString("SOCKET0 opened.");
		}
		if(j&S_IR_DISCON)				/* 在TCP模式下Socket断开连接处理，自己添加代码 */
		{
			Write_W5100(W5100_S0_CR, S_CR_CLOSE);		/* 关闭端口，等待重新打开连接 */
			Socket_Connect(0);
			vPrintString("Socket_Connect reopen\n ");
			S0_State=0;
		}

		if(j&S_IR_SENDOK)				/* Socket0数据发送完成，可以再次启动S_tx_process()函数发送数据 */
		{
			S0_Data|=S_TRANSMITOK;
		}
		if(j&S_IR_RECV)				/* Socket接收到数据，可以启动S_rx_process()函数 */
		{
			S0_Data|=S_RECEIVE;
			
			//for(j=0;j<i;j++)
			i=S_rx_process(0); 

			
			for(j=0;j<i;j++)
			   Tx_Buffer[j]=Rx_Buffer[j];			/* 端口接收数据缓冲区 */
            S_tx_process(0,i);
			
		
		}
		if(j&S_IR_TIMEOUT)			/* Socket连接或数据传输超时处理 */
		{
			Write_W5100(W5100_S0_CR, S_CR_CLOSE);		/* 关闭端口，等待重新打开连接 */
		    Socket_Connect(0);
			S0_State=0;
		}
	}
   	
	/* Socket1事件处理 */
	if((i&IR_S1_INT)==IR_S1_INT)
	{
		j=Read_W5100(W5100_S1_IR);
		Write_W5100(W5100_S1_IR, j);		/* 回写清中断标志 */
	}

	/* Socket2事件处理 */
	if((i&IR_S2_INT)==IR_S2_INT)
	{
		j=Read_W5100(W5100_S2_IR);
		Write_W5100(W5100_S2_IR, j);		/*回写清中断标志 */

	}

	/* Socket3事件处理 */
	if((i&IR_S3_INT)==IR_S3_INT)
	{
		j=Read_W5100(W5100_S3_IR);
		Write_W5100(W5100_S3_IR, j);		/* 回写清中断标志 */
	}
}


