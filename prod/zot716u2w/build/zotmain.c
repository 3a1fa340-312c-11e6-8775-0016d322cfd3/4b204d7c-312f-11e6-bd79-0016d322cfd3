/*===========================================================================
//        zotmain.c
//	Copy Righti(2006) ZOT.
//=========================================================================*/
#include <stdio.h>
#include <config.h>
#include <cyg/infra/diag.h>

int config_net_mbuf_usage = 200;
int config_net_cluster_usage = 2400;

void zotmain( void )
{
    while(1) {
        diag_printf("this is my test\n");
    };
}

externC void
cyg_user_start( void )
{
    zotmain();
}

void telnet_printf(char* message, int messagelen) {}
//void iplinit(void){}
//void udp_init(){}

