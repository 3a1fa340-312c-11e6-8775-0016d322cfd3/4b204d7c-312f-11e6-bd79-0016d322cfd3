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
DST_NAME = mtk7601.a

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
EXTRACFLAGS += -I../incl -I$(PROD_BUILD_DIR) -I$(APPS_DIR)/ps/common/incl \
				-I$(APPS_DIR)/spooler/incl -I$(APPS_DIR)/usb_host/incl -I$(APPS_DIR)/tcpip/incl	\
				-I$(APPS_DIR)/star/incl
MCONFIG = 	LINUX \
			RALINK_ATE \
			CONFIG_RT2880_ATE_CMD_NEW \
			RALINK_QA \
			WSC_STA_SUPPORT \
			WSC_V2_SUPPORT \
			IWSC_SUPPORT \
			WPA_SUPPLICANT_SUPPORT \
			DOT11_N_SUPPORT \
			STATS_COUNT_SUPPORT \
			CONFIG_RX_CSO_SUPPORT \
			MT7601 \
			MT7601U \
			RLT_MAC \
			RLT_RF \
			RTMP_MAC_USB \
			RTMP_USB_SUPPORT \
			RTMP_TIMER_TASK_SUPPORT \
			RX_DMA_SCATTER \
			VCORECAL_SUPPORT \
			RTMP_EFUSE_SUPPORT \
			NEW_MBSSID_MODE \
			RTMP_INTERNAL_TX_ALC \
			CONFIG_ANDES_SUPPORT \
			CONFIG_STA_SUPPORT \
			SCAN_SUPPORT \
			DPD_CALIBRATION_SUPPORT \
			RTMP_FREQ_CALIBRATION_SUPPORT \
			ADHOC_WPA2PSK_SUPPORT \
			CONFIG_WEXT_PRIV \
			DBG \
			MEMORY_OPTIMIZATION \
			RESOURCE_PRE_ALLOC 



#			OS_ABL_SUPPORT \
#			OS_ABL_FUNC_SUPPORT \
#			OS_ABL_OS_USB_SUPPORT \
#			OS_ABL_OS_STA_SUPPORT \
#			DPD_CALIBRATION_SUPPORT \



MODULEDEF = $(addprefix -D, $(MCONFIG))
EXTRACFLAGS += $(MODULEDEF)
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
#
SRCS = action.c assoc.c ate_usb.c auth.c auth_rsp.c ba_action.c cfg80211.c cmm_aes.c \
	   cmm_asic.c cmm_cfg.c cmm_cmd.c cmm_cs.c cmm_data.c cmm_data_usb.c cmm_info.c cmm_mac_usb.c \
	   cmm_profile.c cmm_radar.c cmm_sanity.c cmm_tkip.c cmm_wep.c cmm_wpa_adhoc.c cmm_wpa.c \
	   connect.c crypt_aes.c crypt_arc4.c crypt_biginteger.c crypt_dh.c crypt_hmac.c crypt_md5.c \
	   crypt_sha2.c ee_efuse.c rt_eeprom.c frq_cal.c mlme.c mt7601_ate.c mt7601.c mtk7601_dep.c \
	   ral_omac.c rlt_rf.c rt_ate.c rt_channel.c rt_linux.c rt_main_dev.c rt_profile.c rtmp_and.c \
	   rtmp_chip.c rtmp_data.c rtmp_init.c rtmp_init_inf.c rtmp_M51.c rtmp_mac.c rtmp_mcu.c rtmp_phy.c \
	   rtmp_timer.c rt_qa.c rt_usb.c rtusb_bulk.c rtusb_data.c rtusb_io.c sanity.c scan.c spectrum.c \
	   sta_cfg.c sta_ioctl.c sta_iwsc.c sync.c txpower.c usb_main_dev.c wpa.c wsc.c wlanif.c \
	   mgmt_entrytb.c cmm_sync.c rt_os_util.c mgmt_ht.c wsc_tlv.c ps.c ra_ctrl.c wsc_v2.c rt_usb_util.c \
	   alg_legacy.c rtusb_dev_id.c
	
# DO not edit OBJS and DST
OBJS=${SRCS:%.c=$(OBJ_DIR)/%.o}
DST=$(LIB_DIR)/$(DST_NAME)

$(DST_NAME): ${OBJS}
	$(AR) $(ARFLAGS) $(DST) $?

include $(FTR_MAK)

