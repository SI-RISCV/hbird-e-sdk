#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "stdatomic.h"

//#include "n200/drivers/n200_func.h"
//#include "soc/drivers/soc.h"
//#include "soc/drivers/board.h"
//#include "n200/drivers/riscv_encoding.h"
//#include "n200/drivers/n200_timer.h"
//#include "n200/drivers/n200_eclic.h"
//#include "n200/drivers/riscv_bits.h"
#include "plic/plic_driver.h"
#include "encoding.h"


#define SPI2_MOSI_GPIO_OFFSET 27
#define SPI2_MISO_GPIO_OFFSET 28

#define SPI2_SCK_GPIO_OFFSET   29
#define SPI2_SCKMODE_CPOL 1
#define SPI2_SCKMODE_CPHA 0
#define SPI2_CS_GPIO_OFFSET     26     //
#define SPI2_RESET_GPIO_OFFSET 25


#define S_RX_SIZE	2048		/*����Socket���ջ������Ĵ�С�����Ը���W5100_RMSR�������޸� */
#define S_TX_SIZE	2048  	/*����Socket���ͻ������Ĵ�С�����Ը���W5100_TMSR�������޸� */

//W5100 Register definition
#define	COMMON_BASE	0x0000			//Base address��������Ҫ�޸�

//-----------------------------------------------------------------------------
//                               Common register
//-----------------------------------------------------------------------------
//Mode register, R/W, Default=0x00
#define W5100_MODE	COMMON_BASE
	#define MODE_RST		0x80		//Software reset
	#define MODE_MT		0x20		//Memory test
	#define MODE_PB		0x10		//Ping block mode
	#define MODE_PPPOE	0x08		//PPOE mode
	#define MODE_LB		0x04		//Little_endian/Big_endian ordering in indirect bus I/F
	#define MODE_AI		0x02		//Address auto increment in indirect bus I/F
	#define MODE_IND		0x01		//Indirect bus I/F mode

//Gateway address register, R/W, default=0x00
#define	W5100_GAR	COMMON_BASE+0x01

//Subnet mask address, R/W, default=0x00
#define	W5100_SUBR	COMMON_BASE+0x05

//Source hardware address, R/W, default=0x00
#define	W5100_SHAR	COMMON_BASE+0x09

//Source IP address, R/W, default=0x00
#define	W5100_SIPR	COMMON_BASE+0x0f

//Interrupt and interrupt mask register
#define	W5100_IR	COMMON_BASE+0x15	//RO, Default=0x00
	#define IR_CONFLICT	0x80		//IP conflict
	#define IR_UNREACH	0x40		//Destination unreachable
	#define IR_PPPOE	0x20		//PPOE close
	#define IR_S3_INT	0x08		//Occurrence of socket 3 socket interrupt
	#define IR_S2_INT	0x04		//Occurrence of socket 2 socket interrupt
	#define IR_S1_INT	0x02		//Occurrence of socket 1 socket interrupt
	#define IR_S0_INT	0x01		//Occurrence of socket 0 socket interrupt

#define	W5100_IMR	COMMON_BASE+0x16	//R/W, Default=0x00
	#define IMR_CONFLICT	0x80		//IP conflict
	#define IMR_UNREACH	0x40		//Destination unreachable
	#define IMR_PPPOE	0x20		//PPOE close
	#define IMR_S3_INT	0x08		//Occurrence of socket 3 socket interrupt
	#define IMR_S2_INT	0x04		//Occurrence of socket 2 socket interrupt
	#define IMR_S1_INT	0x02		//Occurrence of socket 1 socket interrupt
	#define IMR_S0_INT	0x01		//Occurrence of socket 0 socket interrupt

//Retry time value. Value 1 means 100us, R/W, default=0x07D0
#define W5100_RTR	COMMON_BASE+0x17

//Retry count, R/W, Default=0x08
#define W5100_RCR	COMMON_BASE+0X19

//RX memory size register, R/W, default=0x55
//--------------------------------------------------------    S1      S0      Memory size
//  Socket3    |  Socket2    |  Socket1    |  Socket0    |    0       0           1KB
//-------------|-------------|-------------|-------------|    0       1           2KB
//S1       S0  |S1       S0  |S1       S0  |S1       S0  |    1       0           4KB
//--------------------------------------------------------    1       1           8KB
#define W5100_RMSR	COMMON_BASE+0x1a

