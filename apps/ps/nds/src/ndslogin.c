#include "pstarget.h"
#include "psglobal.h"
#include "eeprom.h"
#include "nds.h"
#include "ndscrypt.h"
#include "nwcrypt.h"
#include "mpilib.h"
#include "ndsext.h"
#include "ndsmain.h"
#include "ndslogin.h"
#include "ndsqueue.h"

static void kwait(int a){  cyg_thread_yield();}	//615wu //must be, could not use 'yeild' to replace

#ifdef NDS_PS
#ifdef CONST_DATA
extern const BYTE far buf2str1[8];
extern const BYTE far buf2str2[16];
extern const BYTE far buf2str3[8];
#else
BYTE buf2str1[8] = {1,0,0,0,9,0,2,0};
BYTE buf2str2[16] = {65,0,0,0,1,0,0,0,1,0,9,0,53,0,28,0};
BYTE buf2str3[8] = {1,0,0,0,1,0,6,0};
#endif

//unicode to char copy
void NDSu2c_strcpy(BYTE *d, const uni_char *s)
{
	while( ( *d++ = (BYTE)*s++) != 0);
}

//char to unicode copy
int16 NDSc2u_strcpy(uni_char *d, const char *s)
{
	int16 count = 0;

	while((*d++ = *s++) != 0) count++;

	return count;
}


void NDSput_dword_lh(BYTE **buf,DWORD value)
{
	NSET32((*buf), value);
	*buf += 4;
}

void NDSput_word_lh2(BYTE **buf,WORD value)
{
	NSET16((*buf), value);
	*buf += 2;
}

void NDSput_buf(BYTE **buf,const BYTE *databuf, int16 buflen)
{
	memcpy(*buf, databuf, buflen);
	(*buf) += buflen;
	while (buflen++ & 3) *(*buf)++ = 0;
}

int16 NDSuni_strlen(const uni_char *str)
{
	int16 i = 0;
	while( NGET16(str++) ) i++;
	return i;
}

void NDSput_uni_string(BYTE **buf,const uni_char *str)
{
	int i = (NDSuni_strlen(str)+1) * 2;

	NDSput_dword_lh(buf,i);
	memcpy(*buf,str,i);
	(*buf) += i;
	while(i++ & 3) *(*buf)++ = 0;
}

DWORD NDSget_dword_lh(BYTE **buf)
{
	DWORD value;

	value = NGET32(*buf);
	*buf += 4;

	return value;
}

static WORD NDSget_word_lh(BYTE **buf)
{
	WORD value;

	value = NGET16(*buf);
	*buf += 4;

	return value;
}

static WORD NDSget_word_lh2(BYTE **buf)
{
	WORD value;

	value = NGET16(*buf);
	*buf += 2;

	return value;
}


static void NDSget_buf(BYTE **buf, BYTE *outbuf, int16 bufsize)
{
	if(outbuf) memcpy(outbuf, *buf, bufsize);
	*buf += (bufsize + 3) & (~3);
}


void fillrandom(BYTE *buf, int16 buflen)
{
	while (buflen--) {
		*(buf++) = urandom(256);
	}
}

BYTE keyprefix[] = {1, 0, 0, 0, 3, 0, 1, 0};

// 1: OK, 0 : ERROR
int initkey(const BYTE *key, BYTE **keyptr, int16 *keylen)
{
	if (!memcmp(key, keyprefix, 8)) {
		*keylen = NGET16(key+8);
		*keyptr = (BYTE*) key + 10;
		return 1;
	}
	return 0;
}

int16 findchunk(
const BYTE *keyptr, int16 keylen,
const BYTE *chunk,  BYTE **chunkptr)
{
	const BYTE *p;

	if ((p = keyptr) != NULL) {
		while (p - keyptr < keylen) {
			if ((p[0] != chunk[0]) || (p[1] != chunk[1]))
				p += 4 + p[2] + p[3];
			else {
				if (chunkptr) (*chunkptr) = p + 4;
				return (p[2] + p[3]);
			}
		}
	}
	if (chunkptr) *chunkptr = NULL;
	return 0;
}

// 0 : wrong key, != 0  : key ok
int checkkey(const BYTE *key)
{
	BYTE temp[8];
	BYTE *keyptr, *p;
	int16 keylen;

	if ( initkey(key, &keyptr, &keylen) &&
	     findchunk(keyptr, keylen, "MA", &p)
	){
		nwhash1init(temp,8);
		nwhash1(temp, 8, key + 10, NGET16(key+8) - 20);
		return (!memcmp(p, temp, 8));
	} else
		return 0;
}

int16 countbits_l(BYTE *buf, int16 bufsize)
{
	BYTE b;

	while ((--bufsize) && (!buf[bufsize]));
	b = buf[bufsize];
	bufsize <<= 3;
	while (b) {
		b >>= 2;
		bufsize++;
	}
	return bufsize;
}

void copyfill(void *outbuf, int16 outsize, const void *inbuf, int16 insize)
{
	if (outsize < insize) insize = outsize;
	memcpy(outbuf, inbuf, insize);
	memset((BYTE*)outbuf + insize, 0, outsize - insize);
}

int32 modexpkey(const BYTE *s_key, BYTE *buf, BYTE *outbuf, int16 bufsize)
{
	BYTE *s_keyptr;
	int16 s_keylen, i, nbits, nblocksize;
	unitptr nmod, nexp, nin, nout;
    BYTE *p;
	int32 rc = -1;

	nmod = nexp = nin = nout = NULL;

	if(!initkey(s_key, &s_keyptr, &s_keylen)) return NDS_INVALID_RESPONSE;
	i = findchunk(s_keyptr, s_keylen, "NN", &p);
	if (!p) return NDS_INVALID_RESPONSE;
	nbits = countbits_l(p, i);
	nblocksize = ((nbits + 31) & (~31)) >> 3;
	if (!(nmod = malloc(nblocksize))) return NDS_NOMEM;

	copyfill(nmod, nblocksize, p, i);

	i = findchunk(s_keyptr, s_keylen, "EN", &p);
	if (!p) {
		rc = NDS_INVALID_RESPONSE;
	    goto err_modex_exit;
	}

	rc = NDS_NOMEM;

	if (!(nexp = malloc(nblocksize))) goto err_modex_exit;
	copyfill(nexp, nblocksize, p, i);

	if (!(nin = malloc(nblocksize))) goto err_modex_exit;
	copyfill(nin, nblocksize, buf, bufsize);

    if (!(nout = malloc(nblocksize))) goto err_modex_exit;
	set_precision(bytes2units(nblocksize));

	if (mp_modexp((unitptr) nout, (unitptr) nin,
	    (unitptr) nexp,(unitptr) nmod)) rc = NDS_INVALID_RESPONSE;
	else {
		copyfill(outbuf, bufsize, nout, nblocksize);
		rc = OKAY;
	}

err_modex_exit:
	if (nout) { mp_init0(nout); free(nout); }
	if (nin) { mp_init0(nin); free(nin); }
	if (nexp) free(nexp);
	if (nmod) free(nmod);
	return rc;
}

