// See LICENSE file for licence details

#ifndef PIC_DRIVER_H
#define PIC_DRIVER_H


__BEGIN_DECLS

#include "platform.h"

typedef uint32_t pic_source;
typedef uint32_t pic_priority;
typedef uint32_t pic_threshold;

void PIC_init (
                uintptr_t         base_addr,
                uint32_t num_sources,
                uint32_t num_priorities
                );

void PIC_set_threshold (
			 pic_threshold threshold);
  
void PIC_enable_interrupt (
			    pic_source source);

void PIC_disable_interrupt (
			     pic_source source);
  
void PIC_set_priority (
			pic_source source,
			pic_priority priority);

pic_source PIC_claim_interrupt();

void PIC_complete_interrupt(
			     pic_source source);

pic_source PIC_check_eip();

// Structures for registering different interrupt handlers
// for different parts of the application.
typedef void (*function_ptr_t) (void);

// The interrupt 0 is empty
__attribute__((weak)) function_ptr_t pic_interrupt_handlers[PIC_NUM_INTERRUPTS];

__END_DECLS

#endif
