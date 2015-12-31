# The following variable MUST be defined in the environment
#    ROOT_DIR     :: fully qualified name of root of entire source tree
#    HDR_MAK      :: fully qualified name of the common header makefile
#    FTR_MAK      :: fully qualified name of the common footer makefile
#

include $(HDR_MAK)

# Names
DST_NAME = common.a

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
				-I$(APPS_DIR)/spooler/incl -I$(APPS_DIR)/ps/ntps/incl\
				-I$(APPS_DIR)/ipxbeui/incl \
				-I$(APPS_DIR)/ps/atalk/incl\
				-I$(APPS_DIR)/tcpip/incl\
				-I$(APPS_DIR)/mac/incl	\
				-I$(APPS_DIR)/star/incl
				
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
SRCS=uart.c print.c psmain.c eeprom.c rwflash.c led.c pkdriver.c io.c spiflash.c crc32.c pm.c bridge.c

# DO not edit OBJS and DST
OBJS=${SRCS:%.c=$(OBJ_DIR)/%.o}
DST=$(LIB_DIR)/$(DST_NAME)

$(DST_NAME): ${OBJS}
	$(AR) $(ARFLAGS) $(DST) $?

include $(FTR_MAK)

