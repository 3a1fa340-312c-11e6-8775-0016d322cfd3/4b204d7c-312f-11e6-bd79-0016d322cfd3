#ifndef _BANNER_H
#define _BANNER_H
char *print_large(char *Buf, char *Name, char IsEndText); //5/14/99 changed
#ifdef CONST_DATA
extern const char far BannerData[];
#else
extern const char BannerData[];
#endif
#endif  _BANNER_H
