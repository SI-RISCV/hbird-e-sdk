// See LICENSE for license details.

#ifndef _NUCLEI_N200_H
#define _NUCLEI_N200_H

#include <stdint.h>

/****************************************************************************
 * GPIO Connections
 *****************************************************************************/

// These are the GPIO bit offsets for the RGB LED on Board.
// These are also mapped to RGB LEDs on the FPGA Dev Kit.


#define RED_LED_GPIO_OFFSET   19
#define GREEN_LED_GPIO_OFFSET 21
#define BLUE_LED_GPIO_OFFSET  22





#define RTC_FREQ 32768

void write_hex(int fd, unsigned long int hex);

#endif /* _NUCLEI_N200_H */
