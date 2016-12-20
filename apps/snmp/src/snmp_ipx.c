#include <stdio.h>
#include <cyg/kernel/kapi.h>	//615wu
#include "network.h"    //615wu
#include "pstarget.h"
#include "psglobal.h"
#include "ipx.h"
#include "asn1.h"
#include "eeprom.h"		//615wu
#include "snmp_api.h"
#include "snmpaget.h"
#include "nds.h"	//615wu
#include "ndsext.h"
#include "snmpgrup.h"	//615wu
#include "snmp_ipx.h"

#ifdef SNMPIPX

//615wu
//SnmpdIPX1 Task create information definition
#define SnmpdIPX1_TASK_PRI         20	//ZOT716u2
#define SnmpdIPX1_TASK_STACK_SIZE	 2048
static uint8 			SnmpdIPX1_Stack[SnmpdIPX1_TASK_STACK_SIZE];
static cyg_thread       SnmpdIPX1_Task;
static cyg_handle_t     SnmpdIPX1_TaskHdl;
cyg_sem_t SNMPovIPXESR_sem;

//SnmpdIPX2 Task create information definition
#define SnmpdIPX2_TASK_PRI         20	//ZOT716u2
#define SnmpdIPX2_TASK_STACK_SIZE	 2048
static uint8 			SnmpdIPX2_Stack[SnmpdIPX2_TASK_STACK_SIZE];
static cyg_thread       SnmpdIPX2_Task;
static cyg_handle_t     SnmpdIPX2_TaskHdl;

#define DataCopy memcpy	//615wu

#ifdef WEBADMIN // add by ---- Arius 6/20/2000
extern uint8   JetModel[];
#endif WEBADMIN

#define SnmpIPXSocket 0x0F90   //900F, RFC1420 (SNMP over IPX)
#define IPXDiagSocket 0x5604   //0456

ECB  SnmpInECB,   SnmpOutECB;
IPXHeader SnmpInIPXHeader, SnmpOutIPXHeader;
BYTE SnmpInData[SNMP_MAX_LEN], SnmpOutData[SNMP_MAX_LEN];

/*
#define DIAG_DATA_LEN  10
ECB  DiagInECB,   DiagOutECB;
IPXHeader DiagInIPXHeader, DiagOutIPXHeader;
BYTE DiagInData[DIAG_DATA_LEN], DiagOutData[DIAG_DATA_LEN];
*/

#define IPX_COMMUNITY_MAX_LEN 80
uint8   ipx_community[IPX_COMMUNITY_MAX_LEN+1];  //Input community name

snmp_session    ipx_session = {
        ipx_community,            // community name
        0,                        // community name length
        SNMP_DEFAULT_REMPORT,     // peer host port
        36879,                    // snmp daemon port
        NULL,
        NULL,                     // call back message that I want to sepecify
        NULL,                     // socket file description
        {NULL},                   // peer socket_internet_address
        {NULL},                   // request list
        0                         // access
};

static void SnmpIPXAgent1(int nouse, void *nouse1,void *nouse2);
static void SnmpIPXAgent2(int nouse, void *nouse1,void *nouse2);
static void ListenSNMPovIPX (void);
static void SNMPovIPXESR (void);

static void ListenDiagPacket (void);
static void DiagIPXESR (void);

static void ListenHPLaserJetSAP (void);
static void HPSapESR (void);
static void SendHPSap(void);

