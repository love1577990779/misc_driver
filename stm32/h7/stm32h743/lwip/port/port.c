#include <lwip/opt.h>
#include <lwip/arch.h>
#include "lwip/tcpip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "ethernetif.h"
#include "netif/ethernet.h"
#include "lwip/def.h"
#include "lwip/stats.h"
#include "lwip/etharp.h"
#include "lwip/ip.h"
#include "lwip/snmp.h"
#include "lwip/timeouts.h"
//#include "app_config.h"
#include "port.h"
#include "mac.h"
#include <stdio.h>
#include <string.h>
#include "stm32h7xx.h"

struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;
uint8_t IP_ADDRESS[4];
uint8_t NETMASK_ADDRESS[4];
uint8_t GATEWAY_ADDRESS[4];










static void low_level_init(struct netif *netif)
{
  init_mac();
  netif->flags |= NETIF_FLAG_LINK_UP;

#if LWIP_ARP || LWIP_ETHERNET

  /* set MAC hardware address length */
  netif->hwaddr_len = ETH_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] =  mac_hw_addr[0];
  netif->hwaddr[1] =  mac_hw_addr[1];
  netif->hwaddr[2] =  mac_hw_addr[2];
  netif->hwaddr[3] =  mac_hw_addr[3];
  netif->hwaddr[4] =  mac_hw_addr[4];
  netif->hwaddr[5] =  mac_hw_addr[5];

  /* maximum transfer unit */
  netif->mtu = NETIF_MTU;

  #if LWIP_ARP
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  #else
    netif->flags |= NETIF_FLAG_BROADCAST;
  #endif /* LWIP_ARP */

#endif /* LWIP_ARP || LWIP_ETHERNET */

    netif_set_up(netif);
        netif_set_link_up(netif);
  //init_dma_desc();
  //enable_mac();

}




static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  err_t errval = ERR_OK;
  struct pbuf *q;
  unsigned int total_len = 0;
  unsigned char* tx_buf = 0;
  /* copy frame from pbufs to driver buffers */

  if(0 == tx_desc[curr_tx_desc].tdes3.bit.own)
  {
    tx_buf = (unsigned char*)(tx_desc[curr_tx_desc].tdes0.all);

    for(q = p; q != NULL; q = q->next)
    {
      if(total_len + q->len <= TX_DMA_BUF_LEN)
      {
        memcpy(tx_buf , q->payload , q->len);
        total_len += q->len;
      }
      else
      {
        errval = ERR_MEM;
        //return errval;
      }
    }

    tx_desc[curr_tx_desc].tdes2.bit.b1l = total_len;
    tx_desc[curr_tx_desc].tdes3.bit.fl = 0;
    tx_desc[curr_tx_desc].tdes3.bit.fd = 1;
    tx_desc[curr_tx_desc].tdes3.bit.ld = 1;
    tx_desc[curr_tx_desc].tdes3.bit.own = 1;

  }

  ETH->DMACTDTPR = (unsigned int)&tx_desc[ETH_TX_DESC_NUM-1];

  curr_tx_desc++;
  if(curr_tx_desc >= ETH_TX_DESC_NUM)
  {
    curr_tx_desc = 0;
  }

  return errval;
}



static struct pbuf * low_level_input(struct netif *netif)
{
  struct pbuf *p = 0;
  unsigned int dma_rx_len = 0;
  if(0 == rx_desc[curr_rx_desc].rdes3.read_bit.own)
  {
    if((1 == rx_desc[curr_rx_desc].rdes3.write_back_bit.ld))
    {
      dma_rx_len = rx_desc[curr_rx_desc].rdes3.write_back_bit.pl;
      p = pbuf_alloc(PBUF_RAW, dma_rx_len, PBUF_POOL);
      if(p->len >= dma_rx_len)
      {
        memcpy(p->payload , (void*)(&dma_rx_buf[curr_rx_desc]) , dma_rx_len);
      }
      else
      {
        pbuf_free(p);
        p = NULL;
      }

    }

    memset(&rx_desc[curr_rx_desc] , 0 , sizeof(rx_desc[curr_rx_desc]));
	rx_desc[curr_rx_desc].rdes0.read_bit.buf1ap = (unsigned int)(&dma_rx_buf[curr_rx_desc]);
	rx_desc[curr_rx_desc].rdes2.read_bit.buf2ap = 0;
	rx_desc[curr_rx_desc].rdes3.read_bit.buf1v = 1;
	rx_desc[curr_rx_desc].rdes3.read_bit.buf2v = 0;
	rx_desc[curr_rx_desc].rdes3.read_bit.own = 1;

    curr_rx_desc++;
    if(curr_rx_desc >= ETH_RX_DESC_NUM)
      curr_rx_desc = 0;
  }

  ETH->DMACRDTPR = (unsigned int)&rx_desc[ETH_TX_DESC_NUM-1];

  return p;

}



void ethernetif_input(struct netif *netif)
{
  volatile err_t err;
  struct pbuf *p = NULL;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);

  /* no packet could be read, silently ignore this */
  if (p == NULL)
    return;

  /* entry point to the LwIP stack */
  err = netif->input(p, netif);

  if(NULL != p)
    pbuf_free(p);
}





err_t ethernetif_init(struct netif *netif)
{

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  netif->name[0] = 's';
  netif->name[1] = 'b';

  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  low_level_init(netif);

  return ERR_OK;

}














void port_init_lwip(void)
{
  /* IP addresses initialization */
  /* USER CODE BEGIN 0 */
  IP4_ADDR(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
  IP4_ADDR(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
  IP4_ADDR(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);

  /* USER CODE END 0 */

  /* Initilialize the LwIP stack without RTOS */
  lwip_init();

  /* add the network interface (IPv4/IPv6) without RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /* Registers the default network interface */
  netif_set_default(&gnetif);

  if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);
  }
  else
  {
    /* When the netif link is down this function must be called */
    netif_set_down(&gnetif);
  }


}


u32_t sys_now(void)
{
  return 0;
}
