// This module offers functions prototype all subroutines need on the telserv.c
// add --- by arius 3/20/2000

#include "stdio.h"

#define MAXCounterForKeyinPassword  3

#define FILE 			ZOT_FILE

void telnetstart(cyg_addrword_t data);
int  telnetstop(int argc, char *argv[], void *p);
int  CheckNameRights(FILE *network);
void TServerMainFunction(cyg_addrword_t data);