void SnmpIPXInit(void)
{
        WORD SocketData = SnmpIPXSocket;

        if(IPXOpenSocket((BYTE *)&SocketData,LONG_LIVED) != OKAY) {
#ifdef PC_OUTPUT
                printf(" Can't Open SNMPovIPX Socket\n");
                exit(0);
#endif
                return;
        }
/*
        SocketData = IPXDiagSocket;
        if(IPXOpenSocket((BYTE *)&SocketData,LONG_LIVED) != OKAY) {
#ifdef PC_OUTPUT
                printf(" Can't Open IPXDiag Socket\n");
                exit(0);
#endif
                return;
        }
*/

//615wu
		//Create SnmpdIPX1 Thread
	    cyg_thread_create(SnmpdIPX1_TASK_PRI,
	                  SnmpIPXAgent1,
	                  0,
	                  "SnmpdIPX1",
	                  (void *) (SnmpdIPX1_Stack),
	                  SnmpdIPX1_TASK_STACK_SIZE,
	                  &SnmpdIPX1_TaskHdl,
	                  &SnmpdIPX1_Task);
		
		//Start SnmpdIPX1 Thread
		cyg_thread_resume(SnmpdIPX1_TaskHdl);

        ListenSNMPovIPX();
                HPJetAdminSAPInit();
        ListenHPLaserJetSAP();
//      ListenDiagPacket();
}

void ListenSNMPovIPX (void)
{
        // Setup Receive-ECB Block
        SnmpInECB.ESRAddress = SNMPovIPXESR;
        SnmpInECB.inUseFlag  = 0;
        SnmpInECB.socketNumber = SnmpIPXSocket;
        SnmpInECB.fragmentCount = 2;
        SnmpInECB.fragmentDescriptor[0].address = &SnmpInIPXHeader;
        SnmpInECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
        SnmpInECB.fragmentDescriptor[1].address = &SnmpInData;
        SnmpInECB.fragmentDescriptor[1].size    = SNMP_MAX_LEN;

        IPXListenForPacket (&SnmpInECB);
}

void SNMPovIPXESR (void)
{
	cyg_semaphore_post(&SNMPovIPXESR_sem);	//615wu
}

void SnmpIPXAgent1(int nouse, void *nouse1,void *nouse2)
{
        cyg_semaphore_init( &SNMPovIPXESR_sem, 0 );	//615wu
        while(1) {
				cyg_semaphore_wait(&SNMPovIPXESR_sem);	//615wu	
				
                if(availmem() == 0) {
//615wu
						//Create SnmpdIPX2 Thread
					    cyg_thread_create(SnmpdIPX2_TASK_PRI,
					                  SnmpIPXAgent2,
					                  0,
					                  "SnmpdIPX2",
					                  (void *) (SnmpdIPX2_Stack),
					                  SnmpdIPX2_TASK_STACK_SIZE,
					                  &SnmpdIPX2_TaskHdl,
					                  &SnmpdIPX2_Task);
						
						//Start SnmpdIPX2 Thread
						cyg_thread_resume(SnmpdIPX2_TaskHdl);	
						cyg_thread_yield();	//615wu
                }
                sys_check_stack();
        }
}

void SnmpIPXAgent2(int nouse, void *nouse1,void *nouse2)
{
        int16 SnmpOutLength;

        snmpInPkts++ ;

        SnmpOutLength = SNMP_MAX_LEN;
                if(snmp_agent_parse(&ipx_session, SnmpInData, SnmpInECB.fragmentDescriptor[1].recv_size, SnmpOutData, &SnmpOutLength))
        {
                snmp_outpkts++;

                SnmpOutECB.ESRAddress = 0x00;
                SnmpOutECB.inUseFlag  = 0;
                SnmpOutECB.socketNumber = SnmpIPXSocket;
                DataCopy(SnmpOutECB.immediateAddress,SnmpInECB.immediateAddress, 6);

                SnmpOutECB.IPXFrameType = SnmpInECB.IPXFrameType;

                SnmpOutECB.fragmentCount = 2;
                SnmpOutECB.fragmentDescriptor[0].address = &SnmpOutIPXHeader;
                SnmpOutECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
                SnmpOutECB.fragmentDescriptor[1].address = &SnmpOutData;
                SnmpOutECB.fragmentDescriptor[1].size    = SnmpOutLength;

                // Setup IPX Header
                SnmpOutIPXHeader.packetType = 0;  // UnKnown Type
                DataCopy(SnmpOutIPXHeader.destination.network,SnmpInIPXHeader.source.network,12);

                // Send IPX Packet
                do{
#ifdef WEBADMIN
                        RunChangeIPAddress();
#ifndef _PC
                        RunWriteDataToEEPROM();
#endif
#endif WEBADMIN

                        IPXSendPacket(&SnmpOutECB);
                } while(SnmpOutECB.inUseFlag) ;

                ListenSNMPovIPX();
        } // if(snmp_agent_parse ...
}

