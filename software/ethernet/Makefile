TARGET =ethernet
CFLAGS += -Os -msave-restore  -fno-unroll-loops -flto

BSP_BASE = ../../bsp

C_SRCS += driver/eth/W5100.c
C_SRCS +=ethernet.c



INCLUDES += -Idriver/eth
include $(BSP_BASE)/$(BOARD)/env/common.mk
