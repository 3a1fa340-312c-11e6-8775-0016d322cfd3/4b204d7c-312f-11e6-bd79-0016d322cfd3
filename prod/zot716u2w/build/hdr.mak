#                Copyright 2006, ZOT.

CC   = $(GCC_DIR)/arm-elf-gcc -mcpu=arm9 -w
LD   = $(GCC_DIR)/arm-elf-gcc
AR   = $(GCC_DIR)/arm-elf-ar
STRIP = $(GCC_DIR)/arm-elf-strip 

include $(TARGET_DEF)
C_DEFINED = -D$(HTML_FILE)

INC_DIR   = -I$(PKG_INSTALL_DIR)/include
CFLAGS    = -Wall $(INC_DIR) -ffunction-sections -fdata-sections
CFLAGS   += -D__ECOS -DECOS
CFLAGS   += -DVERSION=\"$(VERSION_STRING)\"

#add include 
CFLAGS   += $(C_DEFINED)

LDFLAGS   = -nostartfiles -nostdlib -L$(PKG_INSTALL_DIR)/lib -Wl,--gc-sections\
    -Ttarget.ld -Xlinker -Map -Xlinker $(basename $@).map
ARFLAGS   = rv
#COPTFLAGS     = -O3
COPTFLAGS     = -g
DEPEND_FLAGS =  -Wp,-MD,$*.d
EXTRACFLAGS   = $(COPTFLAGS) $(DEPEND_FLAGS)
LIBS          = -Ttarget.ld -nostdlib
SRC_DIR         = .

#-nostdlib

.PHONY: depend all clean

%.o: %.c
	$(CC) -c -o $*.o $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $<

%.d: %.c
	$(CC) -E $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $< >/dev/null
