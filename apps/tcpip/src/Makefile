# The following variable MUST be defined in the environment
#    ROOT_DIR     :: fully qualified name of root of entire source tree
#    HDR_MAK      :: fully qualified name of the common header makefile
#    FTR_MAK      :: fully qualified name of the common footer makefile
#

include $(HDR_MAK)

# Names
DST_NAME = tcpip.a

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
EXTRACFLAGS += -I../incl -I$(PROD_BUILD_DIR) -I$(APPS_DIR)/ps/common/incl -I$(PKG_INSTALL_DIR)/include

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
SRCS=etharp.c tcpip.c ip.c icmp.c ip_addr.c ip_frag.c sockets.c api_lib.c api_msg.c err.c \
		mem.c memp.c sys.c pbuf.c stats.c raw.c udp.c tcp.c tcp_in.c tcp_out.c dhcp.c\
		netif.c inet.c sys_arch.c linklip.c

ifeq ($(CHIP),mt7688)
SRCS += eth_drv.c init.c
endif

# DO not edit OBJS and DST
OBJS=${SRCS:%.c=$(OBJ_DIR)/%.o}
DST=$(LIB_DIR)/$(DST_NAME)

$(DST_NAME): ${OBJS}
	$(AR) $(ARFLAGS) $(DST) $?

include $(FTR_MAK)