/*
void ListenDiagPacket (void)
{
        // Setup Receive-ECB Block
        DiagInECB.ESRAddress = DiagIPXESR;
        DiagInECB.inUseFlag  = 0;
        DiagInECB.socketNumber = IPXDiagSocket;
        DiagInECB.fragmentCount = 2;
        DiagInECB.fragmentDescriptor[0].address = &DiagInIPXHeader;
        DiagInECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
        DiagInECB.fragmentDescriptor[1].address = &DiagInData;
        DiagInECB.fragmentDescriptor[1].size    = DIAG_DATA_LEN;

        IPXListenForPacket (&DiagInECB);
}

void DiagIPXESR (void)
{
        DiagOutECB.ESRAddress = 0x00;
        DiagOutECB.inUseFlag  = 0;
        DiagOutECB.socketNumber = IPXDiagSocket;
        DataCopy(DiagOutECB.immediateAddress,DiagInECB.immediateAddress, 6);

        DiagOutECB.IPXFrameType = DiagInECB.IPXFrameType;

        DiagOutECB.fragmentCount = 2;
        DiagOutECB.fragmentDescriptor[0].address = &DiagOutIPXHeader;
        DiagOutECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
        DiagOutECB.fragmentDescriptor[1].address = &DiagOutData;
        DiagOutECB.fragmentDescriptor[1].size    = DIAG_DATA_LEN;

        // Setup IPX Header
        DiagOutIPXHeader.packetType = 0;  // UnKnown Type
        DataCopy(DiagOutIPXHeader.destination.network,DiagInIPXHeader.source.network,12);
        memcpy(DiagOutData,"\x01\x00\x48\x01\x02\x00\x02",7);

        // Send IPX Packet
        do{
        IPXSendPacket(&DiagOutECB);
        } while(DiagOutECB.inUseFlag) ;

        ListenDiagPacket();
}
*/

SAPResponseData           SAPOutData;
ECB       SAPInECB,       SAPOutECB;
IPXHeader SAPInIPXHeader, SAPOutIPXHeader;
BYTE      SAPInData[20];

void ListenHPLaserJetSAP (void)
{
        // Setup Receive-ECB Block
        SAPInECB.ESRAddress = HPSapESR;
        SAPInECB.inUseFlag  = 0;
        SAPInECB.socketNumber = SAPSocket;
        SAPInECB.fragmentCount = 2;
        SAPInECB.fragmentDescriptor[0].address = &SAPInIPXHeader;
        SAPInECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
        SAPInECB.fragmentDescriptor[1].address = &SAPInData;
        SAPInECB.fragmentDescriptor[1].size    = 20;

        IPXListenForPacket (&SAPInECB);
}

