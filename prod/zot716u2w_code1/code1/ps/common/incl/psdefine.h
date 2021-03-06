#ifndef _PSDEFINE_H
#define _PSDEFINE_H

//for TFTPD
#define TFTP_CONFIG_LNAME 		"config.txt"
#define	TFTP_CONFIG_UNAME 		"CONFIG.TXT"
#define TFTP_UPGRADE_LNAME 		"mps%02d.bin"
#define TFTP_UPGRADE_UNAME		"MPSXX.BIN"
#define TFTP_UPGRADE_C1LNAME	"MPS%02dLO.BIN"
#define TFTP_UPGRADE_C1NAME		"MPSXXLO.BIN"

//for Printer Port Name
#define G_PRN_PORT_NAME   	"lpx"
#define G_PRN_PORT_POS    	(sizeof(G_PRN_PORT_NAME) - 2)

//PSMode type:
#define PS_DHCP_ON        	0x80
#define PS_NETWARE_MODE   	0x01
#define PS_UNIX_MODE      	0x02
#define PS_WINDOWS_MODE   	0x04
#define PS_ATALK_MODE     	0x08
#define PS_NDS_MODE       	0x10
#define PS_IPP_MODE       	0x20
#define PS_SMB_MODE       	0x40

//PSMode2 type:
#define PS_RAWTCP_MODE      0x01
#define PS_FTP_MODE			0x02
#define PS_WEBJETADMIN_ON	0x04
#define PS_CENTRAL_PRINT	0x80

//PSUpgradeMode type:
#define NOT_UPGRADE_MODE	0x00
#define WAIT_UPGRADE_MODE	0x01
#define IPX_UPGRADE_MODE	0x02
#define TFTP_UPGRADE_MODE	0x03

#endif  _PSDEFINE.H
