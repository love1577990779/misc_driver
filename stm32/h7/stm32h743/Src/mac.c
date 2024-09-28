/*
 * mac.c
 *
 *  Created on: Sep 28, 2024
 *      Author: zero
 */

#include "stm32h7xx.h"
#include "main.h"

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
	GPIOC->AFR[0] &= ~((0b1111<<20)|(0b1111<<16)|(0b1111<<4));
	GPIOC->AFR[0] |= (11<<20)|(11<<16)|(11<<4);

	//PA2,PA7
	GPIOA->MODER &= ~((0b11<<14)|(0b11<<4));
	GPIOA->MODER |= (0b10<<14)|(0b10<<4);
	GPIOA->AFR[0] &= ~((0b1111<<28)|(0b1111<<8));
	GPIOA->AFR[0] |= (11<<28)|(11<<8);

	//PB11
	GPIOB->MODER &= ~(0b11<<22);
	GPIOB->MODER |= (0b10<<22);
	GPIOB->AFR[1] &= ~((0b1111<<12));
	GPIOB->AFR[1] |= (11<<12);

	//PG13,PG14
	GPIOG->MODER &= ~((0b11<<28)|(0b11<<26));
	GPIOG->MODER |= (0b10<<28)|(0b10<<26);
	GPIOG->AFR[1] &= ~((0b1111<<24)|(0b1111<<20));
	GPIOG->AFR[1] |= (11<<24)|(11<<20);

	//

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
	ETH->MACMDIOAR |= 0b100;

	write_phy(0 , 0 , 0x8000);
	delay(0xfffff);
	volatile unsigned short read_reg = read_phy(0 , 0);

	write_phy(0,0,0x1000);
	delay(0xfffff);

	read_reg = 0;
	while(!(read_reg & (1<<5)))
	{
		read_reg = read_phy(0 , 1);
		delay(0xfffff);
	}

	delay(0xfffff);
}
