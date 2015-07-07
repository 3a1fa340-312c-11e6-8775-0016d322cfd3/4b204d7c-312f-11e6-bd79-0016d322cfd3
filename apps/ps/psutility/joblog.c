#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "joblog.h"

#ifdef SUPPORT_JOB_LOG
uint32			JOBCount[NUM_OF_PRN_PORT] = {0},CurOFTotal[NUM_OF_PRN_PORT] = {0}, JobTotCount = 0;
JOB_LOG *JOBLIST[MAX_JOB_LIST] = {NULL};
JOB_LOG *job_log = NULL;


BYTE JL_PutList(BYTE proto, BYTE port, BYTE *person, uint8 pLen)
{		
	if(JOBLIST[JobTotCount % MAX_JOB_LIST] == NULL)
		JOBLIST[JobTotCount % MAX_JOB_LIST] = malloc(sizeof(JOB_LOG));
	
	job_log = JOBLIST[JobTotCount % MAX_JOB_LIST];
	memset(job_log, 0, sizeof(JOB_LOG));
	
	JobTotCount += 1;
	
	if( JobTotCount == 0)
		JobTotCount = 1;
	
	CurOFTotal[port] = JobTotCount;
	JOBCount[port] += 1;
	
	job_log->JobID = JobTotCount;
	job_log->Protocol = proto;	// UnixUsed
	job_log->Port = (BYTE)port +1 ;
	job_log->Status = 1;	// PORT_PRINTING;
	memcpy(&job_log->LoginUser, person, pLen);
	job_log->StartTime = msclock();
	job_log-> EndTime = 0;
	
	return 0;
}

void JL_AddSize(BYTE port, uint32 size)
{
	job_log = JOBLIST[(CurOFTotal[port]-1) % MAX_JOB_LIST];
	
	if (job_log == NULL)
		return;
	job_log->ByteSize += size;
}

void JL_EndList(BYTE port, BYTE btStatus)
{
	job_log = JOBLIST[(CurOFTotal[port]-1) % MAX_JOB_LIST];
	
	if (job_log == NULL)
		return;

	// George Add January 26, 2007
	// 0x00: PORT_READY		0x01: PORT_PRINTING		0x02: Abort printing	0x03: Timeout
	job_log->Status = btStatus;

	job_log->EndTime = msclock();
}

#endif //SUPPORT_JOB_LOG