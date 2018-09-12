// See LICENSE for license details

#ifndef ENTRY_S
#define ENTRY_S

#include "encoding.h"
#include "headers/bits.h"
#include "headers/devices/pic.h"



###############################################
###############################################
#Save caller registers
.macro SAVE_CONTEXT

  STORE x1,  0*REGBYTES(sp)
  STORE x5,  1*REGBYTES(sp)
  STORE x6,  2*REGBYTES(sp)
  STORE x7,  3*REGBYTES(sp)
  STORE x10, 4*REGBYTES(sp)
  STORE x11, 5*REGBYTES(sp)
  STORE x12, 6*REGBYTES(sp)
  STORE x13, 7*REGBYTES(sp)
  STORE x14, 8*REGBYTES(sp)
  STORE x15, 9*REGBYTES(sp)
  STORE x16, 10*REGBYTES(sp)
  STORE x17, 11*REGBYTES(sp)
  STORE x28, 12*REGBYTES(sp)
  STORE x29, 13*REGBYTES(sp)
  STORE x30, 14*REGBYTES(sp)
  STORE x31, 15*REGBYTES(sp)
.endm

###############################################
###############################################
# Save the mipc and istatus
#
.macro SAVE_MIPC_ISTATUS
  csrr x5, CSR_MIPC  
  STORE x5,  16*REGBYTES(sp)
  csrr x5, CSR_ISTATUS  
  STORE x5,  17*REGBYTES(sp)
.endm

#restore caller registers
.macro RESTORE_CONTEXT
  LOAD x1,  0*REGBYTES(sp)
  LOAD x5,  1*REGBYTES(sp)
  LOAD x6,  2*REGBYTES(sp)
  LOAD x7,  3*REGBYTES(sp)
  LOAD x10, 4*REGBYTES(sp)
  LOAD x11, 5*REGBYTES(sp)
  LOAD x12, 6*REGBYTES(sp)
  LOAD x13, 7*REGBYTES(sp)
  LOAD x14, 8*REGBYTES(sp)
  LOAD x15, 9*REGBYTES(sp)
  LOAD x16, 10*REGBYTES(sp)
  LOAD x17, 11*REGBYTES(sp)
  LOAD x28, 12*REGBYTES(sp)
  LOAD x29, 13*REGBYTES(sp)
  LOAD x30, 14*REGBYTES(sp)
  LOAD x31, 15*REGBYTES(sp)

.endm


###############################################
###############################################
# Restore the mipc and istatus
#
.macro RESTORE_MIPC_ISTATUS
  LOAD x5,  16*REGBYTES(sp)
  csrw CSR_MIPC, x5  
  LOAD x5,  17*REGBYTES(sp)
  csrw CSR_ISTATUS, x5  
.endm

.macro DISABLE_MIE
  csrc CSR_MSTATUS, MSTATUS_MIE  
.endm

.macro ENBLE_MIE
  csrs CSR_MSTATUS, MSTATUS_MIE  
.endm

###############################################
###############################################
// Trap entry point
//
  .section      .text.trap	
  .align 2
  .global trap_entry
trap_entry:
  // Allocate the stack space
  addi sp, sp, -16*REGBYTES
  // Save the caller saving registers (context)
  SAVE_CONTEXT
     // Set the function argument
  csrr a0, mcause
     // Call the function
  call handle_trap
  // Restore the caller saving registers (context)
  RESTORE_CONTEXT
  // De-allocate the stack space
  addi sp, sp, 16*REGBYTES
  // Return to regular code
  mret

###############################################
###############################################
// IRQ entry point
//
  .section      .text.irq	
  .align 2
  .global irq_entry
irq_entry:
  // Allocate the stack space
  addi sp, sp, -18*REGBYTES

  // Save the caller saving registers (context)
  SAVE_CONTEXT
  SAVE_MIPC_ISTATUS

  // Call the handler function
call_irq:
  call handle_irq

  // Check if there is pending irq to support interrupt tailing
  li x5, (PIC_CTRL_ADDR + PIC_EIP_OFFSET)// Use t0(x5) as the address register
  lw x5, 0(x5)// Read the value from PIC EIP reg
  bnez x5, call_irq // If the EIP reg is not zero, means there is a pending register

  // Restore the caller saving registers (context)
  RESTORE_MIPC_ISTATUS
  RESTORE_CONTEXT

  // De-allocate the stack space
  addi sp, sp, 18*REGBYTES
  // Return to regular code
  mret

###############################################
###############################################
// NMI entry point
//
  .section      .text.nmi	
  .align 2
  .global nmi_entry
nmi_entry:
  // Allocate the stack space
  addi sp, sp, -16*REGBYTES
  // Save the caller saving registers (context)
  SAVE_CONTEXT
     // Call the function
  call handle_nmi
  // Restore the caller saving registers (context)
  RESTORE_CONTEXT
  // De-allocate the stack space
  addi sp, sp, 16*REGBYTES
  // Return to regular code
  mret



###############################################
###############################################
.weak handle_trap
handle_trap:
1:
  j 1b

###############################################
###############################################
.weak handle_irq
handle_irq:
1:
  j 1b

###############################################
###############################################
.weak handle_nmi
handle_nmi:
1:
  j 1b
	
#endif
