TARGET = FreeRTOSv9.0.0
CFLAGS += -Os

BSP_BASE = ../../bsp


C_SRCS += Source/croutine.c  
C_SRCS += Source/list.c  
C_SRCS += Source/queue.c  
C_SRCS += Source/tasks.c  
C_SRCS += Source/timers.c  
C_SRCS += Source/event_groups.c  
C_SRCS += Source/portable/MemMang/heap_4.c  
C_SRCS += Source/portable/GCC/E203/port.c

C_SRCS += Demo/RISCV_E203_GCC/main.c
C_SRCS += $(BSP_BASE)/$(BOARD)/drivers/plic/plic_driver.c

INCLUDES += -ISource/include
INCLUDES += -IDemo/RISCV_E203_GCC
INCLUDES += -ISource/portable/GCC/E203



ASM_SRCS += Source/portable/GCC/E203/portasm.S

include $(BSP_BASE)/$(BOARD)/env/common.mk
