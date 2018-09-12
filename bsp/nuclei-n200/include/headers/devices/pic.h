// See LICENSE for license details.

#ifndef PIC_H
#define PIC_H

#include <headers/const.h>

// 32 bits per source
#define PIC_PRIORITY_OFFSET            _AC(0x0000,UL)
#define PIC_PRIORITY_SHIFT_PER_SOURCE  2
// 1 bit per source (1 address)
#define PIC_PENDING_OFFSET             _AC(0x1000,UL)

//0x80 per target
#define PIC_ENABLE_OFFSET              0x1100
#define PIC_THRESHOLD_OFFSET           0x1200
#define PIC_CLAIM_OFFSET               0x1204
#define PIC_EIP_OFFSET                 0x1208

#define PIC_MAX_SOURCE                 1023
#define PIC_SOURCE_MASK                0x3FF

#define PIC_CTRL_ADDR           0x0C000000
#define PIC_REG(offset)         _REG32(PIC_CTRL_ADDR, offset)
#define PIC_NUM_PRIORITIES 7

#define PIC_INT_RESERVED    0
#define PIC_INT_SFT         1
#define PIC_INT_TMR         2

#endif /* PIC_H */
