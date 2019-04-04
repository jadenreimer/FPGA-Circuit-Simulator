# Created by the Intel FPGA Monitor Program
# DO NOT MODIFY

############################################
# Global Defines
DEFINE_COMMA	:= ,

############################################
# Compilation Targets

# Programs
AS		:= arm-altera-eabi-as
CC		:= arm-altera-eabi-gcc
LD		:= arm-altera-eabi-ld
OC		:= arm-altera-eabi-objcopy
RM		:= rm -f

# Flags
USERCCFLAGS	:= -g -std=c99 -O1
ARCHASFLAGS	:= -mfloat-abi=soft -march=armv7-a -mcpu=cortex-a9 --gstabs -I "$$GNU_ARM_TOOL_ROOTDIR/arm-altera-eabi/include/"
ARCHCCFLAGS	:= -mfloat-abi=soft -march=armv7-a -mtune=cortex-a9 -mcpu=cortex-a9
ARCHLDFLAGS	:= --defsym arm_program_mem=0x40 --defsym arm_available_mem_size=0x3fffffb8 --defsym __cs3_stack=0x3ffffff8 --section-start .vectors=0x0
ARCHLDSCRIPT	:= -T"D:/intelFPGA_lite/18.1/University_Program/Monitor_Program/build/altera-socfpga-hosted-with-vectors.ld"
ASFLAGS		:= $(ARCHASFLAGS)
CCFLAGS		:= -Wall -c $(USERCCFLAGS) $(ARCHCCFLAGS)
LDFLAGS		:= $(patsubst %, -Wl$(DEFINE_COMMA)%, $(ARCHLDFLAGS)) $(ARCHLDSCRIPT)
OCFLAGS		:= -O srec

# Files
HDRS		:=
SRCS		:= ../src/devices.c ../src/devices.h ../src/examples.c ../src/main.c
OBJS		:= $(patsubst %, %.o, $(SRCS))

# Targets
compile: devices.srec

devices.srec: devices.axf
	$(RM) $@
	$(OC) $(OCFLAGS) $< $@

devices.axf: $(OBJS)
	$(RM) $@
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.c.o: %.c $(HDRS)
	$(RM) $@
	$(CC) $(CCFLAGS) $< -o $@

%.s.o: %.s $(HDRS)
	$(RM) $@
	$(AS) $(ASFLAGS) $< -o $@

clean:
	$(RM) devices.srec devices.axf $(OBJS)

