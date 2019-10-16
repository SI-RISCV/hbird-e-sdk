// See LICENSE for license details.
/*
* Description: This demo is to show how to use the WIZnet W5100 Ethernet device for Hbird Development
* Fuction:  The W5100 module will actively connect to the server and send the data it resceives.
* W5100  is client by default                local IP : 192.168.0.177                  
* W5100  connects  remote  server by default  . Server IP : 192.168.0.100 Port : 5000
*
*  
*
*/




#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include <string.h>
#include "plic/plic_driver.h"
#include "encoding.h"
#include <unistd.h>
#include "stdatomic.h"

#include "driver/eth/W5100.h"


void SPI_INIT(void)
{

	GPIO_REG(GPIO_IOF_EN)       |=   ((0x1 << SPI2_MOSI_GPIO_OFFSET) |  (0x1 << SPI2_SCK_GPIO_OFFSET) |(0x1 << SPI2_MISO_GPIO_OFFSET));     //SPI2   IOF (0x1 << SPI2_CS_GPIO_OFFSET)
	GPIO_REG(GPIO_IOF_SEL)      &= ~ ((0x1 << SPI2_MOSI_GPIO_OFFSET) |  (0x1 << SPI2_SCK_GPIO_OFFSET)|(0x1 << SPI2_MISO_GPIO_OFFSET));   //SPI2   IOF0_SPI2_MASK	
  

	GPIO_REG(GPIO_INPUT_EN)     &= ~((0x1 << SPI2_MOSI_GPIO_OFFSET) |  (0x1 << SPI2_SCK_GPIO_OFFSET)|(0x1 << SPI2_CS_GPIO_OFFSET)|(0x1<<SPI2_RESET_GPIO_OFFSET));
	GPIO_REG(GPIO_OUTPUT_EN)    |=  ((0x1 << SPI2_MOSI_GPIO_OFFSET) |  (0x1 << SPI2_SCK_GPIO_OFFSET)|(0x1 << SPI2_CS_GPIO_OFFSET)|(0x1<<SPI2_RESET_GPIO_OFFSET));

  GPIO_REG(GPIO_INPUT_EN)     |=  (0x1 << SPI2_MISO_GPIO_OFFSET);
  GPIO_REG(GPIO_OUTPUT_EN)    &= ~(0x1 << SPI2_MISO_GPIO_OFFSET);


  SPI2_REG(SPI_REG_SCKMODE) &= ~((0x1 << SPI2_SCKMODE_CPOL)|(0x1 << SPI2_SCKMODE_CPHA));

	SPI2_REG(SPI_REG_SCKDIV)=0x08;


  GPIO_REG(GPIO_INPUT_EN)    &= ~((0x1<< RED_LED_GPIO_OFFSET) | (0x1<< GREEN_LED_GPIO_OFFSET) | (0x1 << BLUE_LED_GPIO_OFFSET)) ;
  GPIO_REG(GPIO_OUTPUT_EN)   |=  ((0x1<< RED_LED_GPIO_OFFSET)| (0x1<< GREEN_LED_GPIO_OFFSET) | (0x1 << BLUE_LED_GPIO_OFFSET)) ;

  GPIO_REG(GPIO_OUTPUT_VAL)  |=   (0x1 << RED_LED_GPIO_OFFSET) ;
  GPIO_REG(GPIO_OUTPUT_VAL)  &=  ~((0x1<< BLUE_LED_GPIO_OFFSET) | (0x1<< GREEN_LED_GPIO_OFFSET)) ;

}


void main(void)
{
  //SPI GPIO initialize
  SPI_INIT();
  
  //hardware rese
  GPIO_REG(GPIO_OUTPUT_VAL)  &=  ~(0x1 << SPI2_RESET_GPIO_OFFSET) ;

  Delay(5000);

  GPIO_REG(GPIO_OUTPUT_VAL)  |= (0x1 << SPI2_RESET_GPIO_OFFSET) ;
  
  //W5100 initialize
	W5100_Init();

  Socket_Init(0);

  Socket_Connect(0);

  while(1)
  {

  W5100_Interrupt_Process();

  }
      
}
