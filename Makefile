#############################################################
# Configuration
#############################################################

# Allows users to create Makefile.local or ../Makefile.project with
# configuration variables, so they don't have to be set on the command-line
# every time.
extra_configs := $(wildcard Makefile.local ../Makefile.project)
ifneq ($(extra_configs),)
$(info Obtaining additional make variables from $(extra_configs))
include $(extra_configs)
endif

FLASHXIP    := flashxip
FLASH       := flash
ITCM        := itcm
IECM        := iecm
DOWNLOAD    := flash

FTDI   := ftdi
HBIRD  := hbird
OLMX   := olmx
OCDCFG := ${HBIRD}

E201        := e201
E203        := e203
E205        := e205
E205F       := e205f
E205FD      := e205fd
E225        := e225
E225FD      := e225fd

CORE        := e203


N201        := n201
N203        := n203
N205        := n205
N205F       := n205f
N205FD      := n205fd


CORE_NAME = $(shell echo $(CORE) | tr a-z A-Z)
core_name = $(shell echo $(CORE) | tr A-Z a-z)

REPLACE_PRINTF  := 0
USE_NANO        := 1
NANO_PFLOAT     := 1

ifeq ($(core_name),${E201}) 
RISCV_ARCH := rv32iac
RISCV_ABI  := ilp32
endif

ifeq ($(core_name),${E203}) 
RISCV_ARCH := rv32imac
RISCV_ABI  := ilp32
endif

ifeq ($(core_name),${E205}) 
RISCV_ARCH := rv32imac
RISCV_ABI  := ilp32
endif

ifeq ($(core_name),${E205F}) 
RISCV_ARCH := rv32imafc
RISCV_ABI  := ilp32f
endif

ifeq ($(core_name),${E205FD}) 
RISCV_ARCH := rv32imafdc
RISCV_ABI  := ilp32f
endif

ifeq ($(core_name),${E225FD}) 
RISCV_ARCH := rv32imafdc
RISCV_ABI  := ilp32f
endif

ifeq ($(core_name),${N201}) 
RISCV_ARCH := rv32iac
RISCV_ABI  := ilp32
endif

ifeq ($(core_name),${N203}) 
RISCV_ARCH := rv32imac
RISCV_ABI  := ilp32
endif

ifeq ($(core_name),${N205}) 
RISCV_ARCH := rv32imac
RISCV_ABI  := ilp32
endif

ifeq ($(core_name),${N205F}) 
RISCV_ARCH := rv32imafc
RISCV_ABI  := ilp32f
endif

ifeq ($(core_name),${N205FD}) 
RISCV_ARCH := rv32imafdc
RISCV_ABI  := ilp32f
endif


# Default target
BOARD ?= hbird-e200
PROGRAM ?= demo_gpio
GDB_PORT ?= 3333

# Variables the user probably shouldn't override.
builddir := work/build
installdir := work/install
toolchain_srcdir := riscv-gnu-toolchain
openocd_srcdir := openocd

#############################################################
# BSP Loading
#############################################################

# Finds the directory in which this BSP is located, ensuring that there is
# exactly one.
board_dir := $(wildcard bsp/$(BOARD))
ifeq ($(words $(board_dir)),0)
$(error Unable to find BSP for $(BOARD), expected to find either "bsp/$(BOARD)" or "bsp-addons/$(BOARD)")
endif
ifneq ($(words $(board_dir)),1)
$(error Found multiple BSPs for $(BOARD): "$(board_dir)")
endif


ifeq ($(RISCV_ARCH),)
$(error $(board_dir)/board.mk must set RISCV_ARCH, the RISC-V ISA string to target)
endif

ifeq ($(RISCV_ABI),)
$(error $(board_dir)/board.mk must set RISCV_ABI, the ABI to target)
endif


