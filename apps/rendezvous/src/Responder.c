/*
 * Copyright (c) 2002-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@

    Change History (most recent first):

$Log: Responder.c,v $
Revision 1.16.2.1  2004/04/07 23:51:09  cheshire
Remove obsolete comments referring to doing mDNS on port 53

Revision 1.16  2003/08/14 02:19:55  cheshire
<rdar://problem/3375491> Split generic ResourceRecord type into two separate types: AuthRecord and CacheRecord

Revision 1.15  2003/08/12 19:56:26  cheshire
Update to APSL 2.0

Revision 1.14  2003/08/06 18:20:51  cheshire
Makefile cleanup

Revision 1.13  2003/07/23 00:00:04  cheshire
Add comments

Revision 1.12  2003/07/15 01:55:16  cheshire
<rdar://problem/3315777> Need to implement service registration with subtypes

Revision 1.11  2003/07/14 18:11:54  cheshire
Fix stricter compiler warnings

Revision 1.10  2003/07/10 20:27:31  cheshire
<rdar://problem/3318717> mDNSResponder Posix version is missing a 'b' in the getopt option string

Revision 1.9  2003/07/02 21:19:59  cheshire
<rdar://problem/3313413> Update copyright notices, etc., in source code comments

Revision 1.8  2003/06/18 05:48:41  cheshire
Fix warnings

Revision 1.7  2003/05/06 00:00:50  cheshire
<rdar://problem/3248914> Rationalize naming of domainname manipulation functions

Revision 1.6  2003/03/08 00:35:56  cheshire
Switched to using new "mDNS_Execute" model (see "mDNSCore/Implementer Notes.txt")

Revision 1.5  2003/02/20 06:48:36  cheshire
Bug #: 3169535 Xserve RAID needs to do interface-specific registrations
Reviewed by: Josh Graessley, Bob Bradley

Revision 1.4  2003/01/28 03:07:46  cheshire
Add extra parameter to mDNS_RenameAndReregisterService(),
and add support for specifying a domain other than dot-local.

Revision 1.3  2002/09/21 20:44:53  zarzycki
Added APSL info

Revision 1.2  2002/09/19 04:20:44  cheshire
Remove high-ascii characters that confuse some systems

Revision 1.1  2002/09/17 06:24:35  cheshire
First checkin

*/

#include "mDNSClientAPI.h"// Defines the interface to the client layer above
#include "mDNSPosix.h"    // Defines the specific types needed to run mDNS on this platform

#include <assert.h>
#include <stdio.h>			// For printf()
#include <stdlib.h>			// For exit() etc.
#include <string.h>			// For strlen() etc.
//Ron #include <unistd.h>			// For select()
#include <errno.h>			// For errno, EINTR
#include <signal.h>
//Ron #include <fcntl.h>

#include <cyg/kernel/kapi.h> //Ron
#include "lwip/linkip.h"

#include "pstarget.h" //Ron
#include "psglobal.h" //Ron
#include "psdefine.h" //Ron
#include "prnport.h" //Ron
#include "eeprom.h" //Ron
extern BYTE  PSMode; //Ron
extern	uint8 PSMode2; //Ron
extern struct parport PortIO[NUM_OF_PRN_PORT]; //Ron
uint8 ether_port_down = 0;
uint8 AssociatedFlag = 0;
#define	MIN(a,b) (((a)<(b))?(a):(b)) //Ron
#define	MAX(a,b) (((a)>(b))?(a):(b)) //Ron

extern uint8    mvRENVServiceName[64];
mDNSservicesList *infList; //Ron
//Rendezvous Thread initiation information
#define RENDEZVOUS_TASK_PRI 	       	20	//ZOT716u2
#define RENDEZVOUS_TASK_STACK_SIZE  	40960
static	uint8			rendezvous_Stack[RENDEZVOUS_TASK_STACK_SIZE];
static  cyg_thread		rendezvous_Task;
cyg_handle_t	rendezvous_TaskHdl;

static void  RENDEV_Reg_UDP(cyg_addrword_t data); //Ron

//ZOTIPS extern struct netif *WLanface;  

typedef struct PosixNetworkInterface PosixNetworkInterface;

struct PosixNetworkInterface
{
	NetworkInterfaceInfo    coreIntf;
	const char *            intfName;
	PosixNetworkInterface * aliasIntf;
	int                     index;
	int                     multicastSocket;
	int                     multicastSocketv6;
};

