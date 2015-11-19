

GCC_DIR = /opt/gnutools/arm-elf/bin
PKG_INSTALL_DIR = $(ROOT_DIR)/ecos/$(PROD_NAME)_ecos_install
PROD_DIR= $(ROOT_DIR)/prod
APPS_DIR = $(ROOT_DIR)/apps
#PROD_NAME = zot716u2w
PROD_BUILD_DIR = $(PROD_DIR)/$(PROD_NAME)/build
#HDR_MAK = $(PROD_BUILD_DIR)/hdr.mak
FTR_MAK = $(PROD_BUILD_DIR)/ftr.mak
TARGET_DEF = $(PROD_BUILD_DIR)/Target.def

VERSION='ZOTDWP2020_SDK_0.01'
VERSION_STRING=$(VERSION)
PS1=$'\\033[32m\n$VERSION_STRING \\033[31m$PROD_NAME \\033[33m\\w\\033[0m\n$ '

export VERSION_STRING

CC   = $(GCC_DIR)/arm-elf-gcc -mcpu=arm9 -w
LD   = $(GCC_DIR)/arm-elf-gcc
AR   = $(GCC_DIR)/arm-elf-ar
STRIP = $(GCC_DIR)/arm-elf-strip 

include $(TARGET_DEF)
C_DEFINED = -D$(HTML_FILE)

INC_DIR   = -I$(PKG_INSTALL_DIR)/include
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

ifeq ($(TARGET),N716U2W)
CFLAGS 	 +=	-DWPSBUTTON_LEDFLASH_FLICK \
			-DTXRX_SW_ANTDIV_SUPPORT \
			-DMTK7601 \
			-DUSE_WIRELSS_LIBS 
endif

CFLAGS   += -DVERSION=\"$(VERSION_STRING)\"

#add include 
CFLAGS   += $(C_DEFINED)

LDFLAGS   = -nostartfiles -nostdlib -L$(PKG_INSTALL_DIR)/lib -Wl,--gc-sections\
    -Ttarget.ld  -Xlinker -Map -Xlinker $(basename $@).map
ARFLAGS   = rv
#COPTFLAGS     = -O2
COPTFLAGS     := -g
DEPEND_FLAGS =  -Wp,-MD,$*.d
EXTRACFLAGS   = $(COPTFLAGS) $(DEPEND_FLAGS)
LIBS          = -Ttarget.ld -nostdlib
SRC_DIR         = .

#-nostdlib

.PHONY: depend all clean

%.o: %.c
	$(CC) -o $*.o $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $<

%.d: %.c
	$(CC) -E $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $< >/dev/null
