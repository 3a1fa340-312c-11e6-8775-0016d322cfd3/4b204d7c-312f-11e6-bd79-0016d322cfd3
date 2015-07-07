#ifndef _NDSCRYPT_H
#define _NDSCRYPT_H

#include <string.h>

void nwencrypt(const uint16 *cryptbuf, const BYTE *in, BYTE *out);
void nwdecrypt(const uint16 *cryptbuf, const BYTE *in, BYTE *out);
void nwcryptinit(uint16 *scryptbuf, const BYTE *key);
void nwencryptblock(const BYTE *cryptkey, const BYTE *buf, int16 buflen, BYTE *outbuf);
void nwdecryptblock(const BYTE *cryptkey, const BYTE *buf, int16 buflen, BYTE *outbuf);

#define nwhash1init(hash, hashlen) memset(hash, 0, hashlen)
void nwhash1(BYTE *hash, int16 hashlen, const BYTE *data, int16 datalen);

#define nwhash2init(hashbuf) memset(hashbuf, 0, 0x42)
void nwhash2(BYTE *hashbuf, BYTE c);
void nwhash2block(BYTE *hashbuf, const BYTE *data, int16 datalen);
void nwhash2end(BYTE *hashbuf);

#endif  _NDSCRYPT_H