//TX memory size register, R/W, default=0x55
#define W5100_TMSR	COMMON_BASE+0x1b

//Authentication type in PPPOE mode, R, default=0x0000
#define W5100_PATR	COMMON_BASE+0x1c

//PPP LCP request timer register, R/W, default=0x28
//Value 1 is about 25ms
#define W5100_PTIMER	COMMON_BASE+0x28

//PPP LCP magic number register, R/W, default=0x00
#define W5100_PMAGIC	COMMON_BASE+0x29

//Unreachable IP address, RO, default=0x00
#define W5100_UIPR	COMMON_BASE+0x2a

//Unreachable port register, RO, default=0x0000
#define W5100_UPORT	COMMON_BASE+0x2e


//-----------------------------------------------------------------------------
//                                  Socket register
//-----------------------------------------------------------------------------
//Socket mode register, R/W, default=0x00
#define W5100_S0_MR	COMMON_BASE+0x0400		//Socket 0
#define W5100_S1_MR	COMMON_BASE+0x0500		//Socket 1
#define W5100_S2_MR	COMMON_BASE+0x0600		//Socket 2
#define W5100_S3_MR	COMMON_BASE+0x0700		//Socket 3
	#define S_MR_MULTI	0x80		//Multcasting
	#define S_MR_MC		0x20		//Multcast
							//  P3   P2   P1   P0     Meaning
	#define S_MR_CLOSED	0x00		//  0    0    0    0      Closed
	#define S_MR_TCP		0x01		//  0    0    0    1      TCP
	#define S_MR_UDP		0x02		//  0    0    1    0      UDP
	#define S_MR_IPRAW	0x03		//  0    0    1    1      IPRAW
	#define S_MR_MACRAW	0x04		//  0    1    0    0      MACRAW
	#define S_MR_PPPOE	0x05		//  0    1    0    1      PPPOE

//Socket command register, R/W, default=0x00
#define W5100_S0_CR	COMMON_BASE+0x0401		//Socket 0
#define W5100_S1_CR	COMMON_BASE+0x0501		//Socket 1
#define W5100_S2_CR	COMMON_BASE+0x0601		//Socket 2
#define W5100_S3_CR	COMMON_BASE+0x0701		//Socket 3
	#define S_CR_OPEN	0x01		//It is used to initialize the socket
	#define S_CR_LISTEN	0x02		//In TCP mode, it waits for a connection request from any remote
						//peer(Client)
	#define S_CR_CONNECT	0x04		//In TCP mode, it sends a connection request to remote peer(SERVER)
	#define S_CR_DISCON	0x08		//In TCP mode, it sends a connection termination request
	#define S_CR_CLOSE	0x10		//Used to close the socket
	#define S_CR_SEND	0x20		//It transmit the data as much as the increase size of write pointer
	#define S_CR_SEND_MAC	0x21		//In UDP mode, same as SEND
	#define S_CR_SEND_KEEP	0x22		//In TCP mode
	#define S_CR_RECV	0x40		//Receiving is processed including the value of socket RX read
						//pointer register

	/* Definition for PPPOE */
	#define S_CR_PCON	0x23		//Start of ADSL connection
	#define S_CR_PDISCON	0x24		//End of ADSL connection
	#define S_CR_PCR	0x25		//Send REQ message ineach phase
	#define S_CR_PCN	0x26		//Send NAK message in each phase
	#define S_CR_PCJ	0x27		//Senf REJECT message in each phase

//Socket interrup register, RO, default=0x00
#define W5100_S0_IR	COMMON_BASE+0x0402		//Socket 0
#define W5100_S1_IR	COMMON_BASE+0x0502		//Socket 1
#define W5100_S2_IR	COMMON_BASE+0x0602		//Socket 2
#define W5100_S3_IR	COMMON_BASE+0x0702		//Socket 3
	#define S_IR_SENDOK	0x10		//Send data complete
	#define S_IR_TIMEOUT	0x08		//Set if timeout occurs during connection or termination
							//or data transmission
	#define S_IR_RECV		0x04		//Set if data is received
	#define S_IR_DISCON	0x02		//Set if receiv connection termination request
	#define S_IR_CON		0x01		//Set if connetion is established

	/* Definition for PPPOE */
	#define S_IR_PRECV	0x80		//Indicate receiving no support option data
	#define S_IR_PFAIL	0x40		//Indicate PAP Authentication fail
	#define S_IR_PNEXT	0x20		//Go next phase

