# Makefile

include $(TOPDIR)/rules.mak
include $(TOPDIR)/.config

EXTRA_CFLAGS = -I. -D_KERNEL

#EXTRACFLAGS += -I../incl -I$(PROD_BUILD_DIR) -I$(APPS_DIR)/ps/common/incl -I$(APPS_DIR)/../drivers/ra305x_drivers
EXTRACFLAGS += -I./core -I./host -I$(PROD_BUILD_DIR) 

ALL_OBJS = host/ehci-hcd.o host/ehci-rt3xxx.o host/ehci-hub.o \
		   host/ohci-hcd.o host/ohci-rt3xxx.o host/hoci-hub.o \
		   core/hcd.o core/hub.o core/urb.o core/usb.o core/urb.o core/message.o


MODULE = $(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)

clean:
	rm -f $(MODULE) $(ALL_OBJS)
	rm -f *.d