WORD NDSNCPRequest(FSInfo *FSInfoPointer,WORD NCPSize)
{
	// Setup ECB Block
	SendPsECBInit(NDSPsSocket,FSInfoPointer->PCBPhysicalID,NCPSize);

	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;    // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup SAP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x00;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;

	return (NCPRequest(NDSPsSocket,FSInfoPointer));
}

int32 NDSencrypt(
BYTE *data,
int16 datalen,
BYTE **outp, BYTE *pend,
BYTE *PublicKey)
{
	BYTE rand[28];
	BYTE hashrand[8], temp[8];
	uint16 cryptbuf[128];
	BYTE buf2[56];
	int16 i;
	char *p;
	int32 rc;

	if ((*outp + datalen + 108) > pend) return -1;

	fillrandom(rand, 28);
	nwhash1init(hashrand, 8);
	for (i = 10; i; i--) nwhash1(hashrand, 8, rand, 28);

	memset(buf2 + 40, 0, 16);
	buf2[0] = 11;
	memcpy(buf2 + 1, rand, 28);
	memset(buf2 + 29, 11, 11);
    nwhash1(buf2 + 40, 5, buf2 + 1, 39);
	nwhash1(buf2 + 45, 2, buf2, 45);
	fillrandom(buf2 + 47, 5);

	if( (rc = modexpkey(PublicKey, buf2, buf2, 56)) != OKAY) return rc;

	NDSput_dword_lh(outp, datalen + 108);		            //4
	NDSput_buf(outp, buf2str1, sizeof(buf2str1));           //8
	NDSput_dword_lh(outp, datalen + 96);                    //4
	NDSput_buf(outp, buf2str2, sizeof(buf2str2));          //16
    NDSput_buf(outp, buf2, 56);					           //56
    NDSput_dword_lh(outp, datalen + 20);			        //4
    NDSput_buf(outp, buf2str3, sizeof(buf2str3));           //8
    NDSput_dword_lh(outp, (datalen + 8) | (datalen << 16)); //4

	memset(temp, 3, 3);
	nwhash1init(temp + 3, 5);
	nwhash1(temp + 3, 5, data, datalen);
	nwhash1(temp + 3, 5, temp, 3);
	nwencryptblock(hashrand, data, datalen, *outp);
    *outp += datalen;
	for (i = 0, p = *outp - 8; i < 8; i++, p++)  temp[i] ^= *p;
	nwcryptinit(cryptbuf, hashrand);
	nwencrypt(cryptbuf, temp, *outp);
	*outp += 8;

//	memzero(rand);
//	memzero(hashrand);
//	memzero(temp);
//	memzero(cryptbuf);
//	memzero(buf2);

	return OKAY;
}


int32
NDSNCPSendFrag(
FSInfo *fsinfo,
uint32 verb,
uint16 inbuflen,
BYTE *outbuf, int16 outbufsize, int16 *outbuflen)
{
	BYTE    result;
	uint16  sizeleft, i;
//	uint16	maxdatasize = 1400;
	uint16	maxdatasize = 522;
	uint16  NCPSize;
	BYTE    first = 1;
	BYTE    firstReply = 1;
	int32   fraghnd = -1;
	int16	replyLen = 0;
	int16	fragLen;
	BYTE    *inbuf = SendNDSData->NDSReqData;
	int32	ndsReplyCode = -399, rc = OKAY;

	if (outbuflen) *outbuflen = 0;
	do
	{
		sizeleft = maxdatasize;
    	NCPSize = 6;
		SendNDSData->FunCode = 104;
		SendNDSData->SubFunCode = 2;

		SendNDSData->FragHand = fraghnd;
		NCPSize += 6;

		if (first)
		{
			SendNDSData->MaxFragSize  = maxdatasize - 8;
			SendNDSData->MessageSize  = inbuflen + 12;
			SendNDSData->FragFlag     = 0;
			SendNDSData->VerbNumber   = verb;
			SendNDSData->ReplyBufSize = outbufsize;
			NCPSize  += 20;
			sizeleft -= 25;
		}
		else
			sizeleft -= 5;

		i = (sizeleft > inbuflen) ? inbuflen : sizeleft;

		if (!first) memcpy((BYTE*)&SendNDSData->MaxFragSize,inbuf,i);
		else first = 0;

		inbuflen -= i;
		inbuf += i;
		NCPSize += i;

		if ((result = NDSNCPRequest(fsinfo,NCPSize)) != OKAY)
		{
#ifdef PC_OUTPUT
			printf("Error in ncp_request\n");
#endif PC_OUTPUT
			rc = (NDS_REQUESTER_ERROR | (BYTE)result);
			goto SendFragErrExit;
		}

		if( (fragLen = NGET32(ReceiveNCPSubData)) < 4) {
#ifdef PC_OUTPUT
			printf("Fragment too short\n");
#endif PC_OUTPUT
			rc = NDS_NCP_PACKET_LENGTH;
			goto SendFragErrExit;
		}

		fraghnd = NGET32(ReceiveNCPSubData+4);
		fragLen -= 4;
		if (fragLen) {
			int hdr;

			if (firstReply) {
				ndsReplyCode = NGET32(ReceiveNCPSubData+8);
				hdr = 12;
				fragLen -= 4;
				firstReply = 0;
			} else {
				hdr = 8;
			}
			if (fragLen > outbufsize) {
#ifdef PC_OUTPUT
				printf("Fragment too large, len=%d, max=%d\n", fragLen, outbufsize);
#endif PC_OUTPUT
				rc = NDS_BUFFER_OVERFLOW;
				goto SendFragErrExit;
			}

			if (outbuf) {
				memcpy(outbuf, (ReceiveNCPSubData+hdr), fragLen);
				outbuf += fragLen;
			}
			replyLen += fragLen;
		} else {
			// if reply len == 0 then we must have something to transmit
			// otherwise it can cause endless loop
			if ((fraghnd != -1) && (inbuflen == 0)) {
#ifdef PC_OUTPUT
				printf("Why next fragment?\n");
#endif PC_OUTPUT
				rc = NDS_SERVER_FAILURE;
				goto SendFragErrExit;
			}
		}
		if (fraghnd != -1) {
#ifdef PC_OUTPUT
			printf("Fragmented\n");
#endif PC_OUTPUT
		}
	} while (fraghnd != -1);

	if (inbuflen || firstReply) {
#ifdef PC_OUTPUT
		printf("InBufLen after request=%d, FirstReply=%d\n", inbuflen, firstReply);
#endif PC_OUTPUT
		rc = NDS_SERVER_FAILURE;
		goto SendFragErrExit;
	}
	if (outbuflen) *outbuflen = replyLen;
	if (ndsReplyCode != 0) {
#ifdef PC_OUTPUT
		printf("NDS error %d\n", ndsReplyCode);
#endif PC_OUTPUT
		if ((ndsReplyCode < 0) && (ndsReplyCode > -256))
			rc = (-ndsReplyCode | NDS_SERVER_ERROR);
		else
			rc = ndsReplyCode;
	}

SendFragErrExit:

	return rc;
}