//Socket status register, RO, default=0x00
#define W5100_S0_SSR	COMMON_BASE+0x0403		//Socket 0
#define W5100_S1_SSR	COMMON_BASE+0x0503		//Socket 1
#define W5100_S2_SSR	COMMON_BASE+0x0603		//Socket 2
#define W5100_S3_SSR	COMMON_BASE+0x0703		//Socket 3
	#define S_SSR_CLOSED	0x00		//In case that OPEN command are given to Sn_CR, Timeout interrupt
						//is asserted or connection is terminated
	#define S_SSR_INIT	0x13		//In case that Sn_MR is set as TCP and OPEN commands are given to Sn_CR
	#define S_SSR_LISTEN	0x14		//In case that under the SOCK_INIT status, LISTEN commands
						//are given to Sn_CR
	#define S_SSR_ESTABLISHED	0x17	//In case that connection is established
	#define S_SSR_CLOSE_WAIT	0x1c	//In case that connection temination request is received
	#define S_SSR_UDP	0x22		//In case that OPEN command is given to Sn_CR when Sn_MR is set as UDP
	#define S_SSR_IPRAW	0x32		//In case that OPEN command is given to Sn_CR when Sn_MR is set as IPRAW
	#define S_SSR_MACRAW	0x42		//In case that OPEN command is given to S0_CR when S0_MR is set as MACRAW
	#define S_SSR_PPPOE	0x5f		//In case that OPEN command is given to S0_CR when S0_MR is set as PPPOE
	//Below is shown in the status change, and does not need much attention
	#define S_SSR_SYNSEND	0x15
	#define S_SSR_SYNRECV	0x16
	#define S_SSR_FIN_WAIT	0x18
	#define S_SSR_CLOSING	0x1a
	#define S_SSR_TIME_WAIT	0x1b
	#define S_SSR_LAST_ACK	0x1d
	#define S_SSR_ARP0	0x11
	#define S_SSR_ARP1	0x21
	#define S_SSR_ARP2	0x31

//Socket Source port register, R/W, default=0x00
#define W5100_S0_PORT	COMMON_BASE+0x0404		//Socket 0
#define W5100_S1_PORT	COMMON_BASE+0x0504		//Socket 1
#define W5100_S2_PORT	COMMON_BASE+0x0604		//Socket 2
#define W5100_S3_PORT	COMMON_BASE+0x0704		//Socket 3

//Socket destination hardware address register, R/W, default=0x00
#define W5100_S0_DHAR	COMMON_BASE+0x0406		//Socket 0
#define W5100_S1_DHAR	COMMON_BASE+0x0506		//Socket 1
#define W5100_S2_DHAR	COMMON_BASE+0x0606		//Socket 2
#define W5100_S3_DHAR	COMMON_BASE+0x0706		//Socket 3

//Socket destination IP address register, R/W, default=0x00
#define W5100_S0_DIPR	COMMON_BASE+0x040c		//Socket 0
#define W5100_S1_DIPR	COMMON_BASE+0x050c		//Socket 1
#define W5100_S2_DIPR	COMMON_BASE+0x060c		//Socket 2
#define W5100_S3_DIPR	COMMON_BASE+0x070c		//Socket 3

//Socket destionation port register, R/W, default=0x00
#define W5100_S0_DPORT	COMMON_BASE+0x0410		//Socket 0
#define W5100_S1_DPORT	COMMON_BASE+0x0510		//Socket 1
#define W5100_S2_DPORT	COMMON_BASE+0x0610		//Socket 2
#define W5100_S3_DPORT	COMMON_BASE+0x0710		//Socket 3

//Socket maximum segment size register, R/W, default=0x00
#define W5100_S0_MSS	COMMON_BASE+0x0412		//Socket 0
#define W5100_S1_MSS	COMMON_BASE+0x0512		//Socket 1
#define W5100_S2_MSS	COMMON_BASE+0x0612		//Socket 2
#define W5100_S3_MSS	COMMON_BASE+0x0712		//Socket 3

