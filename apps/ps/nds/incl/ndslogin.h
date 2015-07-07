#ifndef _NDSLOGIN_H
#define _NDSLOGIN_H

void NDSu2c_strcpy(BYTE *d, const uni_char *s);
int16 NDSc2u_strcpy(uni_char *d, const char *s);
int16 NDSuni_strlen(const uni_char *str);
DWORD NDSget_dword_lh(BYTE **buf);
int32 NDSLoginAuth(FSInfo *ConnInfo, const BYTE *PrintServerNameconst, const BYTE *ContextName, const BYTE *Password,uint32 *UserID);
int16 NDSRead(BYTE *buf,uint16 bufsize, FSInfo *ConnInfo,uint32 ObjectID,BYTE *ObjectName);
int32 NDSResolveName(FSInfo *fsinfo, uint32 flags, uni_char *entry_name, uint32 *entry_id, BYTE *remote, BYTE *networkaddress);

#endif  _NDSLOGIN_H
