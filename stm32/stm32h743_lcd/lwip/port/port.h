#ifndef _PORT_H_
#define _PORT_H_

#include "ethernetif.h"

/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0                    192
#define IP_ADDR1                    168
#define IP_ADDR2                    2
#define IP_ADDR3                    30

/*NETMASK*/
#define NETMASK_ADDR0               255
#define NETMASK_ADDR1               255
#define NETMASK_ADDR2               255
#define NETMASK_ADDR3                 0

/*Gateway Address*/
#define GW_ADDR0                    192
#define GW_ADDR1                    168
#define GW_ADDR2                    2
#define GW_ADDR3                    1
/* USER CODE END 0 */

#define NETIF_MTU                      ( 1500 )


void port_init_lwip(void);

extern struct netif gnetif;
extern ip4_addr_t ipaddr;



#endif
