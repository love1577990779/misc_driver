#include "udp_app.h"
#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/init.h"
#include "netif/etharp.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"

#include "port.h"

struct udp_pcb *udpecho_pcb = NULL;
ip_addr_t dst_ip_addr;

static void udp_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
  unsigned short cmd_code = 0;


}


void init_udp_pcb(void)
{

  udpecho_pcb = udp_new();

  udp_bind(udpecho_pcb, &ipaddr, LOCAL_PORT);

  udp_recv(udpecho_pcb, udp_callback, NULL);

  IP4_ADDR(&dst_ip_addr, DST_IP_ADDR0, DST_IP_ADDR1, DST_IP_ADDR2, DST_IP_ADDR3);

}

void udp_data_send(uint8_t *p, uint16_t num)
{

  struct pbuf *q = pbuf_alloc(PBUF_TRANSPORT, num, PBUF_POOL);

  memcpy(q->payload, p, num);
  udp_sendto(udpecho_pcb, q, &dst_ip_addr, DST_PORT);

  pbuf_free(q);

}


void test_udp_send(void)
{
	char data[] = {0xaa , 0x55};
	  struct pbuf *q = pbuf_alloc(PBUF_TRANSPORT, sizeof(data), PBUF_POOL);

	  memcpy(q->payload, data, sizeof(data));
	  udp_sendto(udpecho_pcb, q, &dst_ip_addr, DST_PORT);

	  pbuf_free(q);
}