#############################################################
# Prints help message
#############################################################
.PHONY: help
help:
	@echo "  HummingBird RISC-V Embedded Processor Software Development Kit "
	@echo "  Makefile targets:"
	@echo ""
	@echo " tools [BOARD = $(BOARD)]:"
	@echo "    Install compilation & debugging tools to target your desired board."
	@echo ""
	@echo " uninstall:"
	@echo "    Uninstall the compilation & debugging tools."
	@echo ""
	@echo " software [PROGRAM=$(PROGRAM) BOARD=$(BOARD)]:"
	@echo "    Build a software program to load with the"
	@echo "    debugger."
	@echo ""
	@echo " upload [PROGRAM=$(PROGRAM) BOARD=$(BOARD)]:"
	@echo "    Launch OpenOCD to flash your program to the"
	@echo "    on-board Flash."
	@echo ""
	@echo " run_openocd [BOARD=$(BOARD)]:"
	@echo " run_gdb     [PROGRAM=$(PROGRAM) BOARD=$(BOARD)]:"
	@echo "     Launch OpenOCD or GDB seperately. Allows Ctrl-C to halt running"
	@echo "     programs."
	@echo ""
	@echo " dasm [PROGRAM=$(BOARD)]:"
	@echo "     Generates the dissassembly output of 'objdump -D' to stdout."
	@echo ""

.PHONY: clean
clean:

#############################################################
# This section is for tool installation
#############################################################
.PHONY: tools
tools: riscv-gnu-toolchain openocd

# Pointers to various important tools in the toolchain.
toolchain_builddir := $(builddir)/riscv-gnu-toolchain/riscv32-unknown-elf
toolchain_prefix := $(toolchain_builddir)/prefix

RISCV_PATH ?= $(toolchain_prefix)

RISCV_GCC     := $(abspath $(RISCV_PATH)/bin/riscv-none-embed-gcc)
RISCV_GXX     := $(abspath $(RISCV_PATH)/bin/riscv-none-embed-g++)
RISCV_OBJDUMP := $(abspath $(RISCV_PATH)/bin/riscv-none-embed-objdump)
RISCV_OBJCOPY := $(abspath $(RISCV_PATH)/bin/riscv-none-embed-objcopy)
RISCV_GDB     := $(abspath $(RISCV_PATH)/bin/riscv-none-embed-gdb)
RISCV_AR      := $(abspath $(RISCV_PATH)/bin/riscv-none-embed-ar)
RISCV_SIZE    := $(abspath $(RISCV_PATH)/bin/riscv-none-embed-size)

PATH := $(abspath $(RISCV_PATH)/bin):$(PATH)

$(RISCV_GCC) $(RISCV_GXX) $(RISCV_OBJDUMP) $(RISCV_GDB) $(RISCV_AR): $(toolchain_builddir)/install.stamp
	touch -c $@

# Builds riscv-gnu-toolchain, which contains GCC and all the supporting
# software for C code.
.PHONY: riscv-gnu-toolchain
riscv-gnu-toolchain: $(RISCV_GCC) $(RISCV_GXX) $(RISCV_OBJDUMP) $(RISCV_GDB) $(RISCV_AR)

$(builddir)/riscv-gnu-toolchain/%/install.stamp: $(builddir)/riscv-gnu-toolchain/%/build.stamp
	$(MAKE) -C $(dir $@) install
	date > $@

$(builddir)/riscv-gnu-toolchain/%/build.stamp: $(builddir)/riscv-gnu-toolchain/%/configure.stamp
	$(MAKE) -C $(dir $@)
	date > $@

$(builddir)/riscv-gnu-toolchain/%-elf/configure.stamp:
	$(eval $@_TUPLE := $(patsubst $(builddir)/riscv-gnu-toolchain/%-elf/configure.stamp,%,$@))
	rm -rf $(dir $@)
	mkdir -p $(dir $@)
	cd $(dir $@); $(abspath $(toolchain_srcdir)/configure) \
		--prefix=$(abspath $(dir $@)/prefix) \
		--disable-linux \
		--enable-multilib \
		--with-cmodel=medany \
		--with-libgcc-cmodel
	date > $@

.PHONY: 
clean: 

# Builds and installs OpenOCD, which translates GDB into JTAG for debugging and
# initializing the target.
openocd_builddir := $(builddir)/openocd
openocd_prefix := $(openocd_builddir)/prefix

RISCV_OPENOCD_PATH ?= $(openocd_prefix)
RISCV_OPENOCD ?= $(RISCV_OPENOCD_PATH)/bin/openocd

.PHONY: openocd
openocd: $(RISCV_OPENOCD)

$(RISCV_OPENOCD): $(openocd_builddir)/install.stamp
	touch -c $@

$(openocd_builddir)/install.stamp: $(openocd_builddir)/build.stamp
	$(MAKE) -C $(dir $@) install
	date > $@