int32
NDSResolveName(FSInfo *fsinfo,
	uint32 flags,
	uni_char *entry_name,
	uint32 *entry_id,
	BYTE *remote,
	BYTE *networkaddress
)
{
	BYTE *buf;
	BYTE *p;
    int32 i;
    int16 len;
	int32 err;

	if (!(buf = malloc(2048))) return NDS_NOMEM;

	BEGIN_NETWARE_CRITICAL(INTO_NDS_RESOLVE);

	p = SendNDSData->NDSReqData;
	NDSput_dword_lh(&p,0);                 //Version
	NDSput_dword_lh(&p,flags);             //Flags
	NDSput_dword_lh(&p,0);                 //Scope of Referral=any server
	NDSput_uni_string(&p,entry_name);      //Entry Name
	NDSput_dword_lh(&p,1);                 //Transport Length
	NDSput_dword_lh(&p,0);                 //Transport value
	NDSput_dword_lh(&p,1);                 //Tree Walker Type
	NDSput_dword_lh(&p,0);                 //Value

	if ((err = NDSNCPSendFrag(fsinfo, 1, p - SendNDSData->NDSReqData, buf, 2048,&len)) == OKAY) {
		p = buf;
		if(len < 32) err = NDS_NCP_PACKET_LENGTH;
		else if ( ((i = NDSget_dword_lh(&p)) < 1) || (i > 2)) {
					err = NDS_NCP_PACKET_LENGTH;
		}
		else {
			*entry_id = NDSget_dword_lh(&p);
			if (i == 1) { //local
				if (remote) *remote = 0;
			}
			else { //remote
				if (remote) *remote = 1;

				// FIXME! Scan all available transports !!!
				if( NDSget_dword_lh(&p) != 0 ||
					NDSget_dword_lh(&p) == 0 ||
					NDSget_dword_lh(&p) != 0) {
					err = NDS_NCP_PACKET_LENGTH;
				}
				else if (NDSget_dword_lh(&p) != 12) {
					err = NDS_NCP_PACKET_LENGTH;
				}
				else {
					if(networkaddress) memcpy(networkaddress,p,12);
					err = OKAY;
				}
			}
		}
    }
	free(buf);

	END_NETWARE_CRITICAL();
	return err;
}

int32 NDSGetServerName(FSInfo *ConnInfo, uni_char **ServerName)
{
    int16 outlen;
    uint32 namelen;
    BYTE *p,*outbuf;
    int32 rc;

	if (!(outbuf = malloc(2048))) return NDS_NOMEM;

	BEGIN_NETWARE_CRITICAL(INTO_NDS_GET_SERVER_NAME);

	//NDS Get Server Address
	if( (rc = NDSNCPSendFrag(ConnInfo, 53, NULL, outbuf, 2048, &outlen)) == 0)
	{
		p = outbuf;
		namelen = NDSget_dword_lh(&p);
		if(namelen+4 > outlen)
			rc = NDS_NCP_PACKET_LENGTH;
		else {
			if(!( (*ServerName)=malloc(namelen)))
				rc = NDS_NOMEM;
			else
				memcpy(*ServerName, p, namelen);
		}
	}
	free(outbuf);

	END_NETWARE_CRITICAL();
	return rc;
}

//size of buf must >= 1024
// return code = buf read length
int16 NDSRead(BYTE *buf,uint16 bufsize, FSInfo *ConnInfo,uint32 ObjectID,BYTE *ObjectName)
{
	int16 outlen;
	BYTE *p;
	int32 rc;

	BEGIN_NETWARE_CRITICAL(INTO_NDS_READ);

	p = SendNDSData->NDSReqData;
	NDSput_dword_lh(&p, 0);          //Version
	NDSput_dword_lh(&p, -1L); //Interaction Handle
	NDSput_dword_lh(&p, ObjectID);	 //Entry ID
	NDSput_dword_lh(&p, 1);          //Info Type = Attribute Name & Value
	NDSput_dword_lh(&p, 0);			 //All attribute = FALSE
	NDSput_dword_lh(&p, 1);          //Name Entries = 1

	NDSc2u_strcpy((uni_char*) buf,ObjectName);
	NDSput_uni_string(&p, (uni_char *) buf); //Attribute Name

	//NDS Read
	rc = NDSNCPSendFrag(ConnInfo, 3, p - SendNDSData->NDSReqData, buf, bufsize, &outlen);

	END_NETWARE_CRITICAL();

	if(rc == OKAY) return outlen; // > 0
	if(rc == NDS_NO_SUCH_OBJECT) return rc;	 // < 0
	if(rc == NDS_BUFFER_OVERFLOW) return rc; // < 0

	return (0); // <= 0
}

