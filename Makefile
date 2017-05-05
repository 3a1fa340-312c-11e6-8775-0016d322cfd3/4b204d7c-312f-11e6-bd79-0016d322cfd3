
# arm9, mt7688
ifeq ($(CHIP),)
CHIP = mt7688
export CHIP
endif

ifeq ($(CHIP),mt7688)
CHIPSET = mt7628 
PLATFORM = MT7628
RT28xx_MODE = STA
export CHIPSET RT28xx_MODE PLATFORM
endif

# zot716u2w, zot716u2, zotdwp2020
PROD_NAME ?= zot716u2
# TPLINK, ZOTEH, ZOT, ZOTCHS, LINEUP, IOG(IOGEAR), ASSMAN, DIGISOL, LS(LONGSHINE), LEVELO(LEVELONE)
OEM ?= TPLINK

ifeq ($(OEM),ZOTCHS)
P_MARK = X
else
P_MARK = G
endif

ROOT_DIR = $(shell pwd)
TOPDIR = $(ROOT_DIR)
HDR_MAK = $(ROOT_DIR)/rules.mak
#ENDIAN=$(shell echo $(ECOS_GLOBAL_CFLAGS) | sed -e "s/.*-EL.*/-EL/" )
ENDIAN = -EL
HOSTOS = $(shell uname | tr A-Z a-z)

PROD_DIR= $(ROOT_DIR)/prod
APPS_DIR = $(ROOT_DIR)/apps
PROD_BUILD_DIR = $(PROD_DIR)/$(PROD_NAME)/build
FTR_MAK = $(PROD_BUILD_DIR)/ftr.mak
TARGET_DEF = $(PROD_BUILD_DIR)/Target.def

TARGET_OS = ECOS
ECOS_REPOSITORY := $(ROOT_DIR)/ecos/packages
ECOS_TOOL_PATH := $(ROOT_DIR)/tools/$(HOSTOS)/bin
ifeq ($(HOSTOS),darwin)
ECOS_MIPSTOOL_PATH := $(ROOT_DIR)/tools/mipsel-linux-uclibc/bin
TFTP_DIR = /private/tftpboot
export STAGING_DIR =
else
ECOS_MIPSTOOL_PATH := $(ROOT_DIR)/tools/mipsisa32-elf/bin
TFTP_DIR = /tftpboot
endif
DRIVERS =
EXTOBJS =

ifeq ($(CHIP),arm9)
PKG_INSTALL_DIR = $(ROOT_DIR)/ecos/$(PROD_NAME)_ecos_install
endif

ifeq ($(CHIP),mt7688)
KERNEL_BUILD_DIR = $(ROOT_DIR)/ecos/$(CHIP)_ecos_install
PKG_INSTALL_DIR = $(KERNEL_BUILD_DIR)/install
ifeq (.config, $(wildcard .config))
include .config
DRVSUBDIRS =
CONFIG_ZLOAD_BUF = 0x80500000
endif

ifdef CONFIG_RA305X
	DRVSUBDIRS += ./drivers/ra305x_drivers/eth_ra305x
endif
ifdef CONFIG_WIRELESS
ifeq ($(CHIPSET),mt7628)
	DRVSUBDIRS += ./drivers/ra305x_drivers/mt_wifi/embedded
endif	
#ifdef CONFIG_ATE_DAEMON
#	DRVSUBDIRS += ./drivers/ra305x_drivers/wireless_ate
#	CFLAGS += -DRALINK_ATE_SUPPORT
#endif # CONFIG_ATE_DAEMON
#ifdef CONFIG_80211X_DAEMON
#	DRVSUBDIRS += ./drivers/ra305x_drivers/wireless_rtdot1x
#	CFLAGS += -DRALINK_1X_SUPPORT
#endif # CONFIG_80211X_DAEMON
#ifdef CONFIG_LLTD_DAEMON
#	DRVSUBDIRS += ./drivers/ra305x_drivers/lltd
#	CFLAGS += -DRALINK_LLTD_SUPPORT
#endif # CONFIG_LLTD_DAEMON
endif # CONFIG_WIRELESS

DRVSUBDIRS += ./drivers/ra305x_drivers/flash
DRVSUBDIRS += ./drivers/ra305x_drivers/gpio
DRVSUBDIRS += ./drivers/ra305x_drivers/usb
DRIVERS += drivers.o
#EXTOBJS += $(OBJ_DIR)/drivers.o
EXTOBJS += $(patsubst %, $(OBJ_DIR)/%, $(DRIVERS))
endif

ifeq ($(CHIP),arm9)
GCC_DIR = /opt/gnutools/arm-elf/bin
ARCH = ARCH_ARM
endif

