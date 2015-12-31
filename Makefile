
# arm9, mt7688
ifeq ($(CHIP),)
	CHIP = mt7688
	CHIPSET = mt7628
	export CHIP CHIPSET
endif

# zot716u2w, zot716u2, dwp2020
PROD_NAME ?= zot716u2w

ROOT_DIR = $(PWD)
TOPDIR = $(ROOT_DIR)
HDR_MAK = $(ROOT_DIR)/rules.mak

PROD_DIR= $(ROOT_DIR)/prod
APPS_DIR = $(ROOT_DIR)/apps
PROD_BUILD_DIR = $(PROD_DIR)/$(PROD_NAME)/build
FTR_MAK = $(PROD_BUILD_DIR)/ftr.mak
TARGET_DEF = $(PROD_BUILD_DIR)/Target.def

TARGET_OS = ECOS
ECOS_REPOSITORY := $(ROOT_DIR)/ecos/packages
ECOS_TOOL_PATH := $(ROOT_DIR)/tools/bin
ECOS_MIPSTOOL_PATH := $(ROOT_DIR)/tools/mipsisa32-elf/bin

ifeq ($(CHIP),arm9)
PKG_INSTALL_DIR = $(ROOT_DIR)/ecos/$(PROD_NAME)_ecos_install
endif

ifeq ($(CHIP),mt7688)
KERNEL_BUILD_DIR = $(ROOT_DIR)/ecos/$(CHIP)_ecos_install
PKG_INSTALL_DIR = $(KERNEL_BUILD_DIR)/install
ifeq (.config, $(wildcard .config))
include .config
DRVSUBDIRS =
endif

ifdef CONFIG_RA305X
	DRVSUBDIRS += ./drivers/ra305x_drivers/eth_ra305x
endif
ifdef CONFIG_WIRELESS
ifeq ($(CHIPSET),mt7628)
	DRVSUBDIRS += ./drivers/ra305x_drivers/Jedi_7628/embedded
endif	
ifdef CONFIG_ATE_DAEMON
	DRVSUBDIRS += ./drivers/ra305x_drivers/wireless_ate
	CFLAGS += -DRALINK_ATE_SUPPORT
endif # CONFIG_ATE_DAEMON
ifdef CONFIG_80211X_DAEMON
	DRVSUBDIRS += ./drivers/ra305x_drivers/wireless_rtdot1x
	CFLAGS += -DRALINK_1X_SUPPORT
endif # CONFIG_80211X_DAEMON
ifdef CONFIG_LLTD_DAEMON
	DRVSUBDIRS += ./drivers/ra305x_drivers/lltd
	CFLAGS += -DRALINK_LLTD_SUPPORT
endif # CONFIG_LLTD_DAEMON
endif # CONFIG_WIRELESS
endif

ifeq ($(CHIP),arm9)
GCC_DIR = /opt/gnutools/arm-elf/bin
endif

ifeq ($(CHIP),mt7688)
GCC_DIR = $(ROOT_DIR)/tools/mipsisa32-elf/bin
endif

include rules.mak
include $(TARGET_DEF)

#Ron Add 4/15/2004 ==================================================================
#|0xB0: ALL FLASH | 0xB1: Code1 | 0xB2: Code2 | 0xB3: EEPROM |0xB4: LOADER
ALLFLASHMARK    = 0xB0
CODE1MARK       = 0xB1
CODE2MARK       = 0xB2
EEPROMMARK      = 0xB3
LOADERMARK      = 0xB4

TOOLS_DIR = ./prod/$(PROD_NAME)/build/tools

DST_NAME = $(PROD_NAME).axf

ifeq ($(CHIP),arm9)
PATH := $(GCC_DIR):$(PATH)
endif

ifeq ($(CHIP),mt7688)
PATH := $(ECOS_TOOL_PATH):$(ECOS_MIPSTOOL_PATH):$(PATH)
endif

export PATH ROOT_DIR TOPDIR PROD_DIR APPS_DIR HDR_MAK FTR_MAK GCC_DIR TARGET_DEF 
export PKG_INSTALL_DIR ECOS_REPOSITORY ECOS_TOOL_PATH ECOS_MIPSTOOL_PATH TARGET_OS

ifeq ($(CHIP),arm9)
all: DIR_CHECK RM_AXF_FILE $(DST_NAME) 
else
all: mt7688_ecos.img
endif

$(warning DRVSUBDIRS is $(DRVSUBDIRS))

DIR_CHECK: MAKE_KERNEL_DIR MAKE_LIB_DIR MAKE_OBJ_DIR


# use this library must to define USE_SYS_LIBS
SYS_LIBS	= mac.a usb_host.a common.a http_zot.a tcpip.a psutility.a uart.a

ADMIN_LIBS 	= ntps.a ipxbeui.a

WIRELESS_LIBS = mtk7601.a

# use this library must to define USE_PS_LIBS 
PS_LIBS		= spooler.a novell.a nds.a lpd.a ippd.a atalk.a rawtcpd.a rendezvous.a snmp.a tftp_zot.a

# use htis library must to define USE_NETAPP_LIBS
NETAPP_LIBS = telnet.a smbd.a

ALL_LIBS = $(SYS_LIBS) $(ADMIN_LIBS) $(PS_LIBS) $(NETAPP_LIBS)