//Socket IP protocol register, R/W, default=0x00
//See http://www.iana.org/assignments/protocol-number
#define W5100_S0_PROTO	COMMON_BASE+0x0414		//Socket 0
#define W5100_S1_PROTO	COMMON_BASE+0x0514		//Socket 1
#define W5100_S2_PROTO	COMMON_BASE+0x0614		//Socket 2
#define W5100_S3_PROTO	COMMON_BASE+0x0714		//Socket 3

//Socket IP type of servce register, R/W, default=0x00
#define W5100_S0_TOS	COMMON_BASE+0x0415		//Socket 0
#define W5100_S1_TOS	COMMON_BASE+0x0515		//Socket 1
#define W5100_S2_TOS	COMMON_BASE+0x0615		//Socket 2
#define W5100_S3_TOS	COMMON_BASE+0x0715		//Socket 3

//Socket IP time to live register, R/W, default=0x80
#define W5100_S0_TTL	COMMON_BASE+0x0416		//Socket 0
#define W5100_S1_TTL	COMMON_BASE+0x0516		//Socket 1
#define W5100_S2_TTL	COMMON_BASE+0x0616		//Socket 2
#define W5100_S3_TTL	COMMON_BASE+0x0716		//Socket 3

//Socket TX free size register, RO, default=0x0800
//should read upper byte first and lower byte later
#define W5100_S0_TX_FSR		COMMON_BASE+0x0420		//Socket 0
#define W5100_S1_TX_FSR		COMMON_BASE+0x0520		//Socket 1
#define W5100_S2_TX_FSR		COMMON_BASE+0x0620		//Socket 2
#define W5100_S3_TX_FSR		COMMON_BASE+0x0720		//Socket 3

//Socket TX read pointer register, RO, default=0x0000
//should read upper byte first and lower byte later
#define W5100_S0_TX_RR		COMMON_BASE+0x0422		//Socket 0
#define W5100_S1_TX_RR		COMMON_BASE+0x0522		//Socket 1
#define W5100_S2_TX_RR		COMMON_BASE+0x0622		//Socket 2
#define W5100_S3_TX_RR		COMMON_BASE+0x0722		//Socket 3

//Socket TX write pointer register, R/W, default=0x0000
//should read upper byte first and lower byte later
#define W5100_S0_TX_WR		COMMON_BASE+0x0424		//Socket 0
#define W5100_S1_TX_WR		COMMON_BASE+0x0524		//Socket 1
#define W5100_S2_TX_WR		COMMON_BASE+0x0624		//Socket 2
#define W5100_S3_TX_WR		COMMON_BASE+0x0724		//Socket 3

//Socket RX size register, RO, default=0x0000
//should read upper byte first and lower byte later
#define W5100_S0_RX_RSR		COMMON_BASE+0x0426		//Socket 0
#define W5100_S1_RX_RSR		COMMON_BASE+0x0526		//Socket 1
#define W5100_S2_RX_RSR		COMMON_BASE+0x0626		//Socket 2
#define W5100_S3_RX_RSR		COMMON_BASE+0x0726		//Socket 3

//Socket RX read pointer register, R/W, default=0x0000
//should read upper byte first and lower byte later
#define W5100_S0_RX_RR		COMMON_BASE+0x0428		//Socket 0
#define W5100_S1_RX_RR		COMMON_BASE+0x0528		//Socket 1
#define W5100_S2_RX_RR		COMMON_BASE+0x0628		//Socket 2
#define W5100_S3_RX_RR		COMMON_BASE+0x0728		//Socket 3

//Socket RX read pointer register, R/W, default=0x0000
//should read upper byte first and lower byte later
#define W5100_S0_RX_WR		COMMON_BASE+0x042A		//Socket 0
#define W5100_S1_RX_WR		COMMON_BASE+0x052A		//Socket 1
#define W5100_S2_RX_WR		COMMON_BASE+0x062A		//Socket 2
#define W5100_S3_RX_WR		COMMON_BASE+0x072A		//Socket 3

//TX memory
#define W5100_TX	COMMON_BASE+0x4000

//RX memory
#define W5100_RX	COMMON_BASE+0x6000



#define S_INIT	0x01
#define S_CONN	0x02
#define S_RECEIVE		0x01
#define S_TRANSMITOK	0x02
#define TRUE	0xff
#define FALSE	0x00