int32 NDSGetPublicKey(BYTE *buf,FSInfo *ConnInfo,uint32 ObjectID,BYTE **pKey)
{
	int16 KeyLen;
	int16 outlen;
	int32 n1, n2, n3, n5;
	uint32 type, skiplen;
	int16 ofs, klen;
	BYTE *kptr;
	BYTE *p;
	int32 rc;

	if( (outlen = NDSRead(buf, 1024, ConnInfo,ObjectID, "Public Key")) <= 0)
		return NDS_INVALID_RESPONSE;

	p = buf;
	n1 = NDSget_dword_lh(&p);      //Interaction Handle (-1)
	n2 = NDSget_dword_lh(&p);      //Entry Info  (1)
	n3 = NDSget_dword_lh(&p);      //Name of attributes (1)
	type = NDSget_dword_lh(&p);	   //Syntax ID =  Octet String (9)
	skiplen = NDSget_dword_lh(&p); //Length of Attribute Name
	p += (skiplen+3) & (~3);       //Skip attribute name
	n5 = NDSget_dword_lh(&p);      //Entries (1)
	KeyLen = NDSget_dword_lh(&p);   //Key Length

	if(n1 != -1 || n2 != 1 || n3 != 1 || type != 9 ||
	   n5 != 1  || KeyLen+skiplen+28 > outlen) {
			rc = NDS_NCP_PACKET_LENGTH;
	} else {
		memcpy(buf,p,KeyLen);
		rc = OKAY;
	}

	if(rc != OKAY) return rc;

	ofs = NGET16(buf+10) + 0x1a;
	if ((ofs > KeyLen) || (!initkey(buf + ofs, &kptr, &klen)) ||
	    (klen + ofs > KeyLen) || (!checkkey(buf + ofs)))
	{
		return NDS_INVALID_RESPONSE;
	}

	if (!(kptr = malloc(klen + 10))) {
		return NDS_NOMEM;
	}
	memcpy(kptr, buf + ofs, klen +  10);
	*pKey = kptr;

	return OKAY;
}

BYTE bufstr[16]={28, 0, 0, 0, 1, 0, 0, 0, 1, 0, 6, 0, 16, 0, 4, 0};


int32 NDSLogin(FSInfo *ConnInfo, uint32 UserID, const BYTE *Password,
uint32 ServerID, BYTE *LoginData, BYTE **PrivateKey)
{
	BYTE  *p,*pend;
	BYTE  *buf, *randbuf, *tempbuf;
	int16  replylen;
	BYTE   loginid[4];
	BYTE   temp[16];
	BYTE   hashshuf[8];
	int16  i;
	BYTE   crypt1strc[28];
	BYTE   *PublicKey;
	BYTE   grace_period = 0;
	int32  n1, n2;
	uint16 n2a, n3;
	BYTE   randno[4];
	int32  rc;

	if (!(buf = malloc(2048+1024))) return NDS_NOMEM;
	randbuf = buf + 2048;

	BEGIN_NETWARE_CRITICAL(INTO_NDS_LOGIN_1); //////////////////////

	p = SendNDSData->NDSReqData;
	NDSput_dword_lh(&p,0);               //Version
	NDSput_dword_lh(&p,UserID);          //UserID

	//NDS Req Begin Login
	if ((rc = NDSNCPSendFrag(ConnInfo, 57, p - SendNDSData->NDSReqData,
							 buf, 2048,&replylen)) != OKAY) {
		END_NETWARE_CRITICAL();	////////////////////
		goto err_login_exit;
	}

	END_NETWARE_CRITICAL();	////////////////////////

	if(replylen < 8) {
		rc = NDS_NCP_PACKET_LENGTH;
	    goto err_login_exit;
	}

	p = buf;
	NSET32(temp,NDSget_dword_lh(&p));    //pseudoID
	NSET32(loginid,NDSget_dword_lh(&p));	//Login ID

	//Read Public key
	if ((rc = NDSGetPublicKey(buf,ConnInfo, ServerID,&PublicKey)) != OKAY) {
		goto err_login_exit;
	}

	strcpy(randbuf, Password);

	for (p = randbuf; *p; p++) *p = toupper(*p);

	//      pseudoID, password, passwordLen, Target
	shuffle(temp, randbuf, strlen(randbuf), temp);
	memset(hashshuf,0,8);

	for (i = 10; i; i--) nwhash1(hashshuf, 8, temp, 16);

	memcpy(temp, loginid, 4);
	memset(temp + 4, 7, 7);
	nwhash1init(temp + 11, 5);
	nwhash1(temp + 11, 5, temp, 11);
	memcpy(crypt1strc, bufstr + 4, 12);
	nwencryptblock(hashshuf, temp, 16, crypt1strc + 12);

	fillrandom(randno, 4);
	fillrandom(randbuf, 1024);

	kwait(NULL);

	BEGIN_NETWARE_CRITICAL(INTO_NDS_LOGIN_2); //////////////////////

	p = SendNDSData->NDSReqData;
	NDSput_buf(&p, randno, 4);               //4
	NDSput_dword_lh(&p, 1024);               //4
	NDSput_buf(&p, randbuf, 1024);        //1024
	NDSput_buf(&p, bufstr, sizeof(bufstr));	//16
	NDSput_buf(&p, crypt1strc + 12, 16);	//16 ==> total : 1064 bytes

	pend = (p = buf)+ 2048;	//size = 2048
	NDSput_dword_lh(&p, 2);
	NDSput_dword_lh(&p, 0);
	NDSput_dword_lh(&p, UserID);

	NDSencrypt(SendNDSData->NDSReqData, 1064, &p, pend,PublicKey);
	free(PublicKey);

#ifdef PC_OUTPUT
	while(pend -p > 1400) printf("NDSlogin memory size too short !\n");
#endif PC_OUTPUT

	memcpy(SendNDSData->NDSReqData,buf,p-buf);

	//NDS Req Finish Login
	if ((rc = NDSNCPSendFrag(ConnInfo, 58, p-buf,
							 buf, 2048,&replylen)) != OKAY) {
		if (rc != NDS_PASSWORD_EXPIRED) {
			END_NETWARE_CRITICAL();	//////////////////
			goto err_login_exit;
		}
		grace_period = 1;
	}

	END_NETWARE_CRITICAL();	//////////////////

	kwait(NULL);

	rc = NDS_INVALID_RESPONSE;
	pend = (p = buf) + replylen;

	NDSget_buf(&p, LoginData, 8);
	n1 = NDSget_dword_lh(&p);
	if(n1 > (pend - p)) goto err_login_exit;

	pend = p + n1;
	n1 = NDSget_dword_lh(&p);
	n2 = NDSget_dword_lh(&p);
	n3 = NDSget_word_lh(&p);
	if((n1 != 1) || (n2 != 0x060001) || (n3 > pend - p)) goto err_login_exit;

    nwhash1init(temp, 8);
    for (i = 10; i; i--)  nwhash1(temp, 8, crypt1strc, 28);
	nwdecryptblock(temp, p, n3, p);
    nwhash1init(temp, 5);
	nwhash1(temp, 5, p, n3 - 5);
	if (memcmp(temp, p + n3 - 5, 5))  goto err_login_exit;
	pend = p + n3 - 12;
	NDSget_buf(&p, loginid, 4);
	n2 = NDSget_dword_lh(&p);
	if((memcmp(loginid, randno, 4)) || (n2 > pend - p)) goto err_login_exit;
	for (i = 0; i < n2; i++) p[i] ^= randbuf[i];

    pend = p + n2;
    n1 = NDSget_dword_lh(&p);
	n2 = NDSget_dword_lh(&p);
	n3 = NDSget_word_lh(&p);
	if((n1 != 1) || (n2 != 0x060001) || (n3 > pend - p)) goto err_login_exit;

	pend = p + n3;
	nwdecryptblock(hashshuf, p, n3, p);
	n1 = NDSget_dword_lh(&p);
	n2a = NDSget_word_lh2(&p);
	n3 =  NDSget_word_lh2(&p);
	if( (n1 != 1) || (n2a != 2) || (n3 > pend - p)) goto err_login_exit;
	if (!(tempbuf = malloc(n3 + 10))) {
			rc = NDS_NOMEM;
			goto err_login_exit;
	}
	memset(tempbuf, 0, 8);
	tempbuf[0] = 1;
	tempbuf[4] = 3;
	tempbuf[6] = 1;
	NSET16((tempbuf+8), n3);
	memcpy(tempbuf + 10, p, n3);
	if(!checkkey(tempbuf)){
		free(tempbuf);
		goto err_login_exit;
	}
	*PrivateKey = tempbuf;

	rc = grace_period ? NDS_PASSWORD_EXPIRED : 0;

err_login_exit:

//  memzero(hashshuf);
//  memzero(randbuf);
//  memzero(crypt1strc);
//  memzero(randno);
//  memzero(temp);
	free(buf);

	return rc;
}

