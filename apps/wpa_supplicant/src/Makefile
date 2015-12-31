#                Copyright 2003, Marvell International Ltd.
# This code contains confidential information of Marvell Semiconductor, Inc.
# No rights are granted herein under any patent, mask work right or copyright
# of Marvell or any third party.
# Marvell reserves the right at its sole discretion to request that this code
# be immediately returned to Marvell. This code is provided "as is".
# Marvell makes no warranties, express, implied or otherwise, regarding its
# accuracy, completeness or performance.

#
# Makefile for debug utilities module
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
DST_NAME = wpa_supplicant.a

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
EXTRACFLAGS += -I../realtek/incl -I$(PROD_BUILD_DIR) -I$(APPS_DIR)/ps/common/incl -I$(APPS_DIR)/realtek/incl/wpa_supplicant\
				-I$(APPS_DIR)/spooler/incl -I$(APPS_DIR)/usb_host/incl -I$(APPS_DIR)/tcpip/incl	\

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
SRCS=aes.c aes_wrap.c base64.c blacklist.c common.c config.c config_file.c wpa_main.c eloop.c\
	 events.c l2_packet_linux.c eap_wsc_common.c eap_wsc.c eapol_supp_sm.c eap_methods.c \
	 eap_common.c eap.c drivers.c dh_groups.c des.c ctrl_iface.c ctrl_iface_unix.c md4.c md5.c\
	 os_unix.c peerkey.c pmksa_cache.c preauth.c rc4.c scan.c scan_helpers.c sha1.c sha256.c \
	 tls_none.c uuid.c wpa.c wpabuf.c wpa_cli.c wpa_common.c wpa_ctrl.c wpa_debug.c wpa_ie.c \
	 wpa_passphrase.c wpas_glue.c wpa_supplicant.c wps.c wps_attr_build.c wps_attr_parse.c \
	 wps_attr_process.c wps_common.c wps_dev_attr.c wps_enrollee.c wps_registrar.c wps_supplicant.c \
	 driver_wext.c crypto_openssl.c wps_dep.c
	

# DO not edit OBJS and DST
OBJS=${SRCS:%.c=$(OBJ_DIR)/%.o}
DST=$(LIB_DIR)/$(DST_NAME)

$(DST_NAME): ${OBJS}
	$(AR) $(ARFLAGS) $(DST) $?

include $(FTR_MAK)

