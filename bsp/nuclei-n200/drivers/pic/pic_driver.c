// See LICENSE for license details.

#include "headers/devices/pic.h"
#include "pic/pic_driver.h"
#include "platform.h"
#include "encoding.h"
#include <string.h>


// Note that there are no assertions or bounds checking on these
// parameter values.

void PIC_set_threshold (
			 pic_threshold threshold){

  volatile pic_threshold* threshold_ptr = (pic_threshold*) (PIC_CTRL_ADDR +
                                                              PIC_THRESHOLD_OFFSET 
                                                              );

  *threshold_ptr = threshold;

}
  

void PIC_enable_interrupt (pic_source source){

  volatile pic_source * current_ptr = (volatile pic_source *)(PIC_CTRL_ADDR +
                                                        PIC_ENABLE_OFFSET +
                                                        ((source >> 3) & (~0x3))//Source number divide 32 and then multip 4 (bytes)
                                                        );
  pic_source current = *current_ptr;
  current = current | ( 1 << (source & 0x1f));// Only check the least 5 bits
  *current_ptr = current;

}

void PIC_disable_interrupt (pic_source source){
  
  volatile pic_source * current_ptr = (volatile pic_source *) (PIC_CTRL_ADDR +
                                                         PIC_ENABLE_OFFSET +
                                                         ((source >> 3) & (~0x3))//Source number divide 32 and then multip 4 (bytes)
                                                          );
  pic_source current = *current_ptr;
  current = current & ~(( 1 << (source & 0x1f)));// Only check the least 5 bits
  *current_ptr = current;
  
}

void PIC_set_priority (pic_source source, pic_priority priority){

  if (PIC_NUM_PRIORITIES > 0) {
    volatile pic_priority * priority_ptr = (volatile pic_priority *)
      (PIC_CTRL_ADDR +
       PIC_PRIORITY_OFFSET +
       (source << PIC_PRIORITY_SHIFT_PER_SOURCE));// Each priority reg occupy a word, so multiple 2
    *priority_ptr = priority;
  }
}

pic_source PIC_claim_interrupt(){
  

  volatile pic_source * claim_addr = (volatile pic_source * )
    (PIC_CTRL_ADDR +
     PIC_CLAIM_OFFSET 
     );

  return  *claim_addr;
  
}

pic_source PIC_check_eip(){
  

  volatile pic_source * eip_addr = (volatile pic_source * )
    (PIC_CTRL_ADDR +
     PIC_EIP_OFFSET 
     );

  return  *eip_addr;
  
}

void PIC_complete_interrupt(pic_source source){
  
  volatile pic_source * claim_addr = (volatile pic_source *) (PIC_CTRL_ADDR +
                                                                PIC_CLAIM_OFFSET  
                                                                );
  *claim_addr = source;
  
}

