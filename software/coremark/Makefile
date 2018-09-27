TARGET := coremark

C_SRCS := \
	core_list_join.c \
	core_main.c \
	core_matrix.c \
	core_state.c \
	core_util.c \
	core_portme.c \

HEADERS := \
	coremark.h \
	core_portme.h \

#CFLAGS := -O2 -fno-common -funroll-loops -finline-functions --param max-inline-insns-auto=20 -falign-functions=4 -falign-jumps=4 -falign-loops=4
CFLAGS := -O2 -fno-common -funroll-loops -finline-functions -falign-functions=4 -falign-jumps=4 -falign-loops=4
#CFLAGS := -O2 -fno-common -funroll-loops -finline-functions -falign-functions=4 -falign-jumps=4 -falign-loops=4 -funswitch-loops 
#CFLAGS := -O2 -fno-common -funroll-loops -finline-functions -falign-functions=4 -falign-jumps=4 -falign-loops=4 -fpeel-loops 
#CFLAGS := -O2 -fno-common -funroll-loops -finline-functions -falign-functions=4 -falign-jumps=4 -falign-loops=4 -funswitch-loops -fpeel-loops 
#CFLAGS := -O3 -fno-common -funroll-loops -finline-functions  -falign-functions=4 -falign-jumps=4 -falign-loops=4 -funswitch-loops -fpeel-loops -fgcse-sm -fgcse-las 
CFLAGS += -DFLAGS_STR=\""$(CFLAGS)"\"
CFLAGS += -DITERATIONS=10000 -DPERFORMANCE_RUN=1

BSP_BASE = ../../bsp
include $(BSP_BASE)/$(BOARD)/env/common.mk
