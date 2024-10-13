/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif






#include "rcc.h"
#include "stm32h7xx.h"
#include "uasrt.h"
#include "mac.h"
#include "port.h"


void delay(unsigned int times)
{
	while(times--);
}

unsigned char tmp = 'a';

int main(void)
{
	init_rcc();
	init_usart();
	//init_mac();
	port_init_lwip();
	//test_eth();
	//while(1);

//	while(1)
//	{
//		delay(5000);
//		usart1_send(&tmp , 1);
//	}

	  while (1)
	  {
	    //tx_mutex_get(&mutex_eth, TX_WAIT_FOREVER);
	    ethernetif_input(&gnetif);
	    sys_check_timeouts();
	    //tx_mutex_put(&mutex_eth);
	    //tx_thread_relinquish();
	  }
}