mStatus RENDEVmDNSDynamicTextRecordAppendCString(char  *ioTxt, char *ioTxtSize,
	 const char *inName, const char *inValue)  //         ,size_t  inValueSize)
{
				char                  oldSize;
				char                  newSize;
				int                             hasName;
				int                             hasValue;

				hasName  = ( inName !=NULL ) && ( *inName != '\0' );
				hasValue = ( inValue != NULL )  && ( * inValue != '\0' );
				if (  hasName==mDNSfalse ||  hasValue==mDNSfalse )   return mDNSfalse;

				oldSize = *ioTxtSize;
				newSize = oldSize +strlen( inName)+strlen(inValue)+sizeof(unsigned char)*2;                          // add length byte size
				assert(newSize<MAX_mDNSservice_TXT);

				 if ( newSize>MAX_mDNSservice_TXT )
					 return mDNSfalse;

				 ioTxt[oldSize]=          strlen( inName)+strlen(inValue)+sizeof(unsigned char);
				 strncpy( ioTxt+oldSize+sizeof(unsigned char), inName,strlen( inName));
				 strncpy(  ioTxt+oldSize+sizeof(unsigned char)+strlen( inName)   ,"=",1);
				 strncpy(  ioTxt+oldSize+sizeof(unsigned char)+strlen( inName) +1    ,inValue,strlen( inValue));
				 *ioTxtSize=newSize;
				 return mDNStrue;
	}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark ***** Globals
#endif

static mDNS mDNSStorage;       // mDNS core uses this to store its globals
static mDNS_PlatformSupport PlatformStorage;  // Stores this platform's globals

static const char *gProgramName = "mDNSResponderPosix";

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark ***** Signals
#endif

static volatile mDNSBool gReceivedSigUsr1;
static volatile mDNSBool gReceivedSigHup;
static volatile mDNSBool gStopNow;

// We support 4 signals.
//
// o SIGUSR1 toggles verbose mode on and off in debug builds
// o SIGHUP  triggers the program to re-read its preferences.
// o SIGINT  causes an orderly shutdown of the program.
// o SIGQUIT causes a somewhat orderly shutdown (direct but dangerous)
// o SIGKILL kills us dead (easy to implement :-)
//
// There are fatal race conditions in our signal handling, but there's not much 
// we can do about them while remaining within the Posix space.  Specifically, 
// if a signal arrives after we test the globals its sets but before we call 
// select, the signal will be dropped.  The user will have to send the signal 
// again.  Unfortunately, Posix does not have a "sigselect" to atomically 
// modify the signal mask and start a select.

static void HandleSigUsr1(int sigraised)
    // If we get a SIGUSR1 we toggle the state of the 
    // verbose mode.
{
    assert(sigraised == SIGUSR1);
    gReceivedSigUsr1 = mDNStrue;
}

static void HandleSigHup(int sigraised)
    // A handler for SIGHUP that causes us to break out of the 
    // main event loop when the user kill 1's us.  This has the 
    // effect of triggered the main loop to deregister the 
    // current services and re-read the preferences.
{
    assert(sigraised == SIGHUP);
	gReceivedSigHup = mDNStrue;
}

static void HandleSigInt(int sigraised)
    // A handler for SIGINT that causes us to break out of the 
    // main event loop when the user types ^C.  This has the 
    // effect of quitting the program.
{
    assert(sigraised == SIGINT);
    
    if (gMDNSPlatformPosixVerboseLevel > 0) {
        fprintf(stderr, "\nSIGINT\n");
    }
    gStopNow = mDNStrue;
}

