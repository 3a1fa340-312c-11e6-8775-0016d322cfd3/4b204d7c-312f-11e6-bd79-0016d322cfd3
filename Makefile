

export ROOT_DIR = $(PWD)
export HDR_MAK = $(ROOT_DIR)/rules.mak

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

DST_NAME = zot716u2w.axf

all: DIR_CHECK $(DST_NAME) 

DIR_CHECK: MAKE_LIB_DIR MAKE_OBJ_DIR

# Modules ###################################
MAC_MAK = $(APPS_DIR)/mac
mac:
	make -C $(MAC_MAK)

HTTP_ZOT_MAK = $(APPS_DIR)/http_zot/src
http_zot:
	make -C $(HTTP_ZOT_MAK)
	
IPPD_MAK = $(APPS_DIR)/ps/ippd/src
ippd:
	make -C $(IPPD_MAK)

SMBD_MAK = $(APPS_DIR)/ps/smbd/src
smbd:
	make -C $(SMBD_MAK)

LPD_MAK = $(APPS_DIR)/ps/lpd/src
lpd:
	make -C $(LPD_MAK)
		
RAWTCPD_MAK = $(APPS_DIR)/ps/rawtcpd
rawtcpd:
	make -C $(RAWTCPD_MAK)
	
USB_HOST_MAK = $(APPS_DIR)/usb_host/src
usb_host:
	make -C $(USB_HOST_MAK)	
	
IPXBEUI_MAK = $(APPS_DIR)/ipxbeui
ipxbeui:
	make -C $(IPXBEUI_MAK)		

PSGLOBAL_MAK = $(APPS_DIR)/ps/common/src
psglobal:
	make -C $(PSGLOBAL_MAK)	

PSUTILITY_MAK = $(APPS_DIR)/ps/psutility
psutility:
	make -C $(PSUTILITY_MAK)	
	
NTPS_MAK = $(APPS_DIR)/ps/ntps/src
ntps:
	make -C $(NTPS_MAK)	

SPOOLER_MAK = $(APPS_DIR)/spooler/src
spooler:
	make -C $(SPOOLER_MAK)
	
NOVELL_MAK = $(APPS_DIR)/ps/novell/src
novell:
	make -C $(NOVELL_MAK)

NDS_MAK = $(APPS_DIR)/ps/nds/src
nds:
	make -C $(NDS_MAK)	

ATALK_MAK = $(APPS_DIR)/ps/atalk/src
atalk:
	make -C $(ATALK_MAK)

RENDEZVOUS_MAK = $(APPS_DIR)/rendezvous/src
rendezvous:
	make -C $(RENDEZVOUS_MAK)
	
SNMP_MAK = $(APPS_DIR)/snmp/src
snmp:
	make -C $(SNMP_MAK)
	
TFTP_ZOT_MAK =	$(APPS_DIR)/ps/tftp_zot/src
tftp_zot:
	make -C $(TFTP_ZOT_MAK)
	
TCPIP_MAK = $(APPS_DIR)/tcpip/src
tcpip:
	make -C $(TCPIP_MAK)	

REALTEK_MAK = $(APPS_DIR)/realtek/src
realtek:
	make -C $(REALTEK_MAK)		

WPA_SUPPLICANT_MAK = $(APPS_DIR)/realtek/src/wpa_supplicant
wpa_supplicant:
	make -C $(WPA_SUPPLICANT_MAK)		

MTK7601_MAK = $(APPS_DIR)/mtk7601/src
mtk7601:
	make -C $(MTK7601_MAK)

TELNET_MAK = $(APPS_DIR)/ps/telnet/src
telnet:
	make -C $(TELNET_MAK)	

UART_MAK = $(APPS_DIR)/uart
uart:
	make -C $(UART_MAK)
	
ALL_LIBS =mac.a usb_host.a psglobal.a http_zot.a ipxbeui.a ntps.a spooler.a novell.a nds.a \
    lpd.a rawtcpd.a  ippd.a smbd.a telnet.a tftp_zot.a tcpip.a atalk.a snmp.a psutility.a uart.a\
    rendezvous.a mtk7601.a
#realtek.a wpa_supplicant.a 

PROD_LIBS = $(addprefix $(PROD_BUILD_DIR)/lib/, $(ALL_LIBS))

PROD_MODULES_MAK =$(MAC_MAK) $(USB_HOST_MAK) $(PSGLOBAL_MAK) $(NTPS_MAK) $(SPOOLER_MAK) $(NOVELL_MAK) $(NDS_MAK) $(IPPD_MAK) $(HTTP_ZOT_MAK) $(LPD_MAK) \
	$(RAWTCPD_MAK) $(TELNET_MAK) $(SMBD_MAK) $(TFTP_ZOT_MAK) $(ATALK_MAK) $(SNMP_MAK) $(RENDEZVOUS_MAK) $(TCPIP_MAK) $(PSUTILITY_MAK) $(IPXBEUI_MAK) \
	$(UART_MAK) $(MTK7601_MAK)

#$(MTK7601_MAK)
#$(REALTEK_MAK) $(WPA_SUPPLICANT_MAK)
PROD_MODULES = mac psutility usb_host ipxbeui ntps spooler psglobal novell nds ippd http_zot lpd rawtcpd\
				 telnet smbd tftp_zot tcpip atalk snmp rendezvous uart mtk7601

#realtek wpa_supplicant
prod: $(PROD_MODULES)

prodclean:
	@for M in $(PROD_MODULES_MAK); do\
		echo ....Clean $$M;\
		make -C $$M clean;\
		done
	rm -f $(basename $(DST_NAME)).map *.bin
	rm -f ./obj/*.o
		

proddep:
	@for M in $(PROD_MODULES_MAK); do\
		echo ....Make dependencies $$M;\
		make -C $$M clean;\
		done

clean: prodclean

cleanall: clean 
	
depend: proddep

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

OBJ_DIR       = $(PROD_BUILD_DIR)/obj
LIB_DIR       = $(PROD_BUILD_DIR)/lib

EXTRACFLAGS += -I$(PROD_BUILD_DIR) \
				-I$(APPS_DIR)/ps/common/incl -I$(APPS_DIR)/ps/ntps/incl \
				-I$(APPS_DIR)/ipxbeui -I$(APPS_DIR)/usb_host/src -I$(APPS_DIR)/star

$(OBJ_DIR)/%.o: $(PROD_BUILD_DIR)/%.c
	$(CC) -c -o $(OBJ_DIR)/$*.o $(CFLAGS) $(EXTRACFLAGS) $<

SRCS=$(PROD_BUILD_DIR)/zotmain.c

OBJS=${SRCS:$(PROD_BUILD_DIR)/%.c=$(OBJ_DIR)/%.o}

DST=./$(DST_NAME)

$(DST_NAME): ${OBJS} prod
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(PROD_LIBS) 
#	cp $(DST_NAME) zotdwp2020.bin
#	$(STRIP) zotdwp2020.bin
#	cp zotdwp2020.bin TEMP.bin
#	$(TOOLS_DIR)/GZIP -c TEMP.bin > zotdwp2020.gz
#	rm TEMP.bin
#	$(TOOLS_DIR)/maketarget 
#	$(TOOLS_DIR)/makimage $(PSMODELINDEX) $(CODE2MARK) $(MajorVer) $(MinorVer) $(ReleaseVer) $(RDVersion) $(BuildVer) $(MAKER_AND_CPU) zotdwp2020.gz MPS$(PSMODELINDEX).bin
#	cp MPS$(PSMODELINDEX).bin $(ROOT_DIR)/img/.

include $(FTR_MAK)