int32 NDSBeginAuth(
FSInfo *ConnInfo,
uint32 UserID,
uint32 ServerID,
BYTE *AuthID)
{
	BYTE  *buf,*p, *pend, *n_temp, temp[8];
    BYTE   randno[4];
    int16  replylen;
    int32  outlen, n1, n2, n3, n4;
	BYTE  *PublicKey;
    uint16 n3a;
    int32  rc;

	if (!(buf = malloc(2048))) return NDS_NOMEM;
	n_temp = NULL;
	fillrandom(randno, 4);

	BEGIN_NETWARE_CRITICAL(INTO_NDS_BEGIN_AUTH); //////////////////

	p = SendNDSData->NDSReqData;
	NDSput_dword_lh(&p, 0);		 //version
	NDSput_dword_lh(&p, UserID); //user ID
	NDSput_buf(&p, randno, 4);

	//Begin Authentication
	if ((rc = NDSNCPSendFrag(ConnInfo, 59, 12, buf, 1024,&replylen)) != OKAY)
	{
		END_NETWARE_CRITICAL();	////////////////////
		goto err_begauth_exit;
	}

	END_NETWARE_CRITICAL();	////////////////////

	rc = NDS_INVALID_RESPONSE;
	pend = (p = buf) + replylen;
	NDSget_buf(&p, AuthID, 4);
	outlen = NDSget_dword_lh(&p);
    if (outlen > pend - p) goto err_begauth_exit;

	pend = p + outlen;
	n1 = NDSget_dword_lh(&p);
	n2 = NDSget_dword_lh(&p);
	n3 = NDSget_dword_lh(&p);
	if( (n1 != 1) || (n2 != 0x020009) || (n3 > pend - p))
		goto err_begauth_exit;

    pend = p + n3;
	n1 = NDSget_dword_lh(&p);
    n1 = NDSget_dword_lh(&p);
    n2 = NDSget_dword_lh(&p);
    n3a = NDSget_word_lh(&p);
	if((n1 != 1) || (n2 != 0x0a0001) || (n3a > pend - p))
		goto err_begauth_exit;

	n1 = ((countbits_l(p, n3a) + 31) & (~31)) >> 3;
	if (n1 < 52) goto err_begauth_exit;

	if (!(n_temp = malloc(n1))) {
		rc = NDS_NOMEM;
		goto err_begauth_exit;
	}
	copyfill(n_temp, n1, p, n3a);
	p += (n3a + 3) & (~3);

	if ((rc = NDSGetPublicKey(buf+1024,ConnInfo, ServerID,&PublicKey)) != OKAY)
	{
		goto err_begauth_exit;
	}

	rc = modexpkey(PublicKey, n_temp, n_temp, n1);
	free(PublicKey);
	if (rc) goto err_begauth_exit;

	rc = NDS_INVALID_RESPONSE;
	nwhash1init(temp, 7);
	nwhash1(temp + 5, 2, n_temp, 45);
	nwhash1(temp, 5, n_temp + 1, 39);
	if (memcmp(temp, n_temp + 40, 7))  goto err_begauth_exit;
	nwhash1init(temp, 8);
    for (n1 = 10; n1; n1--) nwhash1(temp, 8, n_temp + 1, 28);
    free(n_temp);
    n_temp = NULL;

	n1 = NDSget_dword_lh(&p);
	n2 = NDSget_dword_lh(&p);
	n3 = NDSget_dword_lh(&p);
	n4 = NDSget_dword_lh(&p);
	if( (n1 != 28) || (n2 != 1) || (n3 != 0x060001) || (n4 != 0x040010) ||
	    (pend - p < 16))
		goto err_begauth_exit;

	kwait(NULL);

	nwdecryptblock(temp, p, 16, p);
	nwhash1init(temp, 5);
	nwhash1(temp, 5, p, 11);
	if ((!memcmp(temp, p + 11, 5)) || (!memcmp(p, randno, 4))) rc = OKAY;
err_begauth_exit:
	if (n_temp) free(n_temp);
	free(buf);
	return rc;
}