static void HandleSigQuit(int sigraised)
    // If we get a SIGQUIT the user is desperate and we 
    // just call mDNS_Close directly.  This is definitely 
    // not safe (because it could reenter mDNS), but 
    // we presume that the user has already tried the safe 
    // alternatives.
{
    assert(sigraised == SIGQUIT);

    if (gMDNSPlatformPosixVerboseLevel > 0) {
        fprintf(stderr, "\nSIGQUIT\n");
    }
    mDNS_Close(&mDNSStorage);
    exit(0);
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark ***** Parameter Checking
#endif

static mDNSBool CheckThatRichTextHostNameIsUsable(const char *richTextHostName, mDNSBool printExplanation)
    // Checks that richTextHostName is a reasonable host name 
    // label and, if it isn't and printExplanation is true, prints 
    // an explanation of why not.
{
    mDNSBool    result;
    domainlabel richLabel;
    domainlabel poorLabel;
    
    result = mDNStrue;
    if (result && strlen(richTextHostName) > 63) {
        if (printExplanation) {
//            fprintf(stderr, 
//                    "%s: Host name is too long (must be 63 characters or less)\n", 
//                    gProgramName);
        }
        result = mDNSfalse;
    }
    if (result && richTextHostName[0] == 0) {
        if (printExplanation) {
//            fprintf(stderr, "%s: Host name can't be empty\n", gProgramName);
        }
        result = mDNSfalse;
    }
    if (result) {
        MakeDomainLabelFromLiteralString(&richLabel, richTextHostName);
        ConvertUTF8PstringToRFC1034HostLabel(richLabel.c, &poorLabel);
        if (poorLabel.c[0] == 0) {
            if (printExplanation) {
//                fprintf(stderr, 
//                        "%s: Host name doesn't produce a usable RFC-1034 name\n", 
//                        gProgramName);
            }
            result = mDNSfalse;
        }
    }
    return result;
}

static mDNSBool CheckThatServiceTypeIsUsable(const char *serviceType, mDNSBool printExplanation)
    // Checks that serviceType is a reasonable service type 
    // label and, if it isn't and printExplanation is true, prints 
    // an explanation of why not.
{
    mDNSBool result;
    
    result = mDNStrue;
    if (result && strlen(serviceType) > 63) {
        if (printExplanation) {
//            fprintf(stderr, 
//                    "%s: Service type is too long (must be 63 characters or less)\n", 
//                    gProgramName);
        }
        result = mDNSfalse;
    }
    if (result && serviceType[0] == 0) {
        if (printExplanation) {
//            fprintf(stderr, 
//                    "%s: Service type can't be empty\n", 
//                    gProgramName);
        }
        result = mDNSfalse;
    }
    return result;
}

static mDNSBool CheckThatServiceTextIsUsable(const char *serviceText, mDNSBool printExplanation,
                                             mDNSu8 *pStringList, mDNSu16 *pStringListLen)
    // Checks that serviceText is a reasonable service text record 
    // and, if it isn't and printExplanation is true, prints 
    // an explanation of why not.  Also parse the text into 
    // the packed PString buffer denoted by pStringList and 
    // return the length of that buffer in *pStringListLen.
    // Note that this routine assumes that the buffer is 
    // sizeof(RDataBody) bytes long.
{
    mDNSBool result;
    size_t   serviceTextLen;
    
    // Note that parsing a C string into a PString list always 
    // expands the data by one character, so the following 
    // compare is ">=", not ">".  Here's the logic:
    //
    // #1 For a string with not ^A's, the PString length is one 
    //    greater than the C string length because we add a length 
    //    byte.
    // #2 For every regular (not ^A) character you add to the C 
    //    string, you add a regular character to the PString list.
    //    This does not affect the equivalence stated in #1.
    // #3 For every ^A you add to the C string, you add a length 
    //    byte to the PString list but you also eliminate the ^A, 
    //    which again does not affect the equivalence stated in #1.
    
    result = mDNStrue;
    serviceTextLen = strlen(serviceText);
    if (result && strlen(serviceText) >= sizeof(RDataBody)) {
        if (printExplanation) {
//            fprintf(stderr, 
//                    "%s: Service text record is too long (must be less than %d characters)\n", 
//                    gProgramName,
//                    (int) sizeof(RDataBody) );
        }
        result = mDNSfalse;
    }
    
    // Now break the string up into PStrings delimited by ^A.
    // We know the data will fit so we can ignore buffer overrun concerns. 
    // However, we still have to treat runs long than 255 characters as
    // an error.
    
    if (result) {
        int         lastPStringOffset;
        int         i;
        int         thisPStringLen;
        
        // This algorithm is a little tricky.  We start by copying 
        // the string directly into the output buffer, shifted up by 
        // one byte.  We then fill in the first byte with a ^A. 
        // We then walk backwards through the buffer and, for each 
        // ^A that we find, we replace it with the difference between 
        // its offset and the offset of the last ^A that we found
        // (ie lastPStringOffset).
        
        memcpy(&pStringList[1], serviceText, serviceTextLen);
        pStringList[0] = 1;
        lastPStringOffset = serviceTextLen + 1;
        for (i = serviceTextLen; i >= 0; i--) {
            if ( pStringList[i] == 1 ) {
                thisPStringLen = (lastPStringOffset - i - 1);
                assert(thisPStringLen >= 0);
                if (thisPStringLen > 255) {
                    result = mDNSfalse;
                    if (printExplanation) {
//                        fprintf(stderr, 
//                                "%s: Each component of the service text record must be 255 characters or less\n", 
//                                gProgramName);
                    }
                    break;
                } else {
                    pStringList[i]    = thisPStringLen;
                    lastPStringOffset = i;
                }
            }
        }
        
        *pStringListLen = serviceTextLen + 1;
    }

    return result;
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark ***** Command Line Arguments
#endif

static const char kDefaultPIDFile[]     = "/var/run/mDNSResponder.pid";
static const char kDefaultServiceType[] = "_afpovertcp._tcp.";
static const char kDefaultServiceDomain[] = "local.";
enum {
    kDefaultPortNumber = 548
};

static const char *gRichTextHostName = "";
static const char *gServiceType      = kDefaultServiceType;
static const char *gServiceDomain    = kDefaultServiceDomain;
static mDNSu8      gServiceText[sizeof(RDataBody)];
static mDNSu16     gServiceTextLen   = 0;
static        int  gPortNumber       = kDefaultPortNumber;
static const char *gServiceFile      = "";
static   mDNSBool  gDaemon           = mDNSfalse;
static const char *gPIDFile          = kDefaultPIDFile;

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark ***** Registration
#endif

typedef struct PosixService PosixService;

struct PosixService {
    ServiceRecordSet coreServ;
    PosixService *next;
    int serviceID;
};

static PosixService *gServiceList = NULL;

static void RegistrationCallback(mDNS *const m, ServiceRecordSet *const thisRegistration, mStatus status)
    // mDNS core calls this routine to tell us about the status of 
    // our registration.  The appropriate action to take depends 
    // entirely on the value of status.
{
    switch (status) {

        case mStatus_NoError:      
            debugf("Callback: %##s Name Registered",   thisRegistration->RR_SRV.resrec.name.c); 
            // Do nothing; our name was successfully registered.  We may 
            // get more call backs in the future.
            break;

        case mStatus_NameConflict: 
            debugf("Callback: %##s Name Conflict",     thisRegistration->RR_SRV.resrec.name.c); 

            // In the event of a conflict, this sample RegistrationCallback 
            // just calls mDNS_RenameAndReregisterService to automatically 
            // pick a new unique name for the service. For a device such as a 
            // printer, this may be appropriate.  For a device with a user 
            // interface, and a screen, and a keyboard, the appropriate response 
            // may be to prompt the user and ask them to choose a new name for 
            // the service.
            //
            // Also, what do we do if mDNS_RenameAndReregisterService returns an 
            // error.  Right now I have no place to send that error to.
            
            status = mDNS_RenameAndReregisterService(m, thisRegistration, mDNSNULL);
			
			//Ron
			if  (  memcmp(EEPROM_Data.RENVServiceName,&(thisRegistration->RR_SRV.resrec.name.c[1]), thisRegistration->RR_SRV.resrec.name.c[0]) !=0)
		   {	
				memset(EEPROM_Data.RENVServiceName,0,sizeof(EEPROM_Data.RENVServiceName));
				memcpy(EEPROM_Data.RENVServiceName, &(thisRegistration->RR_SRV.resrec.name.c[1]), thisRegistration->RR_SRV.resrec.name.c[0]);
				
				memset(mvRENVServiceName,0,sizeof(EEPROM_Data.RENVServiceName));
				memcpy(mvRENVServiceName, &(thisRegistration->RR_SRV.resrec.name.c[1]), thisRegistration->RR_SRV.resrec.name.c[0]);
				
				WriteToEEPROM(&EEPROM_Data);
		   }

            assert(status == mStatus_NoError);
            break;

        case mStatus_MemFree:      
            debugf("Callback: %##s Memory Free",       thisRegistration->RR_SRV.resrec.name.c); 
            
            // When debugging is enabled, make sure that thisRegistration 
            // is not on our gServiceList.
            
            #if !defined(NDEBUG)
                {
                    PosixService *cursor;
                    
                    cursor = gServiceList;
                    while (cursor != NULL) {
                        assert(&cursor->coreServ != thisRegistration);
                        cursor = cursor->next;
                    }
                }
            #endif
            free(thisRegistration);
            break;

        default:                   
            debugf("Callback: %##s Unknown Status %d", thisRegistration->RR_SRV.resrec.name.c, status); 
            break;
    }
}

static int gServiceID = 0;

static mStatus RegisterOneService(const char *  richTextHostName, 
                                  const char *  serviceType, 
                                  const char *  serviceDomain, 
                                  const mDNSu8  text[],
                                  mDNSu16       textLen,
                                  long          portNumber)
{
    mStatus             status;
    PosixService *      thisServ;
    mDNSOpaque16        port;
    domainlabel         name;
    domainname          type;
    domainname          domain;
    
    status = mStatus_NoError;
    thisServ = (PosixService *) malloc(sizeof(*thisServ));

	assert(MAX_mDNSservice_TXT>=textLen);

	if (thisServ == NULL)   status = mStatus_NoMemoryErr;
    if (status == mStatus_NoError) {
        MakeDomainLabelFromLiteralString(&name,  richTextHostName);
        MakeDomainNameFromDNSNameString(&type, serviceType);
        MakeDomainNameFromDNSNameString(&domain, serviceDomain);
        port.b[0] = (portNumber >> 8) & 0x0FF;
        port.b[1] = (portNumber >> 0) & 0x0FF;
        status = mDNS_RegisterService(&mDNSStorage, &thisServ->coreServ,
                &name, &type, &domain,				// Name, type, domain
                NULL, port, 						// Host and port
                text, textLen,						// TXT data, length
                NULL, 0,							// Subtypes
                mDNSInterface_Any,					// Interace ID
                RegistrationCallback, thisServ);	// Callback and context
    }
    if (status == mStatus_NoError) {
        thisServ->serviceID = gServiceID;
        gServiceID += 1;

        thisServ->next = gServiceList;
        gServiceList = thisServ;

        if (gMDNSPlatformPosixVerboseLevel > 0) {
//            fprintf(stderr, 
//                    "%s: Registered service %d, name '%s', type '%s', port %ld\n", 
//                    gProgramName, 
//                    thisServ->serviceID, 
//                    richTextHostName,
//                    serviceType,
//                    portNumber);
        }
    } else {
        if (thisServ != NULL) {
            free(thisServ);
        }
    }
    return status;
}



static mStatus RegisterServicesInFile(const char *filePath)
{
    mStatus     status;
    FILE *      fp;
    int         junk;
    mDNSBool    good;
    int         ch;
    char name[256];
    char type[256];
    const char *dom = kDefaultServiceDomain;
    char rawText[1024];
    mDNSu8  text[sizeof(RDataBody)];
    mDNSu16 textLen;
    char port[256];
    
    status = mStatus_NoError;
    fp = fopen(filePath, "r");
    if (fp == NULL) {
        status = mStatus_UnknownErr;
    }
    if (status == mStatus_NoError) {
        good = mDNStrue;
        do {
            // Skip over any blank lines.
            do {
                ch = fgetc(fp);
            } while ( ch == '\n' || ch == '\r' );
            if (ch != EOF) {
                good = (ungetc(ch, fp) == ch);
            }
            
            // Read three lines, check them for validity, and register the service.
            if ( good && ! feof(fp) ) {
                good = ReadALine(name, sizeof(name), fp);               
                if (good) {
                    good = ReadALine(type, sizeof(type), fp);
                }
                if (good) {
                	char *p = type;
                	while (*p && *p != ' ') p++;
                	if (*p) {
                		*p = 0;
                		dom = p+1;
                	}
                }
                if (good) {
                    good = ReadALine(rawText, sizeof(rawText), fp);
                }
                if (good) {
                    good = ReadALine(port, sizeof(port), fp);
                }
                if (good) {
                    good =     CheckThatRichTextHostNameIsUsable(name, mDNSfalse)
                            && CheckThatServiceTypeIsUsable(type, mDNSfalse)
                            && CheckThatServiceTextIsUsable(rawText, mDNSfalse, text, &textLen)
                            && CheckThatPortNumberIsUsable(atol(port), mDNSfalse);
                }
                if (good) {
                    status = RegisterOneService(name, type, dom, text, textLen, atol(port));
                    if (status != mStatus_NoError) {
                        fprintf(stderr, 
                                "%s: Failed to register service, name = %s, type = %s, port = %s\n", 
                                gProgramName,
                                name,
                                type,
                                port);
                        status = mStatus_NoError;       // keep reading
                    }
                }
            }
        } while (good && !feof(fp));

        if ( ! good ) {
            fprintf(stderr, "%s: Error reading service file %s\n", gProgramName, gServiceFile);
        }
    }
    
    if (fp != NULL) {
        junk = fclose(fp);
        assert(junk == 0);
    }
    
    return status;
}

static mStatus RegisterOurServices( mDNSservicesList *const infService)
{
	mStatus status= mStatus_NoError;
	mDNSservicesList *temp_Service=(mDNSservicesList *) infService;

	for( temp_Service=infService ;  temp_Service ;   temp_Service = temp_Service->next){
		assert(strlen(temp_Service-> mDNSservice_servicesName)>0);
		assert (  CheckThatRichTextHostNameIsUsable(temp_Service-> mDNSservice_servicesName, mDNStrue) );
		assert(CheckThatServiceTypeIsUsable(temp_Service-> mDNSservice_serviceType, mDNStrue) ) ;
		assert(temp_Service-> service_port >0);
		assert(strlen(temp_Service->mDNSservice_TXT)>0);
		status=RegisterOneService(temp_Service-> mDNSservice_servicesName,
									 temp_Service-> mDNSservice_serviceType,
									 temp_Service-> mDNSservice_domain,
									 temp_Service-> mDNSservice_TXT,
									 temp_Service->mDNSservice_TXT_len,
									 temp_Service-> service_port);
	}

	return status;
}

void DeregisterOurServices(void)
{
    PosixService *thisServ;
    int thisServID;
    
    while (gServiceList != NULL) {
        thisServ = gServiceList;
        gServiceList = thisServ->next;

        thisServID = thisServ->serviceID;
        
        mDNS_DeregisterService(&mDNSStorage, &thisServ->coreServ);

        if (gMDNSPlatformPosixVerboseLevel > 0) {
//            fprintf(stderr, 
//                    "%s: Deregistered service %d\n",
//                    gProgramName, 
//                    thisServ->serviceID);
        }
    }
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark **** Main
#endif

#ifdef NOT_HAVE_DAEMON

    // The version of Solaris that I tested on didn't have the daemon 
    // call.  This implementation was basically stolen from the 
    // Mac OS X standard C library.



#endif /* NOT_HAVE_DAEMON */



#define  RENDEV_LPR_priority             30
#define  RENDEV_IPP_priority              40
#define  RENDEV_TCPRAW_priority   50
mDNSu8 rawText[256];
void Rendezous_txt(mDNSservicesList *infService){
	mDNSservicesList *tmp_infList;
	mStatus  status;
	int port_add_rendev=1;

	tmp_infList=infList;

	PSMode=    EEPROM_Data.PrintServerMode;
	memset(&mDNSStorage,0,sizeof(mDNSStorage));
	status = mDNS_Init(&mDNSStorage, &PlatformStorage,mDNS_Init_NoCache, mDNS_Init_ZeroCacheSize,
					 mDNS_Init_AdvertiseLocalAddresses, mDNS_Init_NoInitCallback, mDNS_Init_NoInitCallbackContext);
	assert(status == mStatus_NoError );
	assert( strlen( EEPROM_Data.RENVServiceName )>0 ) ;

////////  start to set TXT record data
	memcpy( tmp_infList->mDNSservice_servicesName ,EEPROM_Data.RENVServiceName,strlen( EEPROM_Data.RENVServiceName ) );

	strcpy(infList->mDNSservice_domain,"local.");
//		if (strlen(EEPROM_Data.SnmpSysLocation)>0)
//			RENDEVmDNSDynamicTextRecordAppendCString(  infList->mDNSservice_TXT,   &(infList->mDNSservice_TXT_len) , "note",EEPROM_Data.SnmpSysLocation );
//		status=RENDEVmDNSDynamicTextRecordAppendCString(  infList->mDNSservice_TXT,   &(infList->mDNSservice_TXT_len) , "txtvers","1" );

	if  (PSMode & PS_UNIX_MODE) {     //LPR
		tmp_infList->next=malloc(sizeof( mDNSservicesList));
		assert(tmp_infList->next!=NULL);
		memcpy(tmp_infList->next,infList,sizeof( mDNSservicesList));
		tmp_infList=tmp_infList->next;
		tmp_infList->next=NULL;
		
		strcpy(tmp_infList->mDNSservice_serviceType,"_printer._tcp") ;
		tmp_infList->service_port=515;
		
		sprintf(  rawText  ,"%d",NUM_OF_PRN_PORT);
		status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "qtota", rawText  );

		 for ( port_add_rendev=0; port_add_rendev<NUM_OF_PRN_PORT; port_add_rendev++){
			 sprintf(rawText,"lp%d",port_add_rendev+1);
			 status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "rp", rawText  );
			 sprintf(rawText,"%d",RENDEV_LPR_priority +port_add_rendev);
			 status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,  "priority", rawText  );

			sprintf(rawText,"Printer Server Port:%d",port_add_rendev+1);
			status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "ty", rawText );

			if (  PortIO[port_add_rendev].Manufacture  != 0 ){
				sprintf(rawText,"(%s)",PortIO[port_add_rendev].Manufacture);
				status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "usb_MFG", rawText );
			}

			if ( PortIO[port_add_rendev].Model !=0 ){
				sprintf(rawText,"(%s)",PortIO[port_add_rendev].Model);
				status=RENDEVmDNSDynamicTextRecordAppendCString(  tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "product",rawText );
				status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "usb_MDL", rawText );
			}
		}
