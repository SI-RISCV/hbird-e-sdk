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
#define PIC_ENABLE_OFFSET              _AC(0x1100,UL)
#define PIC_THRESHOLD_OFFSET           _AC(0x1200,UL)
#define PIC_CLAIM_OFFSET               _AC(0x1204,UL)

#define PIC_MAX_SOURCE                 1023
#define PIC_SOURCE_MASK                0x3FF


#endif /* PIC_H */