int32 NDSReadEntryName(
	FSInfo *ConnInfo,
	uint32 ObjectID,
	uni_char **ObjectName,
	int16 *NameLen)
{
	BYTE *p, *buf;
	int16 replylen;
	int16 tmp;
	uni_char *p2;
	int32 rc;

	BEGIN_NETWARE_CRITICAL(INTO_NDS_READ_ENTRY_NAME); //////////

	if(ObjectName) *ObjectName = NULL;
	if (NameLen) *NameLen = 0;

	p = SendNDSData->NDSReqData;
	NDSput_dword_lh(&p,2);	      //Version = 2
	NDSput_dword_lh(&p,0);        //Request Flags = 0
	NDSput_dword_lh(&p,0x281d);	  //Information flags = 0x281D
	NDSput_dword_lh(&p, ObjectID);//Object ID
	if (!(buf = malloc(2048))) {
		rc = NDS_NOMEM;
		goto err_readentryinfo_exit;
	}

	// Read Entry Info (for
	if( (rc = NDSNCPSendFrag(ConnInfo, 2, 16, buf, 2048, &replylen)) != OKAY)
	{
		goto err_readentryinfo_exit;
	}
	p = buf;
	p += 16;//skip output flags, entry flags, subordinate, modifiaction time.
	tmp = (int16) NDSget_dword_lh(&p); //Base class length
	NDSget_buf(&p,NULL,tmp);	   //skip base class value
	*NameLen = (int16) NDSget_dword_lh(&p); //Entry Name length

	if(replylen < (16+tmp+*NameLen)) {
		rc = NDS_INVALID_RESPONSE;
		goto err_readentryinfo_exit;
    }

	if (!( (*ObjectName) = malloc(*NameLen))) {
		rc = NDS_NOMEM;
		goto err_readentryinfo_exit;
	}
	memcpy(*ObjectName, p, *NameLen);
	rc = OKAY;

err_readentryinfo_exit:

	END_NETWARE_CRITICAL(); ////////////

	free(buf);
	return rc;
}

BYTE *allocfillchunk(
const BYTE *keyptr, int16 keylen,
const BYTE *chunk, int16 destsize)
{
	BYTE *p, *p2;
	int16 i;

	i = findchunk(keyptr, keylen, chunk, &p);
	if (!p) return NULL;
	if (!(p2 = malloc(destsize))) return NULL;
	copyfill(p2, destsize, p, i);

	return p2;
}