ifeq ($(CHIP),mt7688)
ifeq ($(HOSTOS),darwin)
GCC_DIR = $(ROOT_DIR)/tools/mipsel-linux-uclibc/bin
else
GCC_DIR = $(ROOT_DIR)/tools/mipsisa32-elf/bin
endif
ARCH = ARCH_MIPS
endif

include rules.mak
include $(TARGET_DEF)

RDVersionPlus := $(shell echo ${RDVersion} + 1 | bc)

#Ron Add 4/15/2004 ==================================================================
#|0xB0: ALL FLASH | 0xB1: Code1 | 0xB2: Code2 | 0xB3: EEPROM |0xB4: LOADER
ALLFLASHMARK    = 0xB0
CODE1MARK       = 0xB1
CODE2MARK       = 0xB2
EEPROMMARK      = 0xB3
LOADERMARK      = 0xB4
WIFIEPMARK		= 0xB5

TOOLS_DIR = ./prod/$(PROD_NAME)/build/tools

DST_NAME = $(PROD_NAME).axf

ifeq ($(CHIP),arm9)
PATH := $(GCC_DIR):$(PATH)
endif

ifeq ($(CHIP),mt7688)
PATH := $(ECOS_TOOL_PATH):$(ECOS_MIPSTOOL_PATH):$(PATH)
endif

export PATH ROOT_DIR TOPDIR PROD_DIR APPS_DIR HDR_MAK FTR_MAK GCC_DIR TARGET_DEF ARCH
export PKG_INSTALL_DIR ECOS_REPOSITORY ECOS_TOOL_PATH ECOS_MIPSTOOL_PATH TARGET_OS HOSTOS
export PROD_NAME OEM

ifeq ($(CHIP),arm9)
all: DIR_CHECK RM_AXF_FILE $(DST_NAME) 
else
all: mt7688_ecos.img
endif

DIR_CHECK: MAKE_KERNEL_DIR MAKE_LIB_DIR MAKE_OBJ_DIR

# use this library must to define USE_SYS_LIBS
# SYS_LIBS		= mac.a usb_host.a common.a http_zot.a tcpip.a psutility.a uart.a
# use this library must to define USE_ADMIN_LIBS
# ADMIN_LIBS 	= ntps.a ipxbeui.a
# use this library must to define USE_PS_LIBS 
# PS_LIBS		= spooler.a novell.a nds.a lpd.a ippd.a atalk.a rawtcpd.a rendezvous.a snmp.a tftp_zot.a
# use htis library must to define USE_NETAPP_LIBS
# NETAPP_LIBS 	= telnet.a smbd.a
# ALL_LIBS = $(SYS_LIBS) $(ADMIN_LIBS) $(PS_LIBS) $(NETAPP_LIBS)

PROD_MODULES 	=

ifeq ($(CHIP),arm9)
SYS_LIBS		= mac usb_host ps/common http_zot tcpip ps/psutility uart
ADMIN_LIBS 		= ps/ntps ipxbeui
PS_LIBS			= spooler ps/novell ps/nds ps/lpd ps/ippd ps/atalk ps/rawtcpd rendezvous snmp ps/tftp_zot
NETAPP_LIBS 	= ps/telnet ps/smbd

#
# wireless driver have realtek, rt3070, mtk7601
#

WIRELESS_LIBS = mtk7601

ifdef $(WIRELESS_LIBS)
PROD_MODULES += $(WIRELESS_LIBS)
endif

else #mt7688

SYS_LIBS		= tcpip ps/common http_zot ps/psutility
ADMIN_LIBS 		= ps/ntps ipxbeui 
PS_LIBS			= spooler ps/novell ps/nds ps/lpd ps/ippd ps/atalk ps/rawtcpd rendezvous snmp ps/tftp_zot
NETAPP_LIBS 	= ps/telnet ps/smbd

endif

ifneq ($(findstring USE_SYS_LIBS,$(CFLAGS)),)
PROD_MODULES += $(SYS_LIBS)
endif
ifneq ($(findstring USE_ADMIN_LIBS,$(CFLAGS)),)
PROD_MODULES += $(ADMIN_LIBS)
endif
ifneq ($(findstring USE_PS_LIBS,$(CFLAGS)),)
PROD_MODULES += $(PS_LIBS)
endif
ifneq ($(findstring USE_NETAPP_LIBS,$(CFLAGS)),)
PROD_MODULES += $(NETAPP_LIBS)
endif

PROD_LIBS = $(addprefix $(PROD_BUILD_DIR)/lib/, $(addsuffix .a, $(notdir $(PROD_MODULES))))

DRV_OBJS = $(join $(DRVSUBDIRS), $(foreach n,$(DRVSUBDIRS), $(shell echo "/"$(shell echo /$(n).o | sed "s/.*\///"))))

drvsubdirs: $(patsubst %, _dir_%, $(DRVSUBDIRS))

