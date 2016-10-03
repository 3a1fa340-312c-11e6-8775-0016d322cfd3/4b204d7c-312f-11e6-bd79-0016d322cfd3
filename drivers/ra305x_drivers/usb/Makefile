# Makefile

include $(TOPDIR)/rules.mak
include $(TOPDIR)/.config

EXTRA_CFLAGS = -I. -D_KERNEL 

#EXTRACFLAGS += -I../incl -I$(PROD_BUILD_DIR) -I$(APPS_DIR)/ps/common/incl -I$(APPS_DIR)/../drivers/ra305x_drivers
EXTRACFLAGS += -I./include -I./include/asm -I$(PROD_BUILD_DIR) \
			   -DCONFIG_RALINK_MT7628 \
			   -DCONFIG_USB_ANNOUNCE_NEW_DEVICES \
			   -DUSB_LED 
#
#ALL_OBJS = host/ehci-hcd.o host/ehci-rt3xxx.o host/ehci-hub.o \
#		   host/ohci-hcd.o host/ohci-rt3xxx.o host/hoci-hub.o \
#		   core/hcd.o core/hub.o core/urb.o core/usb.o core/urb.o core/message.o
#
ALL_OBJS = os-dep.o core/klist.o core/usb.o core/config.o core/hub.o core/driver.o core/generic.o core/message.o core/urb.o \
		   core/endpoint.o core/hcd.o core/quirks.o core/buffer.o \
		   host/ehci-hcd.o host/ohci-hcd.o host/dmapool.o host/mtk-phy-ahb.o host/mtk-phy.o host/mtk-phy-7628.o \
		   class/usblp.o class/hp1020.o class/zot_usb_ops.o

MODULE = $(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)

clean:
	rm -f $(MODULE) $(ALL_OBJS)
	rm -f *.d
	rm -f ./core/*.d
	rm -f ./host/*.d
	rm -f ./class/*.d