//          sprintf(rawText,"http://%s.local./",_BoxName);
			sprintf(rawText,"http://%d.%d.%d.%d/",EEPROM_Data.BoxIPAddress[0],EEPROM_Data.BoxIPAddress[1],EEPROM_Data.BoxIPAddress[2],EEPROM_Data.BoxIPAddress[3]);
			status=RENDEVmDNSDynamicTextRecordAppendCString(  tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "adminurl", rawText );
	}  // END LPR


	if ( PSMode & PS_IPP_MODE) {       //IPP
		tmp_infList->next=malloc(sizeof( mDNSservicesList));
		assert(tmp_infList->next!=NULL);
		memcpy(tmp_infList->next,infList,sizeof( mDNSservicesList));
		tmp_infList=tmp_infList->next;
		tmp_infList->next=0;
		strcpy(tmp_infList->mDNSservice_serviceType	,"_ipp._tcp") ;
		tmp_infList->service_port=631;
		sprintf(rawText,"%d",NUM_OF_PRN_PORT);
		status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "qtota", rawText  );

		for (port_add_rendev=0;port_add_rendev<NUM_OF_PRN_PORT;port_add_rendev++){
			sprintf(rawText,"Printer Server Port:%d",port_add_rendev+1);
			status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "ty", rawText );
			sprintf(rawText,"%d",RENDEV_IPP_priority +port_add_rendev);
			status=RENDEVmDNSDynamicTextRecordAppendCString(tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,  "priority", rawText );
			sprintf(rawText,"lp%d",port_add_rendev+1);
			status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "rp", rawText  );

			if (  PortIO[port_add_rendev].Manufacture  != 0 ){
				sprintf(rawText,"(%s)",PortIO[port_add_rendev].Manufacture);
				status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "usb_MFG", rawText );
			}

			if ( PortIO[port_add_rendev].Model !=0 ){
				sprintf(rawText,"(%s)",PortIO[port_add_rendev].Model);
				status=RENDEVmDNSDynamicTextRecordAppendCString(  tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "product",rawText );
				status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "usb_MDL", rawText );
			}
		}
