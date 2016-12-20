#                Copyright 2003, Marvell International Ltd.
# This code contains confidential information of Marvell Semiconductor, Inc.
# No rights are granted herein under any patent, mask work right or copyright
# of Marvell or any third party.
# Marvell reserves the right at its sole discretion to request that this code
# be immediately returned to Marvell. This code is provided "as is".
# Marvell makes no warranties, express, implied or otherwise, regarding its
# accuracy, completeness or performance.

#
# Makefile for AP31 bridge module
#
# History:
#    8/17/03   cvu         created
#

# The following variable MUST be defined in the environment
#    ROOT_DIR     :: fully qualified name of root of entire source tree
#    HDR_MAK      :: fully qualified name of the common header makefile
#    FTR_MAK      :: fully qualified name of the common footer makefile
#

include $(HDR_MAK)

# Names
DST_NAME = http_zot.a

BIN_DIR = ../../Bin
WEB_DIR = ../../Web

all: DIR_CHECK $(DST_NAME)
	
# MAKE_LIB_DIR and MAKE_OBJ_DIR are defined in $(FTR_MAK)
DIR_CHECK: MAKE_LIB_DIR MAKE_OBJ_DIR

###### OVERRIDE COMMON MAKEFILE VARIABLES HERE
# Override the variables defined in $(HDR_MAK) here
# The following directory variables are defined in $(HDR_MAK)
# SRC_DIR       =
OBJ_DIR       = $(PROD_BUILD_DIR)/obj
LIB_DIR       = $(PROD_BUILD_DIR)/lib

###### FLAGS
EXTRACFLAGS += -DWEBS -DUEMF -DWEBS_PAGE_ROM -DOS="eCos" -D__NO_FCNTL=1

EXTRACFLAGS +=  -I$(PROD_BUILD_DIR) -I../incl  -I$(APPS_DIR)/ps/common/incl \
				-I$(APPS_DIR)/ps/psutility/incl -I$(APPS_DIR)/ps/ntps/incl \
			    -I$(APPS_DIR)/spooler/incl \
			    -I$(APPS_DIR)/ps/ippd/incl \
			    -I$(APPS_DIR)/ps/nds/incl \
			    -I$(APPS_DIR)/ps/atalk/incl \
			    -I$(APPS_DIR)/tcpip/incl \
 				-I$(APPS_DIR)/realtek/incl



###### LOCAL BUILD RULES
#
$(OBJ_DIR)/%.o: %.c
	$(CC) -c -o $(OBJ_DIR)/$*.o $(CFLAGS) $(EXTRACFLAGS) $<
	
###### MANDATORY LOCAL VARIABLES
# The local makefile must define the following:
#    SRCS     :: list of source files
#    OBJS     :: list of object files
#    DST      :: name of build product, like a library
# You need only add new file to the list in SRCS
SRCS = http_constant.c gmtime.c pfs.c httpd.c

ifeq ($(TARGET),NDWP2020)
SRCS += endmark_dwp2020_tp.c
endif
ifeq ($(TARGET),N716U2W)
SRCS += endmark_716u2w_tp.c
endif
ifeq ($(TARGET),N716U2)
SRCS += endmark_716u2_tp.c
endif

# DO not edit OBJS and DST
OBJS=${SRCS:%.c=$(OBJ_DIR)/%.o}
DST=$(LIB_DIR)/$(DST_NAME)

$(DST_NAME): ${OBJS}
#	rm webrom.* web_files
	$(AR) $(ARFLAGS) $(DST) $?

include $(FTR_MAK)

#webrom.c:
#	/bin/find $(WEB_DIR) -name "*.*" >web_files
#	$(BIN_DIR)/webcomp $(WEB_DIR) web_files >webrom.c


#ISL2HEX.exe FW1010.arm ISLIMAGE ISLIMAGE.c