void HPJetAdminSAPInit(void)
{
        SAPOutECB.ESRAddress = 0x00;
        SAPOutECB.inUseFlag  = 0;
        SAPOutECB.socketNumber = SAPSocket;
//      DataCopy(SAPOutECB.immediateAddress,SAPInECB.immediateAddress, 6);
//      SAPOutECB.IPXFrameType = SAPInECB.IPXFrameType; //3/16/98

        SAPOutECB.fragmentCount = 2;
        SAPOutECB.fragmentDescriptor[0].address = &SAPOutIPXHeader;
        SAPOutECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
        SAPOutECB.fragmentDescriptor[1].address = &SAPOutData;
        SAPOutECB.fragmentDescriptor[1].size    = sizeof(SAPResponseData);

        // Setup IPX Header
//      SAPOutIPXHeader.packetType = 0;  // UnKnown Type
//      DataCopy(SAPOutIPXHeader.destination.network,SAPInIPXHeader.source.network,12);

        // Setup SAP Response Data
        SAPOutData.ResponseType = 0x0200; //General Service response
        SAPOutData.ServerType = 0x0C03;  //HPLaserJet  0x30C

        memcpy(SAPOutData.Node,MyPhysNodeAddress,6);
        SAPOutData.Socket = 0x0C40;               //high-low    0x400C
        SAPOutData.IntermediateNetworks = 0x0100; //high-low    0x01
}


void HPSapESR (void)
{
        BYTE i;

        if(NGET32(SAPInData) == 0x0C030100) {
                DataCopy(SAPOutECB.immediateAddress,SAPInECB.immediateAddress, 6);
                SAPOutECB.IPXFrameType = SAPInECB.IPXFrameType; //3/16/98
                SAPOutIPXHeader.packetType = 0;  // UnKnown Type
                DataCopy(SAPOutIPXHeader.destination.network,SAPInIPXHeader.source.network,12);
                memcpy(SAPOutData.Network,MyNetworkAddress,4);
                for(i = 0 ; i < 3; i++) {
                        SendHPSap();
                        ppause(urandom(10)+1);  //delay 1 - 10 ms 9/9/99
                }
                IPXListenForPacket (&SAPInECB);
        }
}

/*
void SendJetAdminSAP(void)
{
        DataCopy (SAPOutECB.immediateAddress, BrocastNode, 6);
        SAPOutECB.IPXFrameType = GetIPXFrameType(CurrentFrameType);
        SAPOutIPXHeader.packetType = 4;                // IPX Packet Type
        DataCopy (SAPOutIPXHeader.destination.network, BrocastNetwork, 4);
        DataCopy (SAPOutIPXHeader.destination.node, BrocastNode, 6);
        NSET16(SAPOutIPXHeader.destination.socket, SAPSocket);
        memcpy(SAPOutData.Network,MyNetworkAddress,4);
        SendHPSap();
}
*/

void SendHPSap(void)
{
        BYTE PrnPort = 0;

#if (NUM_OF_PRN_PORT > 1)
        for(PrnPort = 0; PrnPort < NUM_OF_PRN_PORT; PrnPort++) {
#endif
#if (NUM_OF_PRN_PORT > 1)
                sprintf(SAPOutData.ServerName,"%02X%02X%02X%02X%02X%02X%d0%s%s_P%d",
#else
                sprintf(SAPOutData.ServerName,"%02X%02X%02X%02X%02X%02X%d0%s%s",
#endif

                        MyPhysNodeAddress[0],
                        MyPhysNodeAddress[1],
                        MyPhysNodeAddress[2],
                        MyPhysNodeAddress[3],
                        MyPhysNodeAddress[4],
                        MyPhysNodeAddress[5],
#if (NUM_OF_PRN_PORT > 1)
                        PrnPort+1,
#else
                        PrnPort,
#endif
#ifdef WEBADMIN
                        JetModel,
#endif WEBADMIN 
#if (NUM_OF_PRN_PORT > 1)
//                        Hostname,
                          EEPROM_Data.PrintServerName,
#else
//                        Hostname
                          EEPROM_Data.PrintServerName

#endif

#if (NUM_OF_PRN_PORT > 1)
                        PrnPort+1
#endif
                );

                // Send IPX Packet
                do {
                        IPXSendPacket(&SAPOutECB);
                } while (SAPOutECB.inUseFlag) ;
#if (NUM_OF_PRN_PORT > 1)
        }
#endif

}

#endif SNMPIPX
