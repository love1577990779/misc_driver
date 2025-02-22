/*
 * mac.c
 *
 *  Created on: Sep 28, 2024
 *      Author: zero
 */

#include "stm32h7xx.h"
#include "main.h"
#include "mac.h"

unsigned char mac_hw_addr[6] = {0x2, 0, 0, 0x20, 0, 0};

#pragma pack(8)
unsigned char dma_tx_buf[ETH_TX_DESC_NUM][TX_DMA_BUF_LEN];
unsigned char dma_rx_buf[ETH_RX_DESC_NUM][RX_DMA_BUF_LEN];

unsigned int curr_tx_desc = 0;
unsigned int curr_rx_desc = 0;

volatile struct tdesc_read tx_desc[ETH_TX_DESC_NUM];
volatile struct rdesc_read rx_desc[ETH_RX_DESC_NUM];
#pragma pack()


void enable_mac(void)
{
	ETH->MACCR |= (1<<1)|(1<<0);
	ETH->DMACRCR = 0x10101;
	ETH->DMACRCR |= (1<<0);
	ETH->DMACTDTPR = (unsigned int)&tx_desc[ETH_TX_DESC_NUM-1];
}


void init_dma_desc(void)
{
  memset((void *)tx_desc, 0, sizeof(tx_desc));
  memset((void *)rx_desc, 0, sizeof(rx_desc));


  for (unsigned int i = 0; i < ETH_TX_DESC_NUM; i++)
  {
	  tx_desc[i].tdes0.bit.buf1ap = &dma_tx_buf[i];
	  tx_desc[i].tdes1.bit.buf2ap = 0;

	  tx_desc[i].tdes2.bit.b1l = TX_DMA_BUF_LEN;
	  tx_desc[i].tdes2.bit.b2l = 0;

	  tx_desc[i].tdes3.bit.cic = 3;

  }

  for (unsigned int i = 0; i < ETH_RX_DESC_NUM; i++)
  {
	  rx_desc[i].rdes0.read_bit.buf1ap = (unsigned int)(&dma_rx_buf[i]);
	  rx_desc[i].rdes2.read_bit.buf2ap = 0;
	  rx_desc[i].rdes3.read_bit.buf1v = 1;
	  rx_desc[i].rdes3.read_bit.buf2v = 0;
	  rx_desc[i].rdes3.read_bit.own = 1;
  }

  ETH->DMACTDLAR = (unsigned int)&tx_desc;
  ETH->DMACRDLAR = (unsigned int)&rx_desc;
//  ETH->DMACTDTPR = (unsigned int)&tx_desc[ETH_TX_DESC_NUM];
//  ETH->DMACRDTPR = (unsigned int)&rx_desc[ETH_RX_DESC_NUM];
  ETH->DMACTDRLR = ETH_TX_DESC_NUM-1;
  ETH->DMACRDRLR = ETH_RX_DESC_NUM-1;
}



void init_mac_clk(void)
{
	RCC->APB4ENR |= (1<<1);								//syscfg
	RCC->AHB1ENR |= (1<<17)|(1<<16)|(1<<15);
	RCC->AHB4ENR |= (1<<6)|(1<<2)|(1<<1)|(1<<0);
}


void init_mac_pin(void)
{
	//PC1,PC4,PC5
	GPIOC->MODER &= ~((0b11<<10)|(0b11<<8)|(0b11<<2));
	GPIOC->MODER |= (0b10<<10)|(0b10<<8)|(0b10<<2);
	GPIOC->OSPEEDR &= ~((0b11<<10)|(0b11<<8)|(0b11<<2));
	GPIOC->OSPEEDR |= (0b10<<10)|(0b10<<8)|(0b10<<2);
	GPIOC->AFR[0] &= ~((0b1111<<20)|(0b1111<<16)|(0b1111<<4));
	GPIOC->AFR[0] |= (11<<20)|(11<<16)|(11<<4);

	//PA1,PA2,PA7
	GPIOA->MODER &= ~((0b11<<14)|(0b11<<4)|(0b11<<2));
	GPIOA->MODER |= (0b10<<14)|(0b10<<4)|(0b10<<2);
	GPIOA->OSPEEDR &= ~((0b11<<10)|(0b11<<8)|(0b11<<2));
	GPIOA->OSPEEDR |= (0b10<<10)|(0b10<<8)|(0b10<<2);
	GPIOA->AFR[0] &= ~((0b1111<<28)|(0b1111<<8)|(0b1111<<4));
	GPIOA->AFR[0] |= (11<<28)|(11<<8)|(11<<4);

	//PB11
	GPIOB->MODER &= ~(0b11<<22);
	GPIOB->MODER |= (0b10<<22);
	GPIOB->OSPEEDR &= ~(0b11<<22);
	GPIOB->OSPEEDR |= (0b10<<22);
	GPIOB->AFR[1] &= ~((0b1111<<12));
	GPIOB->AFR[1] |= (11<<12);

	//PG13,PG14
	GPIOG->MODER &= ~((0b11<<28)|(0b11<<26));
	GPIOG->MODER |= (0b10<<28)|(0b10<<26);
	GPIOG->OSPEEDR &= ~((0b11<<28)|(0b11<<26));
	GPIOG->OSPEEDR |= (0b10<<28)|(0b10<<26);
	GPIOG->AFR[1] &= ~((0b1111<<24)|(0b1111<<20));
	GPIOG->AFR[1] |= (11<<24)|(11<<20);

	//PC3
//	GPIOC->MODER &= ~((0b11<<6));
//	GPIOC->MODER |= (0b10<<6);
//	GPIOC->AFR[0] &= ~(0b1111<<12);
//	GPIOC->AFR[0] |= (11<<12);

}


