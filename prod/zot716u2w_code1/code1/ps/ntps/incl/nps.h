#ifndef _NPS_H
#define _NPS_H

#define SERVICE_NEXT_PS   0x10
//---------- Printer Port Status -----------------
#define PRINTER_FREE	  0x00
#define PRINTER_USED	  0x01
#define PRINTER_HOLD      0x02

#define PRINTER_PAPER_OUT 0x01
#define PRINTER_OFF_LINE  0x02
#define PRINTER_BUSY      0x03

//---------- Bindery Object Type ------------------
#define FILE_SERVER_OBJECT         0x0400  //0004
#define DS_SERVER_OBJECT           0x7802  //0278 Directory Server Object

//---------- Novell defined Socket Number ---------
#define RIPSocket                  0x5304 // 0453
#define SAPSocket                  0x5204 // 0452

//--------- Self defined Socket Number ------------
#define PsSocket                   0x1540 // 4015
#define WatchDogSocket             0x1640 // 4016
#define UtilitySocket              0x5050 // 5050
#define SearchFSocket              0x5151 // 5151 //8/13/98 for HTTP Search FS name

//---------- NCP define code ----------------------
#define NCP_REQUEST                0x11
#define NCP_REPLY                  0x3333
#define NCP_ABORT_CONNECTION       0X5555
#define NCP_FILE_SERVER_BUSY       0x9999

#define SERVER_OK                  0x0
#define SERVER_BUSY                0x1
#define SERVER_ABORT               0x2
#define SERVER_LISTEN_NEXT         0x3

#define NCPSTATUS_SHUTDOWN_BAD     0x50
#define NCPSTATUS_DISCONNECT       0x01

//-------------------------------------------------
//Utility List View Model
#define UNIX                       0x01
#define NETWARE                    0x02
#define UNIX_NETWARE               0x03
#define NT_SINGLE_PORT             0x04
#define NT_MULTI_PORT              0x05

#define NTBEUI_VIEW_BOX            0x81

//-------------------------------------------------
#define NULL_BYTE                  0xFF
//-------------------------------------------------
#define CODE_FREE                  0x00
#define SUCCESSFUL                 0x00
#define OK                         0x00
#define FAILURE                    0xFF
//---------------------------------------------------------------------------
#define LEN_OF_PROPERTY_NAME       11
#define PROPERTY_NAME              "NET_ADDRESS"
//---------------------------------------------------------------------------
#define MAX_NAME_LEN			   48    //Maximun Name Length
#define MAX_JOB_READ_BYTE		   0x400 //1024 bytes
//#define MAX_PS_RECEIVE_LEN         (MAX_JOB_READ_BYTE+sizeof(NCPResponseData)+2)
#define MAX_PS_RECEIVE_LEN         (1500) //8/23/99
//#define MAX_PS_SEND_LEN			   (sizeof(NCPQueryData)+3+MAX_NAME_LEN+LEN_OF_PROPERTY_NAME+6)
#define MAX_PS_SEND_LEN			   (1500) //8/23/99
//MAX_PS_SEND_LEN for ReadPropertyValue()
#define MAX_PS_READ_LEN		       (1410) // Charles 2001/09/04

//-------------------------------------------------------
//#define MAX_RETRY_COUNT            10
#define MAX_RETRY_COUNT            4   //12/7/99

#define SAP_RETRY_TIME             ((BYTE)(TICKS_PER_SECOND/4))  //0.25 SEC
#define NCP_RETRY_TIME             ((BYTE)(TICKS_PER_SECOND))	  //1 SEC
#define RIP_RETRY_TIME             ((BYTE)(TICKS_PER_SECOND/8))  //0.125 SEC

//----------------------------------------------------------------------
#define MY_SERVER_TYPE             0xAA55 // 55AA , My Print Server Type
#define NOVELL_COPYRIGHT           "ZOT-N6100EP-SETUP" //Copyright
#define NT_COPYRIGHT               "ZO"
//----------------------------------------------------------------------
#define INVALID_FILE_HANDLE        0x88  //136

#define NO_SUCH_QUEUE              0xEA  //234
#define NO_SUCH_OBJECT             0xFC
#define NO_QUEUE_JOB               0xD5 //?? may be 0xfc

#define SERVICE_AGAIN              0x33 // Service Again  //5/14/98
#define SERVICE_DATA               0x55 // Start Service Queue
#define SERVICE_NEXT_QUEUE         0x77 // Service Next Queue
#define TIME_OUT                   0xFF // Time Out
//----------------------------------------------------------------------