//      sprintf(rawText,"http://%s.local./",_BoxName);
		sprintf(rawText,"http://%d.%d.%d.%d/",EEPROM_Data.BoxIPAddress[0],EEPROM_Data.BoxIPAddress[1],EEPROM_Data.BoxIPAddress[2],EEPROM_Data.BoxIPAddress[3]);
		status=RENDEVmDNSDynamicTextRecordAppendCString(  tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "adminurl", rawText );
	}

	if(PSMode2 & PS_RAWTCP_MODE) {       // RAW TCP
		tmp_infList->next=malloc(sizeof( mDNSservicesList));
		assert(tmp_infList->next!=NULL);
		memcpy(tmp_infList->next,infList,sizeof( mDNSservicesList));
		tmp_infList=tmp_infList->next;
		tmp_infList->next=0;

		strcpy(tmp_infList->mDNSservice_serviceType,"_pdl-datastream._tcp") ;
		tmp_infList->service_port=9100;

		sprintf(rawText,"%d",RENDEV_TCPRAW_priority);
		status=RENDEVmDNSDynamicTextRecordAppendCString(  tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,  "priority", rawText  );
//		status=RENDEVmDNSDynamicTextRecordAppendCString( tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,  "pdl", "application/postscript,application/vnd.hp-PCL"  );

//      sprintf(rawText,"http://%s.local./",_BoxName);
		sprintf(rawText,"http://%d.%d.%d.%d/",EEPROM_Data.BoxIPAddress[0],EEPROM_Data.BoxIPAddress[1],EEPROM_Data.BoxIPAddress[2],EEPROM_Data.BoxIPAddress[3]);
		status=RENDEVmDNSDynamicTextRecordAppendCString(  tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "adminurl", rawText );
	 } //END RAW-TCP


	tmp_infList=infList;
	strcpy(tmp_infList->mDNSservice_serviceType,"_http._tcp") ;
	status=RENDEVmDNSDynamicTextRecordAppendCString(  tmp_infList->mDNSservice_TXT,   &(tmp_infList->mDNSservice_TXT_len) ,   "path", "/index.htm" );
	tmp_infList->service_port=80;
}