void write_phy(unsigned char addr, unsigned char reg, unsigned short value)
{
  while (ETH->MACMDIOAR & (1<<0));
  ETH->MACMDIOAR = (addr<<21)|(reg<<16)|(7<<12)|(0b100<<8)|(1<<2);
  ETH->MACMDIODR = value;
  ETH->MACMDIOAR |= (1<<0);
  while (ETH->MACMDIOAR & (1<<0));
}

unsigned short read_phy(unsigned char addr, unsigned char reg)
{
	while (ETH->MACMDIOAR & (1<<0));
	ETH->MACMDIOAR = (addr<<21)|(reg<<16)|(7<<12)|(0b100<<8)|(0b11<<2)|(1<<0);
	while (ETH->MACMDIOAR & (1<<0));
	return ETH->MACMDIODR & 0xffff;
}


void init_mac(void)
{
	init_mac_clk();
	init_mac_pin();

	SYSCFG->PMCR &= ~(0b111<<21);
	SYSCFG->PMCR |= (0b100<<21);

	ETH->DMAMR |= (1<<0);
	while(!(ETH->DMAMR & (1<<0)));

	ETH->MACMDIOAR &= ~(0b111<<8);
	ETH->MACMDIOAR |= (0b100<<8);

	ETH->MACECR = 1520;

	write_phy(0 , 0 , 0x8000);
	delay(0xfffff);
	while(read_phy(0 , 0))
		;

	write_phy(0,0,0x1000);
	delay(0xfffff);

	unsigned short read_reg = read_phy(0 , 1);
	while(!(read_reg & (1<<5)))
	{
		read_reg = read_phy(0 , 1);
		delay(0xfffff);
	}

	delay(0xfffff);



	read_reg = read_phy(0 , 0x11);
	while(!read_phy(0 , 0x11))
		;

	ETH->MACCR = 0x30006800;
	ETH->DMASBMR = 0x0;
	ETH->DMACCR = 0x00000;
	ETH->DMACTCR = 0x10000;
	ETH->DMACRCR = (1<<16)|(1520<<1);

	ETH->MTLRQOMR = 0x28;

	ETH->MACA0HR = ((unsigned int)mac_hw_addr[5] << 8) | (unsigned int)mac_hw_addr[4];
	ETH->MACA0LR = ((unsigned int)mac_hw_addr[3] << 24) | (unsigned int)mac_hw_addr[2] << 16 | ((unsigned int)mac_hw_addr[1] << 8) | (unsigned int)mac_hw_addr[0];

//	ETH->MACPFR &= ~(0b11<<6);
//	ETH->MACPFR |= (0b10<<6)|(1<<0);
	ETH->MACPFR = 0x800000c0;
	ETH->MACTFCR = 0;
	ETH->MACHWF1R &= ~(1<<18);




	init_dma_desc();

	ETH->MACCR |= (1<<1)|(1<<0);

	ETH->MTLTQOMR = (unsigned int)0xa;
	ETH->MTLTQOMR |= (1<<0);
	while(ETH->MTLTQOMR & (1<<0));

	ETH->DMACTCR |= (1<<0);
	ETH->DMACRCR |= (1<<0);


	//enable_mac();
}

void test_eth(void)
{
  //err_t errval = ERR_OK;
  //struct pbuf *q;
  unsigned int total_len = 0;
  unsigned char* tx_buf = 0;


  /* copy frame from pbufs to driver buffers */


  if(0 == tx_desc[curr_tx_desc].tdes3.bit.own)
  {
    tx_buf = (unsigned char*)(tx_desc[curr_tx_desc].tdes0.bit.buf1ap);
    tx_buf[0] = 0x1;
    tx_buf[1] = 0x2;
    tx_buf[1] = 0x3;

    tx_desc[curr_tx_desc].tdes3.bit.fl = 3;
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

  return;
}