#define NT_VERSION_2  1	  //old epwin & print monitor protocol	9/3/99
#define NT_VERSION_4  2	  //new epwin & print monitor protocol	9/3/99


//--- Utility Command define (Utility Send) -----------------------
#define CMD00     0x00   // Read EEPROM
#define CMD01     0x01   // Config Box
#define CMD02     0x02   // Request PRINTER_TEST (X)
#define CMD03     0x03   // ReBoot
#define CMD04     0x04   // View Box
#define CMD05     0x05   // Down Load Flash
#define CMD06     0x06   // Send Packet of Binary Code    (CODE1)
#define CMD07     0x07   // send Flash Success Acknowagle (CODE1)
#define CMD08     0x08   // Request Printer's Status
#define CMD0A     0x0A   // Start Print Service (NT only)
#define CMD0B     0x0B   // Request Print Data (NT only)
#define CMD0C     0x0C   // End Print Service (NT only)
#define CMD0D     0x0D	 // Write EEPROM
#define CMD0E     0x0E	 // Output Pattern to PORT

#define CMD10     0x10	 // Send Device Info 4/19/2000

#define CMD11	  0x11	 // Get Printer Data
#define CMD12	  0x12	 // Write Printer Data
#define CMD14	  0x14	 //	View Box only for 9312 QC1,9312 not support IPX (ignore '04')
#define CMD15	  0x15	 // Write Router and Printer server CFG data
#define CMD18	  0x18	 // Get firmware inf Data
#define CMD19	  0x19	 // Load Default settings
#define CMD1A	  0x1A	 // Flash Light
#define CMD1B	  0x1B	 // Print Port Loopback
#define CMD1C	  0x1C	 // Wireless Loopback1
#define CMD1D	  0x1D	 // Wireless Loopback2
#define CMD9D	  0x9D	 // Wireless Loopback2
#define CMD1E     0x1E   // USB Loopback
#define CMD1F	  0x1F	 // Wan Port Loopback Test
#define CMD21     0x21   // Config Box
#define CMD24     0x24   // View Box
#define CMD2D     0x2D	 // Write EEPROM > 1408
#define CMDFF     0xFF   // Check Bank of Box    ( for Q.C. )

//------------- Protocol data structure ----------------------------

//SAPQueryData
typedef struct
{
	WORD PacketType;
	WORD ServerType;
}PACK SAPQueryData;

//SAPResponseData
typedef struct
{
	WORD ResponseType;
	WORD ServerType;
	BYTE ServerName[MAX_NAME_LEN];
	BYTE Network[4];
	BYTE Node[6];
	WORD Socket;
	WORD IntermediateNetworks;
}PACK SAPResponseData;

//RIPData
typedef struct
{
	WORD Operation;
	BYTE Network[4];
	WORD Hops;
	WORD Tick;
}PACK RIPData;

// Watch Dog
typedef struct
{
	BYTE ConnectionNumber;
	BYTE SignatureChar;
}PACK WatchDog_Rec;

// NCPQueryData
typedef struct
{
	WORD RequestType;
	BYTE SequenceNumber;
	BYTE ConnectionNumberLow;
	BYTE TaskNumber;
	BYTE ConnectionNumberHigh;
}PACK NCPQueryData;

//NCPResponseData
typedef struct
{
	WORD ReplyType;
	BYTE SequenceNumber;
	BYTE ConnectionNumberLow;
	BYTE TaskNumber;
	BYTE ConnectionNumberHigh;
	BYTE CompletionCode;
	BYTE CompletionStatus;
}PACK NCPResponseData;

//------------ Novell Print Server Data Structure ------------------

//Queue ID
typedef struct
{
	BYTE       QueueObjectID[4];          // Queue Object ID
#ifdef _PC
	BYTE       QueueName[MAX_NAME_LEN];   // Queue Object Name
#endif
}PACK QueueIDStruct;

//File Server Info.
typedef struct 
{
	BYTE       NextFSInfo;
	BYTE      *PCBFileServerName;
	BYTE       PCBPhysicalID[6];
	BYTE       PCBNetworkNumber[12];
	BYTE       PCBSequenceNumber;
	BYTE       PCBConnectionNumberLow;
//	BYTE       PCBTask;
	BYTE       PCBConnectionNumberHigh;
//	WORD       (*ProcessRoutine)();
#ifdef NDS_PS
	WORD       QueueSize; //8/23/99 added
#endif NDS_PS
#ifdef PC_OUTPUT
	BYTE      *NDSTmpFileServerName;
#endif PC_OUTPUT
}PACK FSInfo;

