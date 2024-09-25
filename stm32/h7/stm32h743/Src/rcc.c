#include "stm32h7xx.h"


void init_rcc(void)
{
	SCB->CPACR |= ((3UL << (10*2))|(3UL << (11*2)));
	PWR->D3CR |= (3<<14);
	FLASH->KEYR1 = 0x45670123;
	FLASH->KEYR1 = 0xCDEF89AB;
	FLASH->ACR = 0x1f;


	RCC->PLLCFGR = 0x1FF0888;
	RCC->PLLCKSELR = 0x505052;
	RCC->PLL1DIVR = (1<<24)|(3<<16)|(1<<9)|(159);
	RCC->PLL2DIVR = (1<<24)|(3<<16)|(1<<9)|(159);
	RCC->PLL3DIVR = (1<<24)|(3<<16)|(1<<9)|(159);
	RCC->D1CFGR = (0b100<<4)|(0b1000);
	RCC->D2CFGR = (0b100<<8)|(0b100<<4);
	RCC->D3CFGR = (0b100<<4);

	RCC->CR |= (1<<16);
	while(!(RCC->CR & (1<<17)));

	RCC->CR |= (1<<24);
	while(!(RCC->CR & (1<<25)));

	unsigned int tmp = RCC->CFGR;
	tmp &= ~(0b111);
	tmp |= 3;
	RCC->CFGR = tmp;

	while((0b11<<3) != (RCC->CFGR & (0b111<<3)));

}

