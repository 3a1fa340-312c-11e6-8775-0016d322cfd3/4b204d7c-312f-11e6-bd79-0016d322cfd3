#ifndef _TFTPD_H
#define _TFTPD_H

#define TFTPD_TRY_TIMES			 2

#if defined(_PC) || defined(N6300II) || defined(DEF_PRINTSPEED)
#define CONFIG_FILE_NORMAL_NUM   9
#else
#define CONFIG_FILE_NORMAL_NUM   8
#endif PC || N6300II

//12/31/99 Doesn't include password !!
#define CONFIG_FILE_LINE_NUM (CONFIG_FILE_NORMAL_NUM + NUM_OF_PRN_PORT)

//#define htons  WordSwap  //move to swap.h	 2/24/99
//#define ntohs  WordSwap  //move to swap.h  2/24/99
#define	PKTSIZE	SEGSIZE+4

#define ASCII_TRANSFER_MODE 	"NETASCII"
#define BINARY_TRANSFER_MODE 	"OCTET"

#define far

typedef struct {
	int	e_code;
	char far* e_msg;
}PACK tftpd_errmsg;

typedef struct  {
	int   attrib;
    char  far* Cmd;
	void  (far* Func)(void);
}PACK tftpdreqcmd;

#define METHOD_RRQ				0x01
#define METHOD_WRQ				0x02
#define MODE_ASCII				0x10
#define MODE_BINARY				0x20

//void tftpd(int nouse,void *nouse1,void *nouse2);
void tftpd(cyg_addrword_t data);
void TftpEraseCode2(void);
void TftpEraseEE(void);
void TftpRebootBox(void);
void TftpResetBox(void);
void TftpUpgradeBox(void);
void TftpPrinter1Test(void);
void TftpPrinter2Test(void);
void TftpPrinter3Test(void);

BYTE *AddrToIP(char *IPAddr);

void TftpDLFlash(void);	//CODE1
void TftpDLCode1(void);

BYTE SendPrintTestData (int Port);  //9/9/99

#endif  _TFTPD_H
