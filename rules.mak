
VERSION='ZOT_SDK_0.01'
VERSION_STRING=$(VERSION)
PS1=$'\\033[32m\n$VERSION_STRING \\033[31m$PROD_NAME \\033[33m\\w\\033[0m\n$ '

export VERSION_STRING

ifeq ($(CHIP),arm9)
CC   = $(GCC_DIR)/arm-elf-gcc -mcpu=arm9 -w
LD   = $(GCC_DIR)/arm-elf-gcc
AR   = $(GCC_DIR)/arm-elf-ar
STRIP = $(GCC_DIR)/arm-elf-strip 
endif

ifeq ($(CHIP),mt7688)
export COMMAND_PREFIX = mipsisa32-elf-
export CC = $(COMMAND_PREFIX)gcc
export XCC= $(CC)
export LD = $(COMMAND_PREFIX)ld
export XLD= $(LD)
export AR = $(COMMAND_PREFIX)ar
export OBJCOPY = $(COMMAND_PREFIX)objcopy
export XOC = $(OBJCOPY)
export XOD = $(COMMAND_PREFIX)objdump
endif

include $(TARGET_DEF)
C_DEFINED = -D$(HTML_FILE)

INC_DIR   = -I$(PKG_INSTALL_DIR)/include -I. -I./include
CFLAGS    = -Wall $(INC_DIR) -ffunction-sections -fdata-sections
CFLAGS   += -D__ECOS -DECOS \
			-D$(TARGET) \
			-DUART_OUTPUT \
			-DUSE_SYS_LIBS \
			-DUSE_ADMIN_LIBS \
			-DUSE_PS_LIBS \
			-DUSE_NETAPP_LIBS

#-DWIRELESS_CARD for wireless function
#-DMTK7601 to use mt7601 wifi module
#-DMONITOR -DED_SMART for wifi adapative
#-DWIFI_CERTIFICATION for wifi certification use (power limited)
#			-DWIFI_CERTIFICATION \
#			-DED_MONITOR \
#			-DED_SMART \
#			code package splitor
#			-DUSE_SYS_LIBS
#			-DUSE_ADMIN_LIBS
#			-DUSE_WIRELESS_LIBS
#			-DUSE_PS_LIBS
#			-DUSE_NETAPP_LIBS

ifeq ($(TARGET),$(filter $(TARGET), N716U2W NDWP2020))
CFLAGS 	 +=	-DWPSBUTTON_LEDFLASH_FLICK \
			-DTXRX_SW_ANTDIV_SUPPORT \
			-DMTK7601 \
			-DUSE_WIRELESS_LIBS 
endif

CFLAGS   += -DVERSION=\"$(VERSION_STRING)\"

#add include 
CFLAGS   += $(C_DEFINED)

LDFLAGS   = -nostartfiles -nostdlib -L$(PKG_INSTALL_DIR)/lib \
    -Ttarget.ld  -Xlinker -Map -Xlinker $(basename $@).map
ARFLAGS   	 = rv
#COPTFLAGS   = -O2
COPTFLAGS    := -g
DEPEND_FLAGS =  -Wp,-MD,$*.d
EXTRACFLAGS  = $(COPTFLAGS) $(DEPEND_FLAGS)
LIBS         = -Ttarget.ld -nostdlib
SRC_DIR      = .

ifeq ($(CHIP),arm9)
LDFLAGS += -Wl,--gc-sections
endif

ifeq ($(CHIP),mt7688)
CFLAGS +=  -I$(TOPDIR)/include -I$(TOPDIR)/apps/tcpip/incl/ 
CFLAGS += -EL -mips32 -msoft-float -gstabs -fno-rtti -fno-exceptions -G0 -DCONFIG_MT7628_ASIC
LDFLAGS += -EL -mips32 -msoft-float -Wl,--gc-sections -Wl,-static
endif

.PHONY: depend all clean

%.o: %.c
	$(CC) -o $*.o $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $<

%.d: %.c
	$(CC) -E $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $< >/dev/null

%.o: %.S
	$(CC) -c -o $*.o $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $<

%.d: %.S
	$(CC) -c -E $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $< >/dev/null

%.bin: %.axf
	$(XOC) -O binary $(@:.bin=.axf) $@