int32 NDSGenAuthData(
BYTE **outp,
const BYTE *PublicKey, const BYTE *PrivateKey,
const BYTE *AuthID, BYTE *LoginStr, int16 LoginStrLen)
{
    BYTE *keyptr;
    int16 keylen, i, j;
    int16 nbits, nblocksize, nbytes;
    BYTE  nmask;
    unitptr n_temp, n_mod, n_exp, n_pn, n_qn, n_dp, n_dq, n_cr, n_key;
    unitptr n_key_dp, n_key_dq;
    unitptr up, up2;
	BYTE *p, *tempbuf;
	BYTE *randbuf = NULL;
	BYTE hashbuf[0x42];
	int32 rc;

	n_temp = n_mod = n_exp = n_pn = n_qn = n_dp = n_dq = n_cr = n_key =
	n_key_dp = n_key_dq = NULL;

	if (!initkey(PublicKey, &keyptr, &keylen)) return NDS_INVALID_RESPONSE;
	i = findchunk(keyptr, keylen, "NN", &p);
	if (!p) return NDS_INVALID_RESPONSE;
    nbits = countbits_l(p, i);
    nbytes = (nbits + 7) >> 3;
    nmask = (BYTE)(255 >> (8 - (nbits & 7)));
	nblocksize = ((nbits + 31) & (~31)) >> 3;
	set_precision(bytes2units(nblocksize));

	n_mod = (unitptr)allocfillchunk(keyptr, keylen, "NN", nblocksize);
	n_exp = (unitptr)allocfillchunk(keyptr, keylen, "EN", nblocksize);
	if (!initkey(PrivateKey, &keyptr, &keylen)) {
        rc = NDS_INVALID_RESPONSE;
		goto err_genauthdata_exit;
	}

	n_pn = (unitptr)allocfillchunk(keyptr, keylen, "PN", nblocksize);
	n_qn = (unitptr)allocfillchunk(keyptr, keylen, "QN", nblocksize);
	n_dp = (unitptr)allocfillchunk(keyptr, keylen, "DP", nblocksize);
	n_dq = (unitptr)allocfillchunk(keyptr, keylen, "DQ", nblocksize);
	n_cr = (unitptr)allocfillchunk(keyptr, keylen, "CR", nblocksize);
	n_key = malloc(nblocksize);
	if (n_key == NULL){
		rc = NDS_NOMEM;
		goto err_genauthdata_exit;
	}	

	nwhash2init(hashbuf);
	nwhash2block(hashbuf, LoginStr, LoginStrLen);
	nwhash2end(hashbuf);
	copyfill(n_key, nblocksize, hashbuf, 16);

	if (!(tempbuf = malloc(LoginStrLen + 16))) {
		rc = NDS_NOMEM;
		goto err_genauthdata_exit;
	}
	memset(tempbuf, 0, 16);
	tempbuf[4] = 0x3c;
	memcpy(tempbuf + 8, AuthID, 4);
	p = tempbuf + 12;
	NDSput_dword_lh(&p,LoginStrLen);
	memcpy(p, LoginStr, LoginStrLen);

	nwhash2init(hashbuf);
	nwhash2block(hashbuf, tempbuf, LoginStrLen + 16);
	free(tempbuf);

	n_temp = malloc(nblocksize);
	if (n_temp == NULL){
		rc = NDS_NOMEM;
		goto err_genauthdata_exit;
	}		
	
	n_key_dp = malloc(nblocksize);
	if (n_key_dp == NULL){
		rc = NDS_NOMEM;
		goto err_genauthdata_exit;
	}
			
	n_key_dq = malloc(nblocksize);
	if (n_key_dq == NULL){
		rc = NDS_NOMEM;
		goto err_genauthdata_exit;
	}	
	mp_mult(n_temp, n_pn, n_qn);
	mp_modexp(n_key_dp, n_key, n_dp, n_pn);
	mp_modexp(n_key_dq, n_key, n_dq, n_qn);
	mp_move(n_temp, n_key_dp);
	mp_add(n_temp, n_pn);
	mp_sub(n_temp, n_key_dq);
    stage_modulus(n_pn);
	mp_modmult(n_temp, n_temp, n_cr);
	mp_mult(n_key, n_temp, n_qn);
	mp_add(n_key, n_key_dq);

	randbuf = malloc(nblocksize * 3);
	if (randbuf == NULL){
		rc = NDS_NOMEM;
		goto err_genauthdata_exit;
	}	
	
	memset(randbuf, 0, nblocksize * 3);

	NDSput_dword_lh(outp,12 + nblocksize * 6);
	NDSput_dword_lh(outp, 1);
	NDSput_dword_lh(outp, 0x100008);
	NDSput_word_lh2(outp, 3);
	NDSput_word_lh2(outp, nblocksize * 3);
	memset(*outp, 0, nblocksize * 6);

	up = (unitptr)randbuf;
	up2 = (unitptr)*outp;
	for (i = 3; i; i--) {
		fillrandom((BYTE *)up, nbytes);
		((BYTE*)up)[nbytes - 1] &= nmask;
		if (!(j = mp_compare(up, n_mod))) {
			mp_dec(up);
		} else if (j > 0) {
			mp_sub(up, n_mod);
			mp_neg(up);
			mp_add(up, n_mod);
		}
		mp_modexp(up2, up, n_exp, n_mod);
		up = ((BYTE *)up + nblocksize);
		up2 = ((BYTE *)up2 + nblocksize);
	}
	nwhash2block(hashbuf, *outp, nblocksize * 3);
	nwhash2end(hashbuf);

	up = (unitptr)randbuf;
	for (i = 0; i < 3; i++) {
		mp_init(n_temp, NGET16((hashbuf+ (i<<1) )));
		mp_modexp(up2, n_key, n_temp, n_mod);
		stage_modulus(n_mod);
		mp_modmult(up2, up2, up);
		up = ((BYTE *)up + nblocksize);
		up2 = ((BYTE *)up2 + nblocksize);
	}
	*outp = (BYTE *)up2;
	rc = 0;
err_genauthdata_exit:
//	memzero(hashbuf);
	free(randbuf);
	if (n_key)  { free(n_key);} //12/23/99 Simon added
	if (n_temp) { mp_init0(n_temp); free(n_temp); }
	if (n_key_dp) { mp_init0(n_key_dp); free(n_key_dp); }
	if (n_key_dq) { mp_init0(n_key_dq); free(n_key_dq); }
	if (n_pn) { mp_init0(n_pn); free(n_pn); }
	if (n_qn) { mp_init0(n_qn); free(n_qn); }
	if (n_dp) { mp_init0(n_dp); free(n_dp); }
	if (n_dq) { mp_init0(n_dq); free(n_dq); }
	if (n_cr) { mp_init0(n_cr); free(n_cr); }
	free(n_mod);
	free(n_exp);
	return rc;
}


WORD NDSNCPChangeConnState(FSInfo *ConnInfo, int16 new_state)
{
	WORD rc;

	BEGIN_NETWARE_CRITICAL(INTO_NDS_CHANGE_STATE); //////////

	*(SendNCPSubData + 0) = 0x17;        // Function Code = 23
	*(SendNCPSubData + 1) = 0x00;        // SubFunction Length = 4
	*(SendNCPSubData + 2) = 0x04;
	*(SendNCPSubData + 3) = 0x1D;        // SubFunction Code = 29
	NSET32((SendNCPSubData + 4), new_state);

	rc = NDSNCPRequest(ConnInfo,(6+8));

	END_NETWARE_CRITICAL();	////////////

	return rc;
}

