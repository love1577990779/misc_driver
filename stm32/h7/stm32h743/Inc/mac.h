/*
 * mac.h
 *
 *  Created on: Oct 3, 2024
 *      Author: Administrator
 */

#ifndef MAC_H_
#define MAC_H_


#define ETH_TX_DESC_NUM (5)
#define ETH_RX_DESC_NUM (5)

#define TX_DMA_BUF_LEN (1520)
#define RX_DMA_BUF_LEN (1520)


struct tdes0_read_bit
{
  unsigned int buf1ap;
};

union tdes0_read
{
  unsigned int all;
  struct tdes0_read_bit bit;
};


struct tdes1_read_bit
{
  unsigned int buf2ap;
};

union tdes1_read
{
  unsigned int all;
  struct tdes1_read_bit bit;
};

struct tdes2_read_bit
{
  unsigned int b1l : 14;
  unsigned int vtir:2;
  unsigned int b2l:14;
  unsigned int ttse:1;
  unsigned int ioc:1;
};

union tdes2_read
{
  unsigned int all;
  struct tdes2_read_bit bit;
};

struct tdes3_read_bit
{
  unsigned int fl:15;
  unsigned int tpl:1;
  unsigned int cic:2;
  unsigned int tse:1;
  unsigned int thl:4;
  unsigned int saic:3;
  unsigned int cpc:2;
  unsigned int ld:1;
  unsigned int fd:1;
  unsigned int ctxt:1;
  unsigned int own:1;
};

union tdes3_read
{
  unsigned int all;
  struct tdes3_read_bit bit;
};



struct tdesc_read
{
	union tdes0_read tdes0;
	union tdes1_read tdes1;
	union tdes2_read tdes2;
	union tdes3_read tdes3;
};

struct tdes0_write_back_bit
{
  unsigned int ttsl : 32;
};

union tdes0_write_back
{
  unsigned int all;
  struct tdes0_write_back_bit bit;
};

struct tdes1_write_back_bit
{
  unsigned int ttsh : 32;
};

union tdes1_write_back
{
  unsigned int all;
  struct tdes1_write_back_bit bit;
};

struct tdes2_write_back_bit
{
  unsigned int rsvd : 32;
};

union tdes2_write_back
{
  unsigned int all;
  struct tdes2_write_back_bit bit;
};

struct tdes3_write_back_bit
{
  unsigned int ihe:1;
  unsigned int db:1;
  unsigned int uf:1;
  unsigned int ed:1;
  unsigned int cc:4;
  unsigned int ec:1;
  unsigned int lc:1;
  unsigned int nc:1;
  unsigned int loc:1;
  unsigned int pce:1;
  unsigned int ff:1;
  unsigned int jt:1;
  unsigned int es:1;
  unsigned int rsvd_0:1;
  unsigned int ttss:1;
  unsigned int rsvd_1:10;
  unsigned int ld:1;
  unsigned int fd:1;
  unsigned int ctxt:1;
  unsigned int own:1;

};

union tdes3_write_back
{
  unsigned int all;
  struct tdes3_write_back_bit bit;
};


struct rdes0_read_bit
{
  unsigned int buf1ap : 32;
};

struct rdes0_write_back_bit
{
  unsigned int ovt : 16;
  unsigned int ivt : 16;
};

union rdes0_read
{
  unsigned int all;
  struct tdes0_read_bit read_bit;
  struct rdes0_write_back_bit write_back_bit;
};

struct rdes1_read_bit
{
  unsigned int rsvd : 32;
};

struct rdes1_write_back_bit
{
  unsigned int pt : 3;
  unsigned int iphe : 1;
  unsigned int ipv4 : 1;
  unsigned int ipv6 : 1;
  unsigned int ipcb : 1;
  unsigned int ipce : 1;
  unsigned int pmt : 4;
  unsigned int pft : 1;
  unsigned int pv : 1;
  unsigned int tsa : 1;
  unsigned int td : 1;
  unsigned int opc : 16;


};


union rdes1_read
{
  unsigned int all;
  struct rdes1_read_bit read_bit;
  struct rdes1_write_back_bit write_back_bit;
};

struct rdes2_read_bit
{
  unsigned int buf2ap : 32;
};

struct rdes2_write_back_bit
{
  unsigned int rsvd_0 : 10;
  unsigned int arpnr:1;
  unsigned int rsvd_1 : 4;
  unsigned int vf:1;
  unsigned int saf:1;
  unsigned int daf:1;
  unsigned int hf:1;
  unsigned int madrm:8;
  unsigned int l3fm:1;
  unsigned int l4fm:1;
  unsigned int l3l4fm:3;

};


union rdes2_read
{
  unsigned int all;
  struct rdes2_read_bit read_bit;
  struct rdes2_write_back_bit write_back_bit;
};

struct rdes3_read_bit
{
	unsigned int rsvd_0:24;
	unsigned int buf1v:1;
	unsigned int buf2v :1;
	unsigned int rsvd_1:4;
	unsigned int ioc :1;
	unsigned int own:1;
};

struct rdes3_write_back_bit
{
	unsigned int pl:15;
	unsigned int es:1;
	unsigned int lt:3;
	unsigned int de:1;
	unsigned int re:1;
	unsigned int oe:1;
	unsigned int rwt:1;
	unsigned int gp:1;
	unsigned int ce:1;
	unsigned int rs0v:1;
	unsigned int rs1v:1;
	unsigned int rs2v:1;
	unsigned int ld:1;
	unsigned int fd:1;
	unsigned int ctxt:1;
	unsigned int own:1;
};


union rdes3_read
{
  unsigned int all;
  struct rdes3_read_bit read_bit;
  struct rdes3_write_back_bit write_back_bit;
};



struct rdesc_read
{
	union rdes0_read rdes0;
	union rdes1_read rdes1;
	union rdes2_read rdes2;
	union rdes3_read rdes3;
};

extern unsigned char mac_hw_addr[6];
extern volatile struct tdesc_read tx_desc[ETH_TX_DESC_NUM];
extern volatile struct rdesc_read rx_desc[ETH_RX_DESC_NUM];
extern unsigned int curr_tx_desc;
extern unsigned int curr_rx_desc;
extern unsigned char dma_tx_buf[ETH_TX_DESC_NUM][TX_DMA_BUF_LEN];
extern unsigned char dma_rx_buf[ETH_RX_DESC_NUM][RX_DMA_BUF_LEN];


void test_eth(void);

#endif /* MAC_H_ */