$(openocd_builddir)/build.stamp: $(openocd_builddir)/configure.stamp
	$(MAKE) -C $(dir $@)
	date > $@

$(openocd_builddir)/configure.stamp:
	rm -rf $(dir $@)
	mkdir -p $(dir $@)
	cd $(abspath $(openocd_srcdir)); autoreconf -i
	cd $(dir $@); $(abspath $(openocd_srcdir)/configure) \
		--prefix=$(abspath $(dir $@)/prefix) \
		--disable-werror
	date > $@

.PHONY: 
clean: 

#############################################################
# This Section is for Software Compilation
#############################################################
PROGRAM_DIR = software/$(PROGRAM)
PROGRAM_ELF = software/$(PROGRAM)/$(PROGRAM)

.PHONY: software_clean
software_clean:
	$(MAKE) -C $(PROGRAM_DIR) BSP_BASE=$(abspath bsp) BOARD=$(BOARD)  DOWNLOAD=$(DOWNLOAD) clean

.PHONY: software
software: software_clean
	$(MAKE) -C $(PROGRAM_DIR) SIZE=$(RISCV_SIZE) CC=$(RISCV_GCC) RISCV_ARCH=$(RISCV_ARCH) REPLACE_PRINTF=$(REPLACE_PRINTF)  NANO_PFLOAT=$(NANO_PFLOAT) USE_NANO=$(USE_NANO) DOWNLOAD=$(DOWNLOAD)  RISCV_ABI=$(RISCV_ABI) AR=$(RISCV_AR) BSP_BASE=$(abspath bsp) BOARD=$(BOARD)

dasm: software 
	$(RISCV_OBJDUMP) -D $(PROGRAM_ELF) >& $(PROGRAM_ELF).dump
	$(RISCV_OBJCOPY) $(PROGRAM_ELF) -O verilog $(PROGRAM_ELF).verilog
	sed -i 's/@800/@000/g' $(PROGRAM_ELF).verilog



#############################################################
# This Section is for uploading a program to SPI Flash
#############################################################
OPENOCD_UPLOAD = bsp/${BOARD}/tools/openocd_upload.sh
ifeq ($(OCDCFG),${OLMX})
OPENOCDCFG ?= bsp/$(BOARD)/env/openocd_olmx.cfg
endif
ifeq ($(OCDCFG),${FTDI})
OPENOCDCFG ?= bsp/$(BOARD)/env/openocd_ftdi.cfg
endif
ifeq ($(OCDCFG),${HBIRD})
OPENOCDCFG ?= bsp/$(BOARD)/env/openocd_hbird.cfg
endif
OPENOCDARGS += -f $(OPENOCDCFG)

GDB_UPLOAD_ARGS ?= --batch

GDB_UPLOAD_CMDS += -ex "set remotetimeout 240"
GDB_UPLOAD_CMDS += -ex "target extended-remote localhost:$(GDB_PORT)"
GDB_UPLOAD_CMDS += -ex "monitor reset halt"
GDB_UPLOAD_CMDS += -ex "monitor flash protect 0 0 last off"
GDB_UPLOAD_CMDS += -ex "load"
GDB_UPLOAD_CMDS += -ex "monitor resume"
GDB_UPLOAD_CMDS += -ex "monitor shutdown"
GDB_UPLOAD_CMDS += -ex "quit"

openocd_upload:
	$(OPENOCD_UPLOAD) $(PROGRAM_ELF) $(OPENOCDCFG)

upload:
	$(RISCV_OPENOCD) $(OPENOCDARGS) & \
	$(RISCV_GDB) $(PROGRAM_DIR)/$(PROGRAM) $(GDB_UPLOAD_ARGS) $(GDB_UPLOAD_CMDS) && \
	echo "Successfully uploaded '$(PROGRAM)' to $(BOARD)."

#############################################################
# This Section is for launching the debugger
#############################################################

run_openocd:
	$(RISCV_OPENOCD) $(OPENOCDARGS)

GDBCMDS += -ex "set remotetimeout 240"
GDBCMDS += -ex "target extended-remote localhost:$(GDB_PORT)"

run_gdb:
	$(RISCV_GDB) $(PROGRAM_DIR)/$(PROGRAM) $(GDBARGS) $(GDBCMDS)
