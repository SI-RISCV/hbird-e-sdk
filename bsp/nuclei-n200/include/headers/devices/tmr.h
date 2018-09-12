// See LICENSE for license details

#ifndef _NUCLEI_TMR_H
#define _NUCLEI_TMR_H


#define TMR_MSIP 0x0000
#define TMR_MSIP_size   0x4
#define TMR_MTIMECMP 0x0020
#define TMR_MTIMECMP_size 0x8
#define TMR_MTIME 0x0010
#define TMR_MTIME_size 0x8

#define TMR_CTRL_ADDR           0x02000000
#define TMR_REG(offset)         _REG32(TMR_CTRL_ADDR, offset)

#endif /* _NUCLEI_TMR_H */ 
