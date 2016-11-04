/* This module is responsible for Link Locak IP Task. It will dependent with
   ARP module and DHCP module.
   
   When we can not get IP from DHCP Server, the Link Local IP present.
   After ip query for a while (must over 2 Secons), if noboby conflict it(ARP 
   module will infromm if IP conflict), the IP can be set to sysyem. 
   Next sysyem reboot, we should use the this IP for initial query IP.
     												Ron Create on 12/13/04 */
     												

#define NO_USE 			0
#define PROBE			1
#define RETRY 			2
#define ANNOUNCE 		3
#define IDLE 			4

extern cyg_sem_t linklocal_sem;
extern cyg_sem_t linklocal_conflict;
extern int chang_ip_flag;

extern int is_linklocal_ip( unsigned int addr);
extern int LinkLocal_get_current_state();
extern void LinkLocal_set_state(int newState);