typedef struct portpcb
{
	BYTE NextPortPCB;
	FSInfo*         pFSInfo;                // LinkFileServerInfo
	BYTE            FileHandle[6];          // Job File Handle
	uint32          ReadByteOffset;         // Read Byte offset
	BYTE            JOBNumber[2];
	BYTE            QCount;                 // Service Queue
	BYTE            TotalQueue;
//	BYTE            Port;
	QueueIDStruct   PCBQueueInfo[MAX_QUEUE];
	WORD            (*ProcessRoutine)(WORD Socket,struct portpcb *);
#ifdef NDS_PS
	BYTE            ClientStation;
	BYTE            JOBName[12];
	WORD            Copies;
#endif NDS_PS
}PACK PortPCB;

typedef struct
{
    //Pointer to PortPCB
	BYTE   FreePortPCB; //0 - 0xFE , 0xFF --> NULL
	BYTE   ActivePortPCB;
	BYTE   CurrentPortPCB;
#ifdef NDS_PS //11/19/99
	BYTE   NDSFreePortPCB;
	BYTE   NDSActivePortPCB;
	BYTE   NDSCurrentPortPCB;
	BYTE   CurServiceMode;   //0 -- BINDERY , 1 -- NDS
#endif NDS_PS
	uint32 StartTime;
}PACK PrnInfo;

//-------- Self define Protocol Data Structure ---------------------

//Novell Utility data structure
typedef struct
{
	BYTE Mark[17];   //17B User Define Protocol :"ZOT_N6100EP_SETUP"
	BYTE Cmd[2]; // 2B
	BYTE Data[512];
}PACK NOVELL_Utility_Rec;

//NT Utility data structure
typedef struct
{
	BYTE Mark[3];    // User Define Protocol :"NET"
	BYTE Type;       //50, 60 or 70
	BYTE Cmd[2];	 //command
	BYTE Data[MAX_PS_READ_LEN];  // Charles 2001/09/04
}PACK LONG_NT_Utility_Rec;

typedef struct
{
	BYTE Mark[3];    // User Define Protocol :"NET"
	BYTE Type;       //50, 60 or 70
	BYTE Cmd[2];	 //command
	BYTE Data[1024]; //7/3/2000 add some data for check detail version
	BYTE ExtMark[4];
	BYTE CODE1Ver[80];  //7/3/2000 add code1 version
	BYTE CODE2Ver[80];  //7/3/2000 add code2 version
	BYTE ModelName[40]; //(internal model name)
}PACK NAME_Utility_Rec;

//NT Utility data structure
typedef struct
{
	BYTE Mark[3];    // User Define Protocol :"NET"
	BYTE Type;       //50, 60 or 70
	BYTE Cmd[2];	 //command
	BYTE Data[1024];  //5/18/2000
}PACK NEW_NT_Utility_Rec;

//NT Utility data structure
typedef struct
{
	BYTE Mark[2];    // User Define Protocol :"ZO"
	BYTE Cmd[2];
	BYTE Data[512];
}PACK OLD_NT_Utility_Rec;

typedef union
{
	NOVELL_Utility_Rec     Novell;
	OLD_NT_Utility_Rec     Nt1;
	NEW_NT_Utility_Rec     Nt2;
	NAME_Utility_Rec       Nt3;	 //4/24/2000 for 1K FLASH ONLY
	LONG_NT_Utility_Rec    Nt4;
}PACK Utility_Rec;

/* 4/14/2000 marked
// Receive BOX (PS) Data Struct for UTILITY packet
typedef struct
{
	BYTE  Name[18];   // BOX (PS) Name
	BYTE  node[6];    // BOX's Network Address
	BYTE  Pname[MAX_NAME_LEN];  // Print Server Name
	BYTE  Fname[LENGTH_OF_FS_NAMES]; // Service File Servers Name

#ifdef SNMPD
	BYTE SnmpReserved[LENGTH_OF_RESERVED];
#endif SNMPD

	BYTE  NumOfPort;  // How many port are this printer support ? //10/29/98
	BYTE  status;     // Connect OK ??
	BYTE  ver[2];     // *Not same as EEPROM version*
	                  // low byte = H/W version , high byte = S/W version
	BYTE  CSIP[2];    // CS:IP  for Tracer Program
	WORD  LPDState[1];// state For Unix
	WORD  Model;      // System Type  UNIX(01) or NW (02)
} Bname;
*/

#endif  _NPS_H