$(patsubst %, _dir_%, $(DRVSUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@) ;

drivers.o: drvsubdirs $(DRV_OBJS)
	$(LD) -r $(ENDIAN) -o $(OBJ_DIR)/drivers.o $(DRV_OBJS) ;

prod: $(patsubst %, _folder_%, $(PROD_MODULES))

$(patsubst %, _folder_%, $(PROD_MODULES)) :
	make -C $(patsubst _folder_%, $(APPS_DIR)/%/src, $@) ;

prodclean: $(patsubst %, _clean_%, $(PROD_MODULES))

$(patsubst %, _clean_%, $(PROD_MODULES)) :
	make -C $(patsubst _clean_%, $(APPS_DIR)/%/src, $@) clean ;

driver_clean: $(patsubst %, _drvclean_%, $(DRVSUBDIRS))

$(patsubst %, _drvclean_%, $(DRVSUBDIRS)):
	$(warning make -C $(patsubst _drvclean_%, %, $@) clean)
	make -C $(patsubst _drvclean_%, %, $@) clean ;


N716U2:
	make package PROD_NAME=zot716u2

N716U2W:
	make package PROD_NAME=zot716u2w

NDWP2020:
	make package PROD_NAME=zotdwp2020

package:
	make PROD_NAME=$(PROD_NAME) clean
	
ifeq ($(CHIP),arm9)
	make COPTFLAGS=-O2 
	cp $(DST_NAME) $(PROD_NAME).bin
	$(STRIP) $(PROD_NAME).bin
	cp $(PROD_NAME).bin TEMP.bin
	gzip -c TEMP.bin > $(PROD_NAME).gz
	rm TEMP.bin
endif
ifeq ($(CHIP),mt7688)
	make mt7688_ecos.img
	mv mt7688_ecos.img $(PROD_NAME).gz
endif
	./mkimage.py $(PSMODELINDEX) $(CODE2MARK) $(MajorVer) $(MinorVer) $(ReleaseVer) $(RDVersionPlus) $(BuildVer) $(MAKER_AND_CPU) $(P_MARK) $(PROD_NAME).gz MPS$(PSMODELINDEX).bin
	cp MPS$(PSMODELINDEX).bin $(ROOT_DIR)/img/MPS$(PSMODELINDEX)_$(OEM).bin

target.ld:
	cp $(ROOT_DIR)/ecos/$(HOSTOS)-target.ld $(PKG_INSTALL_DIR)/lib/$@ ;

lzmaImage: DIR_CHECK RM_AXF_FILE target.ld $(DST_NAME) $(PROD_NAME).bin $(PROD_NAME).map
	$(MAKE) CFAGS="$(CFLAGS)" ENDIAN=$(ENDIAN) -C zload ;
	$(ECOS_TOOL_PATH)/lzma e $(PROD_NAME).bin bin.gz ;	
	$(XLD) $(ENDIAN) $(LD_EXTRA) -Ttext=$(CONFIG_ZLOAD_BUF) -Tzload/$(HOSTOS)-zload.ld -o $@ zload/zload.o -\( -b binary bin.gz -\) -Map zload.map ;
	$(XNM) $@ | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | sort > $@.map ;

mt7688_ecos.img: lzmaImage.bin 
	mkimage -A mips -T standalone -C none -a $(CONFIG_ZLOAD_BUF) -e $(CONFIG_ZLOAD_BUF) -n $(PROD_NAME) -d $< $@ ;
	sudo cp $@ $(TFTP_DIR) ;

loader: uboot.bin
	cp $(TARGET_DEF) .
	# wine $(TOOLS_DIR)/maketarget
	# wine $(TOOLS_DIR)/makimage $(PSMODELINDEX) $(LOADERMARK) $(MajorVer) $(MinorVer) $(ReleaseVer) $(RDVersionPlus) $(BuildVer) $(MAKER_AND_CPU) $< MPS$(PSMODELINDEX)_loader.bin
	./makimage.py $(PSMODELINDEX) $(LOADERMARK) $(MajorVer) $(MinorVer) $(ReleaseVer) $(RDVersionPlus) $(BuildVer) $(MAKER_AND_CPU) $(P_MARK) $< MPS$(PSMODELINDEX)_loader.bin
	cp MPS$(PSMODELINDEX)_loader.bin $(ROOT_DIR)/img/.
	# mv Target.def $(TARGET_DEF)

eeprom: 7688A_v2_1.bin
	cp $(TARGET_DEF) .
	# wine $(TOOLS_DIR)/maketarget
	# wine $(TOOLS_DIR)/makimage $(PSMODELINDEX) $(WIFIEPMARK) $(MajorVer) $(MinorVer) $(ReleaseVer) $(RDVersionPlus) $(BuildVer) $(MAKER_AND_CPU) $< MPS$(PSMODELINDEX)_eeprom.bin
	./mkimage.py $(PSMODELINDEX) $(WIFIEPMARK) $(MajorVer) $(MinorVer) $(ReleaseVer) $(RDVersionPlus) $(BuildVer) $(MAKER_AND_CPU) $(P_MARK) $< MPS$(PSMODELINDEX)_eeprom.bin
	cp MPS$(PSMODELINDEX)_eeprom.bin $(ROOT_DIR)/img/.
	# mv Target.def $(TARGET_DEF)

OBJ_DIR       = $(PROD_BUILD_DIR)/obj
LIB_DIR       = $(PROD_BUILD_DIR)/lib

export PROD_BUILD_DIR OBJ_DIR LIB_DIR


ifeq ($(CHIP),arm9)
EXTRACFLAGS +=  -I. \
				-I$(PROD_BUILD_DIR) \
				-I$(APPS_DIR)/ps/common/incl -I$(APPS_DIR)/ps/ntps/incl \
				-I$(APPS_DIR)/ipxbeui/incl -I$(APPS_DIR)/usb_host/src -I$(APPS_DIR)/star/incl
else
EXTRACFLAGS +=  -I. \
				-I$(PROD_BUILD_DIR) \
				-I$(APPS_DIR)/ps/common/incl -I$(APPS_DIR)/ps/ntps/incl \
				-I$(APPS_DIR)/ipxbeui/incl -I$(APPS_DIR)/star/incl
endif


$(OBJ_DIR)/%.o: $(PROD_BUILD_DIR)/%.c
	$(CC) -c -o $(OBJ_DIR)/$*.o $(CFLAGS) $(EXTRACFLAGS) $< ;

SRCS=$(PROD_BUILD_DIR)/zotmain.c
OBJS=${SRCS:$(PROD_BUILD_DIR)/%.c=$(OBJ_DIR)/%.o}

#OBJS += $(PROD_BUILD_DIR)/start.o $(PROD_BUILD_DIR)/tcpip.o $(PROD_BUILD_DIR)/syslib.o
ifeq ($(CHIP),mt7688)
OBJS += $(PROD_BUILD_DIR)/start.o 
endif

DST=./$(DST_NAME)

#$(DST_NAME): ${OBJS} prod $(DRIVERS)
#	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(OBJ_DIR)/drivers.o $(PROD_LIBS) 

$(DST_NAME): build_web ${OBJS} prod $(DRIVERS)
	$(CC) $(LDFLAGS) -Wl,-static -Wl,--cref -o $@ $(OBJS) $(EXTOBJS) $(PROD_LIBS) ;

build_web:
	$(MAKE) -C ./www

#$(DST_NAME): ${OBJS} prod $(DRIVERS) $(DRV_OBJS)
#	$(CC) $(LDFLAGS) -Wl,-static -Wl,--cref -o $@ $(OBJS) $(DRV_OBJS) $(PROD_LIBS) 

#$(DST_NAME): ${OBJS} $(DRIVERS)
#	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(OBJ_DIR)/drivers.o

#
# build ecos kernel for mt7688
#
MAKE_KERNEL_DIR:
ifeq ($(CHIP),mt7688)
	if [ ! -d $(KERNEL_BUILD_DIR) ]; \
		then mkdir $(KERNEL_BUILD_DIR); \
		cd $(KERNEL_BUILD_DIR); \
		cp ../\pkgconf/\$(HOSTOS)-mt7688_bsp.ecc mt7688_bsp.ecc; \
		ecosconfig --config=mt7688_bsp.ecc tree; \
		make;\
		rm $(PKG_INSTALL_DIR)/include/network.h;\
	else \
		cd $(KERNEL_BUILD_DIR); \
		make; \
		rm $(PKG_INSTALL_DIR)/include/network.h;\
	fi
-include $(PKG_INSTALL_DIR)/include/pkgconf/ecos.mak
endif

#include $(FTR_MAK)

MAKE_OBJ_DIR:
	@-if !(test -d $(OBJ_DIR)); then mkdir $(OBJ_DIR); fi;

# Rule to create $(LIB_DIR) if $(LIB_DIR) does not exist
MAKE_LIB_DIR:
	@-if !(test -d $(LIB_DIR)); then mkdir $(LIB_DIR); fi;

RM_AXF_FILE:
	@-if (test -f $(PROD_NAME).axf); then rm $(PROD_NAME).axf; fi;

clean: prodclean driver_clean
	make -C ./zload clean 
	-rm lzmaImage lzmaImage.bin
	-rm bin.gz
	-rm $(OBJ_DIR)/drivers.o
	-rm *.map *.img
	-rm -f $(OBJS)
	-rm -f ${DST}
	-rm -f $(DEPEND_FILES)
	-rm -f .depend

cleanall: clean 
	
depend: prodclean

