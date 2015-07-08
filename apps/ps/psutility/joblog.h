#ifndef	_JOBLOG_H
#define	_JOBLOG_H


#ifdef SUPPORT_JOB_LOG 
#define MAX_JOB_LIST			20
typedef  struct JOB_LOG
{
	uint32 JobID;				// Job ID
	BYTE Protocol;       	// Printing protocol
	BYTE Port;				// Which port
	BYTE Status;		    // Port status	
	BYTE LoginUser[32];		// Login user
	uint32 StartTime;		// Start time
	uint32 EndTime;			// End time
	uint32 ByteSize;      	// Byte size
}PACK JOB_LOG;

extern uint32	JOBCount[NUM_OF_PRN_PORT];
extern uint32   CurOFTotal[NUM_OF_PRN_PORT];
extern uint32   JobTotCount;
extern JOB_LOG  *JOBLIST[MAX_JOB_LIST];

BYTE JL_PutList(BYTE proto, BYTE port, BYTE *person, uint8 pLen);
void JL_AddSize(BYTE port, uint32 size);
void JL_EndList(BYTE port, BYTE btStatus);
#endif //SUPPORT_JOB_LOG
#endif	/* _JOBLOG_H */

