#ifndef _PAPRINT_H
#define _PAPRINT_H

#define AT_COMM_NONE	  (0)
#define	AT_COMM_TBCP	  (1)
#define AT_COMM_BCP		  (2)

void ascpaprint(int port, struct Mypapfile *in, int flags);
void paprint(int port, struct Mypapfile *papf, int flags);
#endif  _PAPRINT_H
