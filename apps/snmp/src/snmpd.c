#include <cyg/kernel/kapi.h>	//615wu
#include "network.h"    //615wu
#include "pstarget.h"	//615wu
#include "psglobal.h"
#include "asn1.h"
#include "mibmodl.h"
#include "eeprom.h"		//615wu
#include "snmp_api.h"	//615wu
#include "prnport.h"    //615wu
#include "snmpaget.h"
#include "snmptrap.h"
#include "snmpgrup.h"
#include "snmpd.h"

#ifdef SNMPIPX
#include "snmp_ipx.h"
#endif SNMPIPX

#define SNMP_TRAP_SPEC_PAPER_OUT     100
#define SNMP_TRAP_SPEC_OFF_LINE          101

void SnmpCheckPrintStatus(cyg_addrword_t data);	//615wu
void init_snmp_variable(void);

//615wu
//Snmpd Task create information definition
#define Snmpd_TASK_PRI         20	//ZOT716u2
#define Snmpd_TASK_STACK_SIZE	 4096 //ZOT716u2 4096
static uint8 			Snmpd_Stack[Snmpd_TASK_STACK_SIZE];
static cyg_thread       Snmpd_Task;
static cyg_handle_t     Snmpd_TaskHdl;


//SnmpCheckStatus Task create information definition
#define SnmpCheckStatus_TASK_PRI         20	//ZOT716u2
#define SnmpCheckStatus_TASK_STACK_SIZE	 2048 //ZOT716u2 4096
static uint8 			SnmpCheckStatus_Stack[SnmpCheckStatus_TASK_STACK_SIZE];
static cyg_thread       SnmpCheckStatus_Task;
static cyg_handle_t     SnmpCheckStatus_TaskHdl;

void snmpd(cyg_addrword_t data)
{
//Jesse add, for delay snmp init 2008/02/20
		ppause(5000);
    // allow all modules to init and register:
        init_modules();
        init_snmp_variable();

//615wu
		//Create Snmpd Thread
	    cyg_thread_create(Snmpd_TASK_PRI,
	                  SnmpAgent,
	                  0,
	                  "Snmpd",
	                  (void *) (Snmpd_Stack),
	                  Snmpd_TASK_STACK_SIZE,
	                  &Snmpd_TaskHdl,
	                  &Snmpd_Task);
		
		//Start Snmpd Thread
		cyg_thread_resume(Snmpd_TaskHdl);
#ifdef SNMPIPX
                SnmpIPXInit();
#endif SNMPIPX

        if(_SnmpTrapEnable == SNMP_TRAP_ENABLE)
        {
//615wu
			//Create SnmpCheckStatus Thread
		    cyg_thread_create(SnmpCheckStatus_TASK_PRI,
		                  SnmpCheckPrintStatus,
		                  0,
		                  "SnmpCheckStatus",
		                  (void *) (SnmpCheckStatus_Stack),
		                  SnmpCheckStatus_TASK_STACK_SIZE,
		                  &SnmpCheckStatus_TaskHdl,
		                  &SnmpCheckStatus_Task);
			
			//Start SnmpCheckStatus Thread
			cyg_thread_resume(SnmpCheckStatus_TaskHdl);
		}
		
		ppause (30000L);
        SendTrap (SNMP_TRAP_COLDSTART, SNMP_TRAP_COLDSTART,"Cold Start");
//      ppause (1500L);
//      SendTrap (SNMP_TRAP_LINKUP, SNMP_TRAP_LINKUP,"LINK UP");
}

void init_snmp_variable(void)
{

#ifdef _PC

#if (NO_OF_SNMP_COMMUNITY != 2)
        printf("SNMP COMMUNITY ERROR.......\n");
        for(1);
#endif

#else

#if (NO_OF_SNMP_COMMUNITY != 2)
        ErrLightOnForever(LED_SNMP_COMM_ERROR);
#endif

#endif _PC

        SnmpCommunityAuth[0].Community  = _SnmpCommunityAuthName[0];
        SnmpCommunityAuth[0].AccessMode = _SnmpComm0AccessMode;
        SnmpCommunityAuth[1].Community  = _SnmpCommunityAuthName[1];
        SnmpCommunityAuth[1].AccessMode = _SnmpComm1AccessMode;
}



//615wuvoid SnmpCheckPrintStatus(int noused0,void *noused1,void *noused2)
extern uint8   adjPortType[NUM_OF_PRN_PORT];
void SnmpCheckPrintStatus(cyg_addrword_t data)
{
        static BYTE PaperOutMessage[]= "Port 1 Paper Out";
        static BYTE OffLineMessage[]= "Port 1 Off Line";

        BYTE i;

        BYTE CurPrnStatus,LastPrnStatus;
        BYTE CurPortStatus, LastPortStatus;

#ifdef _PC
        LastPrnStatus = 3;
#else
        LastPrnStatus = ReadPrintStatus();
#endif

        for(;;) {
                ppause(5000L);

#ifdef _PC
                               //         +----> Off Line
                               //                -+-
                CurPrnStatus = 0x19;   //0 0 0 1 1 0 0 1
                                       //    -+-         -+-
                               //         |               +------> Paper Out
                               //     +----> Off Line
#else
                CurPrnStatus = ReadPrintStatus();
#endif

                for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
                        CurPortStatus = ( (CurPrnStatus & (3 << (i<<1) )) >> (i<<1) );
                        LastPortStatus = ( (LastPrnStatus & (3 << (i<<1) )) >> (i<<1) );

                        if(LastPortStatus == 0 || LastPortStatus == 3) {
                                //for show paper out
                                if( adjPortType[i] == PORT_PAPER_OUT ){ //paper out
    							        PaperOutMessage[5] = i+'1';
                                        SendTrap(SNMP_TRAP_ENTERPRISESPECIFIC, SNMP_TRAP_SPEC_PAPER_OUT,PaperOutMessage);
                                }
                                
                                if(CurPortStatus == 1) { //paper out
                                        PaperOutMessage[5] = i+'1';
                                        SendTrap(SNMP_TRAP_ENTERPRISESPECIFIC, SNMP_TRAP_SPEC_PAPER_OUT,PaperOutMessage);
                                }
                                if(CurPortStatus == 2) { //off line
                                        OffLineMessage[5] = i+'1';
                                        SendTrap(SNMP_TRAP_ENTERPRISESPECIFIC, SNMP_TRAP_SPEC_OFF_LINE,OffLineMessage);
                                }
                        }
                }
                LastPrnStatus = CurPrnStatus;
        }
}