int  Rendezous_Reload(void){
		mStatus  status;
		DeregisterOurServices();
		
		SendResponses(&mDNSStorage);
		
//		mDNSPlatformPosixRefreshInterfaceList(&mDNSStorage);
		ClearInterfaceList(&mDNSStorage);
//		status = mDNS_Init(&mDNSStorage, &PlatformStorage, mDNS_Init_NoCache, mDNS_Init_ZeroCacheSize,
//						 mDNS_Init_AdvertiseLocalAddresses, mDNS_Init_NoInitCallback, mDNS_Init_NoInitCallbackContext);
		infList=(mDNSservicesList *) malloc(sizeof( mDNSservicesList));
		memset(infList,0,sizeof(mDNSservicesList));
		Rendezous_txt(infList);
		RegisterOurServices(infList);
		free_service_list();
}

/* Ron temp */
int8 EtherLinkReady(void){
	return (!ether_port_down);	//Ron
}

int8 WlanLinkReady(void){
    #if defined(NDWP2020)
    AssociatedFlag = wlan_get_linkup();
    #endif
	return AssociatedFlag;
}

extern cyg_sem_t rendezvous_sem;
extern int Need_Rendezous_Reload ;
#ifdef ARCH_ARM
extern int Network_TCPIP_ON;
#endif

