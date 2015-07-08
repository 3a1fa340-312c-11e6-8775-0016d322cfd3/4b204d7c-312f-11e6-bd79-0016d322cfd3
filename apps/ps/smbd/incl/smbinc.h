#ifndef _SMBINC_H
#define _SMBINC_H

#define MAX_NETBIOS_NAMES					2
#define MAX_SUBNETS							5

extern int  ClientNMB;
extern int  ClientDGRAM;

extern char _MyServerName[NCBNAMSZ];
extern char _MyWorkgroup[NCBNAMSZ];
extern char _MyNetBIOSName[NCBNAMSZ];
extern char _MyScopeName[64];
extern char  my_netbios_names[MAX_NETBIOS_NAMES][NCBNAMSZ];

extern struct in_addr lan_ip;
extern struct in_addr ipzero;
extern struct in_addr allones_ip;
extern struct in_addr loopback_ip;

extern struct subnet_record _subnetlist[MAX_SUBNETS];
extern struct subnet_record *unicast_subnet;
extern struct subnet_record *remote_broadcast_subnet;
extern struct subnet_record *wins_server_subnet;

extern char Smbprinterserver[3][SERVICENAME_LENGTH];

// utilsock.c
extern struct in_addr lastip;
extern int lastport;

#define GMAX_TTL				(60*60*12)	/* 12 hours default. */

#endif _SMBINC_H
