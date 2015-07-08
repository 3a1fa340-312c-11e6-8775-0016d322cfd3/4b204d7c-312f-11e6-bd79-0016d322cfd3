/* Internet LPD Client definitions
 *   written by David Johnson (dave@cs.olemiss.edu)
 *
 * This code is in the public domain.
 *
 * Revision History:
 *
 * Revision 1.0  91/09/04  dave
 * Initial Release
 *
 */
#ifndef	_LP_H
#define	_LP_H

/* LPD service mode */
#define LPD_RECEIVE_MODE  0x01
#define LPD_SERVICE_MODE  0x02
#define LPD_ERROR_MODE    0xFF

/* LPD commands */
#define	START_CMD	1
#define	RECEIVE_CMD	2
#define	SQUEUE_CMD	3
#define	QUEUE_CMD	4
#define	REMOVE_CMD	5

/* LPD subcommands */
#define ABORT_SCMD	1
#define RECEIVE_CF_SCMD	2
#define RECEIVE_DF_SCMD	3

/* LPD return status */
#define STATUS_OK	0
#define STATUS_ERR	1

/* Status responses. Correspond to above return values */
#define RESPOND_OK(s)	send(s,"\0",1,0)
//#define RESPOND_ERR(s)	send(s,"\1",1,0)
#define RESPOND_ERR(s)	send(s,"\xFF",1,0)	//5/25/98 for SCO_UNIX

#endif	/* _LP_H */