/* Rendezvous main */
//int Rendezous_init(void){
void Rendezous_init(cyg_addrword_t data){

#ifdef ARCH_ARM
    while (Network_TCPIP_ON == 0)
        ppause (100);
#endif /* ARCH_ARM */
	cyg_semaphore_wait(&rendezvous_sem);
		
	infList=(mDNSservicesList *) malloc(sizeof( mDNSservicesList));
	memset(infList,0,sizeof(mDNSservicesList));

	Rendezous_txt(infList);
	RegisterOurServices(infList);
	free_service_list();

			//err = mDNS_Update( &mDNSStorage, rr, 0, (mDNSu16) inSize, newRData, DNSRegistrationUpdateCallBack );
			
	//Create LpdRecv Thread
	cyg_thread_create(RENDEZVOUS_TASK_PRI,
						RENDEV_Reg_UDP,
						&mDNSStorage,
						"RENDEV_Reg_UDP",
						(void *) (rendezvous_Stack),
						RENDEZVOUS_TASK_STACK_SIZE,
						&rendezvous_TaskHdl,
						&rendezvous_Task);

	//Start LpdRecv Thread
	cyg_thread_resume(rendezvous_TaskHdl);
				
//	newproc("RENDEV_Reg_UDP",8762,RENDEV_Reg_UDP,0,&mDNSStorage,NULL,0);
//		return 0;
}


