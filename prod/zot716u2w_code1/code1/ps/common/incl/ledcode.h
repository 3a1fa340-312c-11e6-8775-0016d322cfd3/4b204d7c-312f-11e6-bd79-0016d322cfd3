#ifndef _LEDCODE_H
#define _LEDCODE_H

/*
(1)
First non Zero Nibble will flash RED-LIGHT, other will flash GREEN-LIGHT

	     ex: 0x0123 : skip(0),R(1),G(2),G(3) ===> R(1),G(2),G(3)
	same as  0x1023 : R(1),skip(0),G(2),G(3) ===> R(1),G(2),G(3)
    same as  0x1230 : R(1),G(2),G(3),skip(0) ===> R(1),G(2),G(3)

    ==> so don't set '0' at any nibble

(2) Can Set 4 Nibble for Flash Code :
ex: 0x1231 : R(1), G(2), G(3), G(1)

(3)	For easy easy reading purpose : don't let any nibble larger than 3.

*/
#define LED_MAINTEST     0x11    //for maintest.c

//////////// for CODE1 only ///////////////////////////////////////
#define	LED_DL_INIT      0x311   //Upgrade initial fail ! (Erase Flash Fail)
#define LED_WR_FLASH     0x312   //Write Data to Flash fail !

#define LED_CODE1_STEP1	 0x21  //don't change this value ( asmcode1.c ) RAM_ERROR
#define LED_CODE1_STEP2	 0x22  //NIC RESET TEST ERROR !
#define LED_CODE1_STEP3	 0x23  //NIC SRAM TEST ERROR !
#define LED_CODE1_STEP4	 0x24  //Read EEPROM TEST ERROR ! (Need update EEPROM)
#define LED_CODE1_STEP5  0x25  //NO USED

///////////////////////////////////////////////////////////////////
#define LED_NET_TRY_CONN 0x11    //try to connect to netware server
#define LED_PS_SOCKET	 0x111   //can not open PS   socket
#define LED_SAP_SOCKET	 0x112   //can not open SAP  socket
#define LED_WDOG_SOCKET	 0x113   //can not open WDOS socket
#define LED_UTIL_SOCKET	 0x121   //can not open UTIL socket
#define LED_NT_SOCKET	 0x122   //can not open NT   socket
#define LED_PKD_ERROR	 0x123   //initial packet driver error
#define LED_WR_EEPROM	 0x131   //write eeprom error
#define LED_RD_EEPROM	 0x132   //read eeprom error
#define LED_HTTP_WR_EEPROM 0x321 //http write eeprom error 6/22/99
#define LED_LOW_MEMORY	 0x133   //Not enough memory !
#define LED_EAT_QUEUE    0x12    //Compare queue pattern errror	5/19/99

#define LED_HOLD_JOB_ERROR   0x211   //LPD.C HOLD_JOB() error !
#define LED_SEARCH_FS_SOCKET 0x212   //MPS3.C open SearchFSocket fail !
///////////////////////////////////////////////////////////////////
//Internal design error
#define LED_SNMP_COMM_ERROR   0x4111   //SNMP COMMUNITY DESIGN ERROR !

#endif  _LEDCODE_H
