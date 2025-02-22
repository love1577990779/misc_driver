/*
 * usart.c
 *
 *  Created on: Sep 26, 2024
 *      Author: zero
 */

#include "stm32h7xx.h"



void init_usart1_clk(void)
{
	RCC->AHB4ENR |= (1<<0);
	RCC->APB2ENR |= (1<<4);
	//rcc_pclk2作为usart1的时钟

}

void init_usart1_pin(void)
{
	GPIOA->MODER &= ~(0xf<<18);
	GPIOA->MODER |= (2<<20)|(2<<18);
	GPIOA->AFR[1] &= ~(0xf<<4);
	GPIOA->AFR[1] |= (7<<4);
	GPIOA->AFR[1] &= ~(0xf<<8);
	GPIOA->AFR[1] |= (7<<8);
}

void init_usart(void)
{
	init_usart1_clk();
	init_usart1_pin();
	USART1->BRR = 0x364;
	USART1->PRESC = 0;
	USART1->CR1 |= (1<<3)|(1<<2)|(1<<0);

}

void usart1_send(unsigned char* data , unsigned int num)
{
	for(unsigned int i = 0 ; i < num ; i++)
	{
		while(!(USART1->ISR & (1<<6)));
		USART1->TDR = data[i];
	}
}