int   free_service_list(void )
{
	mDNSservicesList  *p,*q;
	for (p = infList ; p != NULL; p =q) {
		q=p->next;
		free(p);
		}
	return 0;
}

static void  RENDEV_Reg_UDP(cyg_addrword_t data){
	mDNS *const m = (mDNS *const)data;
	PosixNetworkInterface *info;
//Ron	mDNSs32 ticks;
	mDNSs32 nextevent;
	int probe_flag = 1;
	int eth_probe_flag = 1, wlan_probe_flag = 1;
	struct mbuf *bp;
//Ron		struct arp arpStruct;
//	int32 LinkLocalIP;
//Ron	struct timeval interval;
//Ron	struct timeval timeout;
	BYTE *inline_printer_name[NUM_OF_PRN_PORT];
	int i;
//Ron	timeout.tv_sec=0x3FFFFFFF;
//Ron	timeout.tv_usec=0;
	
	assert(m != NULL);

	for(i=0;i<NUM_OF_PRN_PORT;i++){
		inline_printer_name[i]=malloc(256);
		memset(inline_printer_name[i],0,256);
		if (PortIO[i].Model!=0 )
			strcpy(inline_printer_name[i],PortIO[i].Model);
	}

	for(;;){
		nextevent = mDNS_Execute(m);
	 	info = (PosixNetworkInterface *)(m->HostInterfaces);
		
		cyg_thread_yield();
		
	 	while (info) {
			SocketDataReady(m, info, info->multicastSocket);
			info = (PosixNetworkInterface *)(info->coreIntf.next);
		}

//Ron		ticks = nextevent - mDNSPlatformTimeNow();
//Ron		if (ticks < 1) ticks = 1;
//Ron		interval.tv_sec  = ticks >> 10;                            // The high 22 bits are seconds
//Ron		interval.tv_usec = ((ticks & 0x3FF) * 15625) / 16;      // The low 10 bits are 1024ths
		
		// 4. If client's proposed timeout is more than what we want, then reduce it
//Ron		if (timeout.tv_sec > interval.tv_sec 
//Ron				||(timeout.tv_sec == interval.tv_sec && timeout.tv_usec > interval.tv_usec))
//Ron			timeout = interval;

		if( Need_Rendezous_Reload )
		{
//			DeregisterOurServices();
			Rendezous_Reload();
			Need_Rendezous_Reload = 0;
		}

		if ( ( EtherLinkReady() == 0) && ( WlanLinkReady() == 0) )	
			mDNSCoreMachineSleep(m, mDNStrue);
				
/*		
		if( chang_ip_flag == 1 )
		{
			cyg_semaphore_init(&rendezvous_sem, 0);
			cyg_semaphore_wait(&rendezvous_sem);
			chang_ip_flag = 0;
			Rendezous_Reload();
		}
		
		if( ( EtherLinkReady() && eth_probe_flag == 0 ) ||
		    ( WlanLinkReady() && wlan_probe_flag == 0 ) )
		{
			if ( (LinkLocal_get_current_state() == IDLE))
			{
				erase_netif_ipaddr();
				LinkLocal_set_state( PROBE );
				cyg_semaphore_post(&linklocal_sem);	
				
				cyg_semaphore_init(&rendezvous_sem, 0);
				cyg_semaphore_wait(&rendezvous_sem);
				
			}
			Rendezous_Reload();
			mDNSCoreMachineSleep(m, mDNSfalse);
			eth_probe_flag = EtherLinkReady();
			wlan_probe_flag = WlanLinkReady();
		}
*/		
		
		if( ( EtherLinkReady() && eth_probe_flag == 0 ) ||
		    ( WlanLinkReady() && wlan_probe_flag == 0 ) )
		{
			mDNSCoreMachineSleep(m, mDNSfalse);
			eth_probe_flag = EtherLinkReady();
			wlan_probe_flag = WlanLinkReady();
		}
		
		if( eth_probe_flag )
			eth_probe_flag = EtherLinkReady();
		
		if( wlan_probe_flag )
			wlan_probe_flag = WlanLinkReady();
		
		for(i=0;i<NUM_OF_PRN_PORT;i++){
			if ( (inline_printer_name[i][0]!=0 || PortIO[i].Model!=0 )
				&&memcmp(inline_printer_name[i],PortIO[i].Model, MAX(strlen(PortIO[i].Model), strlen(inline_printer_name[i] ))))
			{
			   	memset(inline_printer_name[i],0,256);
				if (PortIO[i].Model )
					strcpy(inline_printer_name[i],PortIO[i].Model);
					
			 	Rendezous_Reload();
   	       	}
		} //for prn_port
		
	} //main loop
}

