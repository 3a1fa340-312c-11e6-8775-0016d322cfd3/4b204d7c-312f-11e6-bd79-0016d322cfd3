# Makefile

include $(TOPDIR)/rules.mak
include $(TOPDIR)/.config

EXTRA_CFLAGS = -I. -D_KERNEL

EXTRACFLAGS += -I../incl -I$(PROD_BUILD_DIR) -I$(APPS_DIR)/ps/common/incl -I$(APPS_DIR)/../drivers/ra305x_drivers

ALL_OBJS = ralink_gpio.o


MODULE = $(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)

clean:
	rm -f $(MODULE) $(ALL_OBJS)
	rm -f *.d
