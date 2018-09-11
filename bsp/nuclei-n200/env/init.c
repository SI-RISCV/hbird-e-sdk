//See LICENSE for license details.
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "platform.h"
#include "encoding.h"
#include "pic/pic_driver.h"

extern int main(int argc, char** argv);
extern void trap_entry();
extern void nmi_entry();
extern void irq_entry();


static unsigned long mtime_lo(void)
{
  return *(volatile unsigned long *)(TMR_CTRL_ADDR + TMR_MTIME);
}

#ifdef __riscv32

static uint32_t mtime_hi(void)
{
  return *(volatile uint32_t *)(TMR_CTRL_ADDR + TMR_MTIME + 4);
}

uint64_t get_timer_value()
{
  while (1) {
    uint32_t hi = mtime_hi();
    uint32_t lo = mtime_lo();
    if (hi == mtime_hi())
      return ((uint64_t)hi << 32) | lo;
  }
}

#else /* __riscv32 */

uint64_t get_timer_value()
{
  return mtime_lo();
}

#endif

unsigned long get_timer_freq()
{
  return 32768;
}

uint64_t get_instret_value()
{
#if __riscv_xlen == 32
  while (1) {
    uint32_t hi = read_csr(minstreth);
    uint32_t lo = read_csr(minstret);
    if (hi == read_csr(minstreth))
      return ((uint64_t)hi << 32) | lo;
  }
#else
  return read_csr(minstret);
#endif
}

uint64_t get_cycle_value()
{
#if __riscv_xlen == 32
  while (1) {
    uint32_t hi = read_csr(mcycleh);
    uint32_t lo = read_csr(mcycle);
    if (hi == read_csr(mcycleh))
      return ((uint64_t)hi << 32) | lo;
  }
#else
  return read_csr(mcycle);
#endif
}

static unsigned long __attribute__((noinline)) measure_cpu_freq(size_t n)
{
  unsigned long start_mtime, delta_mtime;
  unsigned long mtime_freq = get_timer_freq();

  // Don't start measuruing until we see an mtime tick
  unsigned long tmp = mtime_lo();
  do {
    start_mtime = mtime_lo();
  } while (start_mtime == tmp);

  unsigned long start_mcycle = read_csr(mcycle);

  do {
    delta_mtime = mtime_lo() - start_mtime;
  } while (delta_mtime < n);

  unsigned long delta_mcycle = read_csr(mcycle) - start_mcycle;

  return (delta_mcycle / delta_mtime) * mtime_freq
         + ((delta_mcycle % delta_mtime) * mtime_freq) / delta_mtime;
}

unsigned long get_cpu_freq()
{
  uint32_t cpu_freq;

  // warm up
  measure_cpu_freq(1);
  // measure for real
  cpu_freq = measure_cpu_freq(100);

  return cpu_freq;
}

static void uart_init(size_t baud_rate)
{
  GPIO_REG(GPIO_IOF_SEL) &= ~IOF0_UART0_MASK;
  GPIO_REG(GPIO_IOF_EN) |= IOF0_UART0_MASK;
  UART0_REG(UART_REG_DIV) = get_cpu_freq() / baud_rate - 1;
  UART0_REG(UART_REG_TXCTRL) |= UART_TXEN;
  UART0_REG(UART_REG_RXCTRL) |= UART_RXEN;
}




 __attribute__((weak)) void handle_m_time_interrupt()  {};

uintptr_t handle_trap(uintptr_t mcause)
{
  write(1, "trap\n", 5);
  _exit(1 + mcause);
  return 0;
}


/*Entry Point for PIC Interrupt Handler*/
void handle_irq(){
  pic_source int_num  = PIC_claim_interrupt();
    // Enable interrupts to allow interrupt preempt based on priority
    set_csr(mstatus, MSTATUS_MIE);
  pic_interrupt_handlers[int_num]();
    // Disable interrupts 
    clear_csr(mstatus, MSTATUS_MIE);
  PIC_complete_interrupt(int_num);
}


uintptr_t handle_nmi()
{
  write(1, "nmi\n", 5);
  _exit(1);
  return 0;
}


void _init()
{
  #ifndef NO_INIT
  uart_init(115200);

  printf("Core freq at %d Hz\n", get_cpu_freq());

  write_csr(mtvec, &trap_entry);
  //#define CSR_MIVEC 0x312
  //#define CSR_MNVEC 0x313
  write_csr(0x312, &irq_entry);
  write_csr(0x313, &nmi_entry);

   /**************************************************************************
   * Set up the PIC
   *
   *************************************************************************/

  #endif
}

void _fini()
{
}