ifeq ($(TARGET),$(filter $(TARGET), N716U2W NDWP2020))
ALL_LIBS += $(WIRELESS_LIBS)
endif


ifeq ($(CHIP),arm9)
PROD_LIBS = $(addprefix $(PROD_BUILD_DIR)/lib/, $(ALL_LIBS))

PROD_MODULES = mac ps/psutility usb_host ipxbeui ps/ntps spooler ps/common ps/novell ps/nds ps/ippd http_zot ps/lpd ps/rawtcpd\
				 ps/telnet ps/smbd ps/tftp_zot tcpip ps/atalk snmp rendezvous uart mtk7601
else
PROD_LIBS =
PROD_MODULES =
endif

DRV_OBJS = $(join $(DRVSUBDIRS), $(foreach n,$(DRVSUBDIRS), $(shell echo "/"$(shell echo /$(n).o | sed "s/.*\///"))))

drvsubdirs: $(patsubst %, _dir_%, $(DRVSUBDIRS))

$(patsubst %, _dir_%, $(DRVSUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

drivers.o: drvsubdirs $(DRV_OBJS)
	$(LD) -r $(ENDIAN) -o drivers.o $(DRV_OBJS)

prod: $(patsubst %, _folder_%, $(PROD_MODULES))

$(patsubst %, _folder_%, $(PROD_MODULES)) :
	make -C $(patsubst _folder_%, $(APPS_DIR)/%/src, $@)

prodclean: $(patsubst %, _clean_%, $(PROD_MODULES))

$(patsubst %, _clean_%, $(PROD_MODULES)) :
	make -C $(patsubst _clean_%, $(APPS_DIR)/%/src, $@) clean

driver_clean: $(patsubst %, _drvclean_%, $(DRVSUBDIRS))

$(patsubst %, _drvclean_%, $(DRVSUBDIRS)):
	$(warning make -C $(patsubst _drvclean_%, %, $@) clean)
	make -C $(patsubst _drvclean_%, %, $@) clean


clean: prodclean driver_clean

cleanall: clean 
	
depend: prodclean

N716U2:
	make package PROD_NAME=zot716u2

N716U2W:
	make package PROD_NAME=zot716u2w

DWP2020:
	make package PROD_NAME=zotdwp2020
package:
	make clean
	make COPTFLAGS=-O2 
	cp $(DST_NAME) $(PROD_NAME).bin
	$(STRIP) $(PROD_NAME).bin
	cp $(PROD_NAME).bin TEMP.bin
	gzip -c TEMP.bin > $(PROD_NAME).gz
	rm TEMP.bin
	cp $(TARGET_DEF) .
	wine $(TOOLS_DIR)/maketarget 
	wine $(TOOLS_DIR)/makimage $(PSMODELINDEX) $(CODE2MARK) $(MajorVer) $(MinorVer) $(ReleaseVer) $(RDVersion) $(BuildVer) $(MAKER_AND_CPU) $(PROD_NAME).gz MPS$(PSMODELINDEX).bin
	cp MPS$(PSMODELINDEX).bin $(ROOT_DIR)/img/.
	rm Target.def

mt7688_ecos.img: DIR_CHECK RM_AXF_FILE $(DST_NAME) $(PROD_NAME).bin 
	mkimage -A mips -T standalone -C none -a 0x80000400 -e 0x80000400 -n zot716u2w -d zot716u2w.bin $@ 
	sudo cp $@ /tftpboot

OBJ_DIR       = $(PROD_BUILD_DIR)/obj
LIB_DIR       = $(PROD_BUILD_DIR)/lib

export PROD_BUILD_DIR OBJ_DIR LIB_DIR

EXTRACFLAGS +=  -I. \
				-I$(PROD_BUILD_DIR) \
				-I$(APPS_DIR)/ps/common/incl -I$(APPS_DIR)/ps/ntps/incl \
				-I$(APPS_DIR)/ipxbeui/incl -I$(APPS_DIR)/usb_host/src -I$(APPS_DIR)/star/incl


$(OBJ_DIR)/%.o: $(PROD_BUILD_DIR)/%.c
	$(CC) -c -o $(OBJ_DIR)/$*.o $(CFLAGS) $(EXTRACFLAGS) $<

SRCS=$(PROD_BUILD_DIR)/zotmain.c
OBJS=${SRCS:$(PROD_BUILD_DIR)/%.c=$(OBJ_DIR)/%.o}

#OBJS += $(PROD_BUILD_DIR)/start.o $(PROD_BUILD_DIR)/tcpip.o $(PROD_BUILD_DIR)/syslib.o
ifeq ($(CHIP),mt7688)
OBJS += $(PROD_BUILD_DIR)/start.o 
endif

DST=./$(DST_NAME)

$(DST_NAME): ${OBJS} prod drivers.o
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(PROD_LIBS) 

#
# build ecos kernel for mt7688
#
MAKE_KERNEL_DIR:
ifeq ($(CHIP),mt7688)
	if [ ! -d $(KERNEL_BUILD_DIR) ]; \
		then mkdir $(KERNEL_BUILD_DIR); \
		cd $(KERNEL_BUILD_DIR); \
		cp ../\pkgconf/\mt7688_bsp.ecc .; \
		ecosconfig --config=mt7688_bsp.ecc tree; \
		make;\
	   	fi
endif
include $(FTR_MAK)