int32 NDSAuthenticate(
FSInfo *ConnInfo,
uint32 UserID,
uint32 ServerID,
const BYTE *LoginData,
const BYTE *PrivateKey)
{
	BYTE AuthID[4];
	int16 UserNameLen;
	uni_char *UserName = NULL;
	int16 LoginStrLen;
	BYTE *LoginStr;
	BYTE *PublicKey;
    BYTE *buf,*p;
    int32 rc;

	PublicKey = LoginStr = buf = NULL;

	//NDS Begin Authentication
	if ((rc = NDSBeginAuth(ConnInfo, UserID, ServerID, AuthID)) != OKAY)
	    return rc;

	//Get User Context Name
	if ((rc = NDSReadEntryName(ConnInfo, UserID, &UserName, &UserNameLen)) !=OKAY)
		return rc;

	LoginStrLen = UserNameLen + 22;

	if (!(LoginStr = malloc(LoginStrLen))) {
		rc = NDS_NOMEM;
		goto err_auth_exit;
    }

	memset(LoginStr, 0, 22);
    LoginStr[0] = 1;
    LoginStr[4] = 6;
	memcpy(LoginStr + 6, LoginData, 8);
	fillrandom(LoginStr + 14, 4);
	NSET16((LoginStr+20), UserNameLen);
	memcpy(LoginStr + 22, UserName, UserNameLen);
	free(UserName);
	UserName = NULL;

	if (!(buf = malloc(1500))) {
		rc = NDS_NOMEM;
		goto err_auth_exit;
    }

	if((rc = NDSGetPublicKey(buf,ConnInfo,UserID,&PublicKey)) != OKAY) {
		goto err_auth_exit;
	}

	p = buf;
	NDSput_dword_lh(&p, 0);	   //Version = 0
	NDSput_dword_lh(&p, 0);
	NDSput_dword_lh(&p,LoginStrLen);
	NDSput_buf(&p, LoginStr, LoginStrLen);

	if ((rc= NDSGenAuthData(&p,PublicKey,PrivateKey,AuthID,
	         LoginStr, LoginStrLen)) !=OKAY)
	{
		goto err_auth_exit;
	}

	BEGIN_NETWARE_CRITICAL(INTO_NDS_AUTH); //////////////

#ifdef PC_OUTPUT
	while((p - buf) > 1024 ) printf("GenAuthData too large (%d)\n",p-buf);
#endif PC_OUTPUT

	memcpy(SendNDSData->NDSReqData,buf, p - buf);


	//NDS Finish Authentrication
	if( (rc=NDSNCPSendFrag(ConnInfo, 60, p - buf, NULL, 0, NULL)) != OKAY) {
		END_NETWARE_CRITICAL(); ////////////
		goto err_auth_exit;
	}

	END_NETWARE_CRITICAL();	////////////////

	rc = NDSNCPChangeConnState(ConnInfo, 1);

err_auth_exit:
	if (buf) free(buf);
	if (LoginStr) free(LoginStr);
	if (PublicKey) free(PublicKey);
	if (UserName) free(UserName);

	return rc;
}

int32 NDSLoginAuth(
FSInfo *ConnInfo,
const BYTE *PrintServerName,
const BYTE *ContextName,
const BYTE *Password,
uint32 *UserID)	   //will return UserID if success
{
	static BYTE *PrivateKey = NULL;	//2/22/99 for replica read-only
	uint32 rc;
	int16 pos;
	uni_char uniPrintServerName[NDS_CONTEXT_LEN+55];
	uint32 ServerID;
	BYTE NetworkAddress[12];
	BYTE NoWriteable; //1: Current server has not a writable replica
	uni_char *uniFSServerName = NULL;
	BYTE LoginData[8];
	BYTE grace_period = 0;
	BYTE retry;

#ifdef PC_OUTPUT
	while(strlen(PrintServerName) >= sizeof(uniPrintServerName)/sizeof(uni_char)) {
		printf("(NDS) PrintServerName too long !\n");
	}
#endif PC_OUTPUT

	pos  = NDSc2u_strcpy(uniPrintServerName,PrintServerName);
	pos += NDSc2u_strcpy(uniPrintServerName+pos,".");
	NDSc2u_strcpy(uniPrintServerName+pos,ContextName);

	if(PrivateKey == NULL) {  //2/22/99 added Save PrivateKey for all FS
		retry = 0;
		while(retry++ < 2) {
			//Resolve print Server name ==> Get login user entry ID
			rc = NDSResolveName(ConnInfo, 0x64, uniPrintServerName,
		                    UserID, &NoWriteable,NetworkAddress);

			if(rc != OKAY){
#ifdef PC_OUTPUT
				printf("Error %d finding user\n", rc);
#endif PC_OUTPUT
				goto ErrExit;
			}

			if(NoWriteable) {
				*UserID = 0;
				DisConnection (NDSPsSocket, ConnInfo);
				memset(ConnInfo,0,sizeof(FSInfo));
				memcpy(ConnInfo->PCBNetworkNumber,NetworkAddress,12);
				//Attach to master FS
				if((rc = NDSAttachToFS(ConnInfo))!=OKAY)
					goto ErrExit;
				continue;
			}

			//Get file server context name
			if((rc = NDSGetServerName(ConnInfo, &uniFSServerName)) != OKAY)
				goto ErrExit;

			//Resolve file server name ==>  get FS Entry ID
			if((rc = NDSResolveName(ConnInfo, 0x62, uniFSServerName ,
	    		&ServerID,NULL, NULL)) != OKAY)
				goto ErrExit;

			//Login
			if((rc = NDSLogin(ConnInfo, *UserID, Password,
			                  ServerID, LoginData,&PrivateKey)) != OKAY)
			{
				if (rc != NDS_PASSWORD_EXPIRED) {

					if (PrivateKey) free(PrivateKey);
					PrivateKey = NULL;
#ifdef PC_OUTPUT
					printf("error %d logging in\n", rc);
#endif PC_OUTPUT
					goto ErrExit;
				}
				grace_period = 1;
			}
			break;
		}
	}

	//Get file server context name
	if((rc = NDSGetServerName(ConnInfo, &uniFSServerName)) != OKAY)
		goto ErrExit;

	//Resolve file server name ==>  get FS Entry ID
	if((rc = NDSResolveName(ConnInfo, 0x62, uniFSServerName ,
	   	&ServerID,NULL, NULL)) != OKAY)
		goto ErrExit;

	if((rc = NDSResolveName(ConnInfo, 0x51, uniPrintServerName,
	    UserID, NULL, NULL)) !=OKAY) goto ErrExit;

	if((rc = NDSAuthenticate(ConnInfo,*UserID,ServerID,LoginData,PrivateKey)) != OKAY)
	{
#ifdef PC_OUTPUT
		printf("(NDS) Error %d authenticating\n", rc);
#endif
		goto ErrExit;
	}

	if (grace_period && (rc==OKAY)) {
		rc = NDS_PASSWORD_EXPIRED;
	}

ErrExit:
	if(rc != OKAY) DisConnection(NDSPsSocket,ConnInfo);
//	if (PrivateKey) free(PrivateKey);
	free(uniFSServerName);
	return rc;
}

#endif NDS_PS
