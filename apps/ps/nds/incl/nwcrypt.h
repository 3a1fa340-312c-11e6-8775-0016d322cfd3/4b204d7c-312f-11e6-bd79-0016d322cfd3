#ifndef _NWCRYPT_H
#define _NWCRYPT_H

void shuffle(const unsigned char *lon, const unsigned char *buf, int buflen, unsigned char *target);
void nw_encrypt(const unsigned char *fra, const unsigned char *buf, unsigned char *til);

#endif  _NWCRYPT_H
