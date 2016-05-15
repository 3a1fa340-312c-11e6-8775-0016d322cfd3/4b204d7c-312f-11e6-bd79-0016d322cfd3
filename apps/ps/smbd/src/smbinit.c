extern int smbmain();
extern int nb_init();

#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "smb.h"

//for pipes_struct must include
#include "rpc_misc.h"
#include "rpc_dce.h"
#include "ntdomain.h"

#include "nameserv.h"

#include "btorder.h"

#include "dlinlist.h"

#include "rpcwksvc.h"

#include "rpc_srv.h"

//for struct REG_Q_INFO must include
#include "rpc_sec.h"
#include "rpc_reg.h"

#ifdef ARCH_ARM
extern int Network_TCPIP_ON;
#endif /* ARCH_ARM */

// void SMBInit()
void SMBInit(cyg_addrword_t data)
{
#ifdef ARCH_ARM
    while (Network_TCPIP_ON == 0)
        ppause (100);
#endif /* ARCH_ARM */

    nb_init();
    smbmain();
}
