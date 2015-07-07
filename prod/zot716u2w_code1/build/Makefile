#                Copyright 2006, ZOT.

include $(HDR_MAK)
include $(TARGET_DEF)

#Ron Add 4/15/2004 ==================================================================
#|0xB0: ALL FLASH | 0xB1: Code1 | 0xB2: Code2 | 0xB3: EEPROM |0xB4: LOADER
ALLFLASHMARK    = 0xB0
CODE1MARK       = 0xB1
CODE2MARK       = 0xB2
EEPROMMARK      = 0xB3
LOADERMARK      = 0xB4

IMAGE_COUNT     = 2

TOOLS_DIR = ./tools

DST_NAME = zot716u2w_code1.axf

all: DIR_CHECK $(DST_NAME) 

DIR_CHECK: MAKE_LIB_DIR MAKE_OBJ_DIR

# Modules ###################################
MAC_MAK = $(APPS_DIR)/mac
mac:
	make -C $(MAC_MAK)
	
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
	
TFTP_ZOT_MAK =	$(APPS_DIR)/ps/tftp_zot/src
tftp_zot:
	make -C $(TFTP_ZOT_MAK)
	
TCPIP_MAK = $(APPS_DIR)/tcpip/src
tcpip:
	make -C $(TCPIP_MAK)	

PROD_LIBS =./lib/mac_code1.a ./lib/psglobal_code1.a ./lib/ipxbeui_code1.a ./lib/ntps_code1.a  \
    ./lib/tftp_zot_code1.a ./lib/tcpip_code1.a ./lib/psutility_code1.a 
    

PROD_MODULES_MAK =$(MAC_MAK) $(PSGLOBAL_MAK) $(NTPS_MAK) \
	$(TFTP_ZOT_MAK) $(TCPIP_MAK) $(PSUTILITY_MAK) $(IPXBEUI_MAK)
    
PROD_MODULES = mac psutility ipxbeui ntps psglobal  \
				  tftp_zot tcpip   

prod: $(PROD_MODULES)

prodclean:
	@for M in $(PROD_MODULES_MAK); do\
		echo ....Clean $$M;\
		make -C $$M clean;\
		done
	rm -f $(basename $(DST_NAME)).map *.bin $(PROD_NAME).gz
	rm -f ./obj/*.o
		

proddep:
	@for M in $(PROD_MODULES_MAK); do\
		echo ....Make dependencies $$M;\
		make -C $$M clean;\
		done

clean: prodclean

cleanall: clean 
	
depend: proddep

OBJ_DIR       = $(PROD_BUILD_DIR)/obj
LIB_DIR       = $(PROD_BUILD_DIR)/lib

EXTRACFLAGS += -I$(PROD_BUILD_DIR) \
				-I$(APPS_DIR)/ps/common/incl -I$(APPS_DIR)/ps/ntps/incl \
				-I$(APPS_DIR)/ipxbeui -I$(APPS_DIR)/star

$(OBJ_DIR)/%.o: %.c
	$(CC) -c -o $(OBJ_DIR)/$*.o $(CFLAGS) $(EXTRACFLAGS) $<

SRCS=zotmain_code1.c

OBJS=${SRCS:%.c=$(OBJ_DIR)/%.o}

DST=./$(DST_NAME)

$(DST_NAME): ${OBJS} prod
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(PROD_LIBS) 
	cp $(DST_NAME) $(PROD_NAME).bin
	$(STRIP) $(PROD_NAME).bin
	cp $(PROD_NAME).bin TEMP.bin
	$(TOOLS_DIR)/GZIP -c TEMP.bin > $(PROD_NAME).gz
	rm TEMP.bin
	$(TOOLS_DIR)/maketarget 
	$(TOOLS_DIR)/makimage $(PSMODELINDEX) $(CODE1MARK) $(MajorVer) $(MinorVer) $(ReleaseVer) $(RDVersion) $(BuildVer) $(MAKER_AND_CPU) $(PROD_NAME).gz MPS$(PSMODELINDEX)_code1.bin
	cp $(ROOT_DIR)/img/MPS$(PSMODELINDEX)_loader.bin .
	$(TOOLS_DIR)/makimage $(PSMODELINDEX) $(ALLFLASHMARK) $(MajorVer) $(MinorVer) $(ReleaseVer) $(RDVersion) $(BuildVer) $(MAKER_AND_CPU) $(FLASH_SIZE) $(IMAGE_COUNT) $(LOADERMARK) MPS$(PSMODELINDEX)_loader.bin $(CODE1MARK) MPS$(PSMODELINDEX)_code1.bin MPS$(PSMODELINDEX)LO.bin
	cp MPS$(PSMODELINDEX)_code1.bin $(ROOT_DIR)/img/.
	cp MPS$(PSMODELINDEX)LO.bin $(ROOT_DIR)/img/.
	
include $(FTR_MAK)
