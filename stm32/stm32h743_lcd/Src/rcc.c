#include "stm32h7xx.h"


void init_rcc(void)
{
	SCB->CPACR |= ((3UL << (10*2))|(3UL << (11*2)));
	PWR->D3CR |= (3<<14);
	FLASH->KEYR1 = 0x45670123;
	FLASH->KEYR1 = 0xCDEF89AB;
	FLASH->ACR = 0x1f;


	RCC->PLLCFGR = 0x888;

	RCC->PLLCKSELR = 0x505052;
	//RCC->PLLCKSELR &= ~(0b111111<<20);
	//RCC->PLLCKSELR |= (25<<20);

	RCC->PLLCFGR |= 0x1FF<<16;






//Fvco=25*160/5=800M

	RCC->PLL1DIVR = (1<<24)|(3<<16)|(1<<9)|(159);
	RCC->PLL2DIVR = (1<<24)|(3<<16)|(1<<9)|(159);
	RCC->PLL3DIVR = (8<<24)|(1<<16)|(0<<9)|(59);
	//RCC->PLL3DIVR = (8<<24)|(1<<16)|(0<<9)|(299);



	RCC->D1CFGR = (0b100<<4)|(0b1000);
	RCC->D2CFGR = (0b100<<8)|(0b100<<4);
	RCC->D3CFGR = (0b100<<4);

	RCC->CR |= (1<<16);
	while(!(RCC->CR & (1<<17)));

	RCC->CR |= (1<<24)|(1<<26)|(1<<28);
	while(((1<<25)|(1<<27)|(1<<29)) != (RCC->CR & ((1<<25)|(1<<27)|(1<<29))));




	unsigned int tmp = RCC->CFGR;
	tmp &= ~(0b111);
	tmp |= 3;
	RCC->CFGR = tmp;

	while((0b11<<3) != (RCC->CFGR & (0b111<<3)));

}

