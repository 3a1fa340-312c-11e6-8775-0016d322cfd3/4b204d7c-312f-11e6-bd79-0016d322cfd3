include $(TOPDIR)/rules.mak

ifeq ($(PROD_NAME),zot716u2)
	WEB_FOLDER = N716U2
endif

ifeq ($(PROD_NAME),zot716u2w)
	WEB_FOLDER = N716U2W
endif

ifeq ($(PROD_NAME),dwp2020)
	WEB_FOLDER = NDWP2020
endif

TPLINK_MARK = T
ZOT_MARK = E
ZOTCH_MARK = F
LS_MARK = L
LEVELO_MARK = N
IOG_MARK = I
ASSMAN_MARK = H
LINEUP_MARK = Z
ZOTCHS_MARK = X
DIGISO_MARK = D

all:
	echo '$(OEM)'
ifeq ("$(wildcard ./$(WEB_FOLDER)/O_$(OEM))","")
	$(error $(WEB_FOLDER)/O_$(OEM) folder is not exist!)
endif
	-rm end_mark.c
	./web2c.py $($(OEM)_MARK) ./$(WEB_FOLDER)/O_$(OEM) out_web;
	cp end_mark.c ../apps/http_zot/src/endmark.c
	rm end_mark.c
	./mktarget.py $(PROD_NAME) O_$(OEM)
