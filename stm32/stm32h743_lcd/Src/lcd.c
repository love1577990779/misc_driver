#include "stm32h7xx.h"
#include "lcd.h"
#include "sys.h"

_ltdc_dev lcdltdc;
__attribute__ ((section (".sdram"))) unsigned int layer1_buf[HEIGHT*WIDTH];
//unsigned int layer2_buf[HEIGHT*WIDTH];
uint32_t *g_ltdc_framebuf[2];
uint16_t* ltdc_lcd_framebuf = 0xc0000000;
void init_lcd_pin(void)
{
	RCC->AHB4ENR |= (1<<7)|(1<<8)|(1<<5)|(1<<6)|(1<<1);

	//PB5(背光)
	GPIOB->MODER &= ~(0b11<<10);
	GPIOB->MODER |= (0b01<<10);			//输出模式
	GPIOB->PUPDR &= ~((0b11<<10));
	GPIOB->PUPDR |= (0b1<<10);
	GPIOB->OSPEEDR &= ~(0b11<<10);
	GPIOB->OSPEEDR |= (0b11<<10);


	//PF10
	GPIOF->MODER &= ~((0b11<<20));
	GPIOF->MODER |= (0b10<<20);			//AF模式
	GPIOF->PUPDR &= ~((0b11<<20));
	GPIOF->PUPDR |= (0b1<<20);
	GPIOF->OSPEEDR &= ~((0b11<<20));
	GPIOF->OSPEEDR |= (0b11<<20);
	GPIOF->AFR[1] &= ~(0b1111<<8);
	GPIOF->AFR[1] |= (14<<8);

	//PG6 PG7 PG11
	GPIOG->MODER &= ~((0b11<<12)|(0b11<<14)|(0b11<<22));
	GPIOG->MODER |= (0b10<<12)|(0b10<<14)|(0b10<<22);			//AF模式
	GPIOG->PUPDR &= ~((0b11<<12)|(0b11<<14)|(0b11<<22));
	GPIOG->PUPDR |= (0b1<<12)|(0b1<<14)|(0b1<<22);

	GPIOG->OSPEEDR &= ~((0b11<<12)|(0b11<<14)|(0b11<<22));
	GPIOG->OSPEEDR |= (0b11<<12)|(0b11<<14)|(0b11<<22);
	GPIOG->AFR[0] &= ~((0b1111<<24)|(0b1111<<28));
	GPIOG->AFR[0] |= (14<<24)|(14<<28);
	GPIOG->AFR[1] &= ~(0b1111<<12);
	GPIOG->AFR[1] |= (14<<12);

	//PH9 PH10 PH11 PH12 PH13 PH14 PH15
	GPIOH->MODER &= ~((0b11<<18)|(0b11<<20)|(0b11<<22)|(0b11<<24)|(0b11<<26)|(0b11<<28)|(0b11<<30));
	GPIOH->MODER |= (0b10<<18)|(0b10<<20)|(0b10<<22)|(0b10<<24)|(0b10<<26)|(0b10<<28)|(0b10<<30);			//AF模式
	GPIOH->PUPDR &= ~((0b11<<18)|(0b11<<20)|(0b11<<22)|(0b11<<24)|(0b11<<26)|(0b11<<28)|(0b11<<30));
	GPIOH->PUPDR |= (0b01<<18)|(0b1<<20)|(0b1<<22)|(0b1<<24)|(0b1<<26)|(0b1<<28)|(0b1<<30);
	GPIOH->OSPEEDR &= ~((0b11<<18)|(0b11<<20)|(0b11<<22)|(0b11<<24)|(0b11<<26)|(0b11<<28)|(0b11<<30));
	GPIOH->OSPEEDR |= (0b11<<18)|(0b11<<20)|(0b11<<22)|(0b11<<24)|(0b11<<26)|(0b11<<28)|(0b11<<30);
	GPIOH->AFR[1] &= ~((0b1111<<4)|(0b1111<<8)|(0b1111<<12)|(0b1111<<16)|(0b1111<<20)|(0b1111<<24)|(0b1111<<28));
	GPIOH->AFR[1] |= (14<<4)|(14<<8)|(14<<12)|(14<<16)|(14<<20)|(14<<24)|(14<<28);

	//PI0 PI1 PI2 PI4 PI5 PI6 PI7 PI9 PI10
	GPIOI->MODER &= ~((0b11<<0)|(0b11<<2)|(0b11<<4)|(0b11<<8)|(0b11<<10)|(0b11<<12)|(0b11<<14)|(0b11<<18)|(0b11<<20));
	GPIOI->MODER |= (0b10<<0)|(0b10<<2)|(0b10<<4)|(0b10<<8)|(0b10<<10)|(0b10<<12)|(0b10<<14)|(0b10<<18)|(0b10<<20);			//AF模式
	GPIOI->PUPDR &= ~((0b11<<0)|(0b11<<2)|(0b11<<4)|(0b11<<8)|(0b11<<10)|(0b11<<12)|(0b11<<14)|(0b11<<18)|(0b11<<20));
	GPIOI->PUPDR |= (0b1<<0)|(0b1<<2)|(0b1<<4)|(0b1<<8)|(0b1<<10)|(0b1<<12)|(0b1<<14)|(0b1<<18)|(0b1<<20);

	GPIOI->OSPEEDR &= ~((0b11<<0)|(0b11<<2)|(0b11<<4)|(0b11<<8)|(0b11<<10)|(0b11<<12)|(0b11<<14)|(0b11<<18)|(0b11<<20));
	GPIOI->OSPEEDR |= (0b11<<0)|(0b11<<2)|(0b11<<4)|(0b11<<8)|(0b11<<10)|(0b11<<12)|(0b11<<14)|(0b11<<18)|(0b11<<20);
	GPIOI->AFR[0] &= ~((0b1111<<0)|(0b1111<<4)|(0b1111<<8)|(0b1111<<16)|(0b1111<<20)|(0b1111<<24)|(0b1111<<28));
	GPIOI->AFR[0] |= (14<<0)|(14<<4)|(14<<8)|(14<<16)|(14<<20)|(14<<24)|(14<<28);
	GPIOI->AFR[1] &= ~((0b1111<<4)|(0b1111<<8));
	GPIOI->AFR[1] |= (14<<4)|(14<<8);
}


void ltdc_switch(uint8_t sw)
{
    if (sw)
    {
        LTDC->GCR |= 1 << 0;    /* ´ò¿ªLTDC */
    }
    else
    {
        LTDC->GCR &= ~(1 << 0); /* ¹Ø±ÕLTDC */
    }
}


void ltdc_layer_switch(uint8_t layerx, uint8_t sw)
{
    if (sw)
    {
        if (layerx == 0)LTDC_Layer1->CR |= 1 << 0;      /* ¿ªÆô²ã1 */
        else LTDC_Layer2->CR |= 1 << 0;                 /* ¿ªÆô²ã2 */
    }
    else
    {
        if (layerx == 0)LTDC_Layer1->CR &= ~(1 << 0);   /* ¹Ø±Õ²ã1 */
        else LTDC_Layer2->CR &= ~(1 << 0);              /* ¹Ø±Õ²ã2 */
    }

    LTDC->SRCR |= 1 << 0;                               /* ÖØÐÂ¼ÓÔØÅäÖÃ */
}


/**
 * @brief       LTDC²ã´°¿ÚÉèÖÃ, ´°¿ÚÒÔLCDÃæ°å×ø±êÏµÎª»ù×¼
 * @note        ´Ëº¯Êý±ØÐëÔÚltdc_layer_parameter_configÖ®ºóÔÙÉèÖÃ.ÁíÍâ,µ±ÉèÖÃµÄ´°¿ÚÖµ²»µÈÓÚÃæ°åµÄ³ß
 *              ´çÊ±,GRAMµÄ²Ù×÷(¶Á/Ð´µãº¯Êý),Ò²Òª¸ù¾Ý´°¿ÚµÄ¿í¸ßÀ´½øÐÐÐÞ¸Ä,·ñÔòÏÔÊ¾²»Õý³£(±¾Àý³Ì¾ÍÎ´×öÐÞ¸Ä).
 * @param       layerx      : 0,µÚÒ»²ã; 1,µÚ¶þ²ã;
 * @param       sx, sy      : ÆðÊ¼×ø±ê
 * @param       width,height: ¿í¶ÈºÍ¸ß¶È
 * @retval      ÎÞ
 */
void ltdc_layer_window_config(uint8_t layerx, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    uint32_t temp;
    uint8_t pixformat = 0;

    if (layerx == 0)
    {
        temp = (sx + width + ((LTDC->BPCR & 0X0FFF0000) >> 16)) << 16;
        LTDC_Layer1->WHPCR = (sx + ((LTDC->BPCR & 0X0FFF0000) >> 16) + 1) | temp;   /* ÉèÖÃÐÐÆðÊ¼ºÍ½áÊøÎ»ÖÃ */
        temp = (sy + height + (LTDC->BPCR & 0X7FF)) << 16;
        LTDC_Layer1->WVPCR = (sy + (LTDC->BPCR & 0X7FF) + 1) | temp;    /* ÉèÖÃÁÐÆðÊ¼ºÍ½áÊøÎ»ÖÃ */
        pixformat = LTDC_Layer1->PFCR & 0X07;                           /* µÃµ½ÑÕÉ«¸ñÊ½ */

        if (pixformat == 0)temp = 4;                                    /* ARGB8888,Ò»¸öµã4¸ö×Ö½Ú */
        else if (pixformat == 1)temp = 3;                               /* RGB888,Ò»¸öµã3¸ö×Ö½Ú */
        else if (pixformat == 5 || pixformat == 6)temp = 1;             /* L8/AL44,Ò»¸öµã1¸ö×Ö½Ú */
        else temp = 2;                                                  /* ÆäËû¸ñÊ½,Ò»¸öµã2¸ö×Ö½Ú */

        LTDC_Layer1->CFBLR = (width * temp << 16) | (width * temp + 3); /* Ö¡»º³åÇø³¤¶ÈÉèÖÃ(×Ö½ÚÎªµ¥Î») */
        LTDC_Layer1->CFBLNR = height;                                   /* Ö¡»º³åÇøÐÐÊýÉèÖÃ */
    }
    else
    {
        temp = (sx + width + ((LTDC->BPCR & 0X0FFF0000) >> 16)) << 16;
        LTDC_Layer2->WHPCR = (sx + ((LTDC->BPCR & 0X0FFF0000) >> 16) + 1) | temp;   /* ÉèÖÃÐÐÆðÊ¼ºÍ½áÊøÎ»ÖÃ */
        temp = (sy + height + (LTDC->BPCR & 0X7FF)) << 16;
        LTDC_Layer2->WVPCR = (sy + (LTDC->BPCR & 0X7FF) + 1) | temp;    /* ÉèÖÃÁÐÆðÊ¼ºÍ½áÊøÎ»ÖÃ */
        pixformat = LTDC_Layer2->PFCR & 0X07;                           /* µÃµ½ÑÕÉ«¸ñÊ½ */

        if (pixformat == 0)temp = 4;                                    /* ARGB8888,Ò»¸öµã4¸ö×Ö½Ú */
        else if (pixformat == 1)temp = 3;                               /* RGB888,Ò»¸öµã3¸ö×Ö½Ú */
        else if (pixformat == 5 || pixformat == 6)temp = 1;             /* L8/AL44,Ò»¸öµã1¸ö×Ö½Ú */
        else temp = 2;                                                  /* ÆäËû¸ñÊ½,Ò»¸öµã2¸ö×Ö½Ú */

        LTDC_Layer2->CFBLR = (width * temp << 16) | (width * temp + 3); /* Ö¡»º³åÇø³¤¶ÈÉèÖÃ(×Ö½ÚÎªµ¥Î») */
        LTDC_Layer2->CFBLNR = height;                                   /* Ö¡»º³åÇøÐÐÊýÉèÖÃ */
    }

    ltdc_layer_switch(layerx, 1);                                       /* ²ãÊ¹ÄÜ */
}


/**
 * @brief       LTDC²ã»ù±¾²ÎÊýÉèÖÃ
 *  @note       ´Ëº¯Êý,±ØÐëÔÚltdc_layer_window_configÖ®Ç°ÉèÖÃ.
 * @param       layerx      : 0,µÚÒ»²ã; 1,µÚ¶þ²ã;
 * @param       bufaddr     : ²ãÑÕÉ«Ö¡»º´æÆðÊ¼µØÖ·
 * @param       pixformat   : ÑÕÉ«¸ñÊ½. 0,ARGB8888; 1,RGB888; 2,RGB565; 3,ARGB1555; 4,ARGB4444; 5,L8; 6;AL44; 7;AL88
 * @param       alpha       : ²ãÑÕÉ«AlphaÖµ, 0,È«Í¸Ã÷;255,²»Í¸Ã÷
 * @param       alpha0      : Ä¬ÈÏÑÕÉ«AlphaÖµ, 0,È«Í¸Ã÷;255,²»Í¸Ã÷
 * @param       bfac1       : »ìºÏÏµÊý1, 4(100),ºã¶¨µÄAlpha; 6(101),ÏñËØAlpha*ºã¶¨Alpha
 * @param       bfac2       : »ìºÏÏµÊý2, 5(101),ºã¶¨µÄAlpha; 7(111),ÏñËØAlpha*ºã¶¨Alpha
 * @param       bkcolor     : ²ãÄ¬ÈÏÑÕÉ«,32Î»,µÍ24Î»ÓÐÐ§,RGB888¸ñÊ½
 * @retval      ÎÞ
 */
void ltdc_layer_parameter_config(uint8_t layerx, uint32_t bufaddr, uint8_t pixformat, uint8_t alpha, uint8_t alpha0, uint8_t bfac1, uint8_t bfac2, uint32_t bkcolor)
{
    if (layerx == 0)
    {
        LTDC_Layer1->CFBAR = bufaddr;                           /* ÉèÖÃ²ãÑÕÉ«Ö¡»º´æÆðÊ¼µØÖ· */
        LTDC_Layer1->PFCR = pixformat;                          /* ÉèÖÃ²ãÑÕÉ«¸ñÊ½ */
        LTDC_Layer1->CACR = alpha;                              /* ÉèÖÃ²ãÑÕÉ«AlphaÖµ,255·ÖÆµ;ÉèÖÃ255,Ôò²»Í¸Ã÷ */
        LTDC_Layer1->DCCR = ((uint32_t)alpha0 << 24) | bkcolor; /* ÉèÖÃÄ¬ÈÏÑÕÉ«AlphaÖµ,ÒÔ¼°Ä¬ÈÏÑÕÉ« */
        LTDC_Layer1->BFCR = ((uint32_t)bfac1 << 8) | bfac2;     /* ÉèÖÃ²ã»ìºÏÏµÊý */
    }
    else
    {
        LTDC_Layer2->CFBAR = bufaddr;                           /* ÉèÖÃ²ãÑÕÉ«Ö¡»º´æÆðÊ¼µØÖ· */
        LTDC_Layer2->PFCR = pixformat;                          /* ÉèÖÃ²ãÑÕÉ«¸ñÊ½ */
        LTDC_Layer2->CACR = alpha;                              /* ÉèÖÃ²ãÑÕÉ«AlphaÖµ,255·ÖÆµ;ÉèÖÃ255,Ôò²»Í¸Ã÷ */
        LTDC_Layer2->DCCR = ((uint32_t)alpha0 << 24) | bkcolor; /* ÉèÖÃÄ¬ÈÏÑÕÉ«AlphaÖµ,ÒÔ¼°Ä¬ÈÏÑÕÉ« */
        LTDC_Layer2->BFCR = ((uint32_t)bfac1 << 8) | bfac2;     /* ÉèÖÃ²ã»ìºÏÏµÊý */
    }
}



void init_lcd(void)
{
	init_lcd_pin();
	GPIOB->ODR |= (1<<5);   //打开背光
	//ltdc的时钟频率固定为pll3_r_ck,设置为33M
	RCC->APB3ENR |= 1<<3;
	LTDC->GCR = 0;
	LTDC->SSCR = (VSW-1)|((HSW-1)<<16);
	//LTDC->SSCR = 0x002f0002;
	LTDC->BPCR = (VSW+VBP-1)|((HSW+HBP-1)<<16);
	LTDC->AWCR = (VSW+VBP+HEIGHT-1)|((HSW+HBP+WIDTH-1)<<16);
	LTDC->TWCR = (VSW+VBP+HEIGHT+VFP-1)|((HSW+HBP+WIDTH+HFP-1)<<16);
	LTDC->BCCR = 0x00000000;
	LTDC->GCR |= 1<<0;

	LTDC_Layer1->CFBAR = layer1_buf;
	LTDC_Layer1->PFCR = 0;						//ARGB
	LTDC_Layer1->CACR = 255;
	LTDC_Layer1->DCCR = 0;
	LTDC_Layer1->BFCR = (0b110<<8)|(0b111);

	LTDC_Layer1->WHPCR = (((LTDC->BPCR & 0x0FFF0000) >> 16) + 1)|((((LTDC->BPCR & 0x0FFF0000) >> 16) + 1 + WIDTH)<<16);
	LTDC_Layer1->WVPCR = ((LTDC->BPCR & 0X7FF) + 1)|(((LTDC->BPCR & 0X7FF) + 1 + HEIGHT)<<16);
	LTDC_Layer1->CFBLR = (WIDTH * 4 << 16) | (WIDTH * 4 + 7);
	LTDC_Layer1->CFBLNR = HEIGHT;
	LTDC_Layer1->CR = 1;
	LTDC->SRCR |= 1 << 0;

}

void lcd_fill_color(unsigned int color)
{
	for(int i = 0 ; i < HEIGHT*WIDTH ; i++)
		layer1_buf[i] = color;

}

uint8_t ltdc_clk_set(uint32_t pll3n, uint32_t pll3m, uint32_t pll3r)
{
    uint16_t retry = 0;
    uint8_t status = 0;

    RCC->CR &= ~(1 << 28);  /* ¹Ø±ÕPLL3Ê±ÖÓ */

    while (((RCC->CR & (1 << 29))) && (retry < 0X1FFF))retry++; /* µÈ´ýPLL3Ê±ÖÓÊ§Ëø */

    if (retry == 0X1FFF)status = 1;         /* LTDCÊ±ÖÓ¹Ø±ÕÊ§°Ü */
    else
    {
        RCC->PLLCKSELR &= ~(0X3F << 20);    /* Çå³ýDIVM3[5:0]Ô­À´µÄÉèÖÃ */
        RCC->PLLCKSELR |= pll3m << 20;      /* DIVM3[5:0] = 25,ÉèÖÃPLL3µÄÔ¤·ÖÆµÏµÊý */
        RCC->PLL3DIVR &= ~(0X1FF << 0);     /* Çå³ýDIVN3[8:0]Ô­À´µÄÉèÖÃ */
        RCC->PLL3DIVR |= (pll3n - 1) << 0;  /* DIVN3[8:0] = pll3n - 1, ÉèÖÃPLL3µÄ±¶ÆµÏµÊý,ÉèÖÃÖµÐè¼õ1 */
        RCC->PLL3DIVR &= ~(0X7F << 24);     /* Çå³ýDIVR2[6:0]Ô­À´µÄÉèÖÃ */
        RCC->PLL3DIVR |= (pll3r - 1) << 24; /* DIVR3[8:0] = pll3r - 1,ÉèÖÃPLL3µÄ±¶ÆµÏµÊý,ÉèÖÃÖµÐè¼õ1 */

        RCC->PLLCFGR &= ~(0X0F << 8);       /* Çå³ýPLL3RGE[1:0]/PLL3VCOSEL/PLL3FRACENµÄÉèÖÃ */
        RCC->PLLCFGR |= 0 << 10;            /* PLL3RGE[1:0] = 0,PLL3ÊäÈëÊ±ÖÓÆµÂÊÔÚ1~2MhzÖ®¼ä(25 / 25 = 1Mhz) */
        RCC->PLLCFGR |= 0 << 9;             /* PLL3VCOSEL = 0, PLL3µÄVCO·¶Î§,192~836Mhz */
        RCC->PLLCFGR |= 1 << 24;            /* DIVR3EN = 1, Ê¹ÄÜpll3_r_ck */
        RCC->CR |= 1 << 28;                 /* PLL3ON = 1, Ê¹ÄÜPLL3 */

        while (((RCC->CR & (1 << 29)) == 0) && (retry < 0X1FFF))retry++;    /* µÈ´ýPLL3Ê±ÖÓËø¶¨ */

        if (retry == 0X1FFF)status = 2;
    }

    return status;
}


void ltdc_init(void)
{
    uint32_t tempreg = 0;
    uint16_t lcdid = 0;

    lcdid = 0X4384;    /* ¶ÁÈ¡LCDÃæ°åID */
    init_lcd_pin();

#if 0
    /* ÒÔÏÂÊÇLTDCÐÅºÅ¿ØÖÆÒý½Å BL/DE/VSYNC/HSYNC/CLKµÈµÄÅäÖÃ */
    LTDC_BL_GPIO_CLK_ENABLE();      /* LTDC_BL½ÅÊ±ÖÓÊ¹ÄÜ */
    LTDC_DE_GPIO_CLK_ENABLE();      /* LTDC_DE½ÅÊ±ÖÓÊ¹ÄÜ */
    LTDC_VSYNC_GPIO_CLK_ENABLE();   /* LTDC_VSYNC½ÅÊ±ÖÓÊ¹ÄÜ */
    LTDC_HSYNC_GPIO_CLK_ENABLE();   /* LTDC_HSYNC½ÅÊ±ÖÓÊ¹ÄÜ */
    LTDC_CLK_GPIO_CLK_ENABLE();     /* LTDC_CLK½ÅÊ±ÖÓÊ¹ÄÜ */

    sys_gpio_set(LTDC_BL_GPIO_PORT, LTDC_BL_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);  /* LTDC_BLÒý½ÅÄ£Ê½ÉèÖÃ(ÍÆÍìÊä³ö) */

    sys_gpio_set(LTDC_DE_GPIO_PORT, LTDC_DE_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC_DEÒý½ÅÄ£Ê½ÉèÖÃ */

    sys_gpio_set(LTDC_VSYNC_GPIO_PORT, LTDC_VSYNC_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC_VSYNCÒý½ÅÄ£Ê½ÉèÖÃ */

    sys_gpio_set(LTDC_HSYNC_GPIO_PORT, LTDC_HSYNC_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC_HSYNCÒý½ÅÄ£Ê½ÉèÖÃ */

    sys_gpio_set(LTDC_CLK_GPIO_PORT, LTDC_CLK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC_CLKÒý½ÅÄ£Ê½ÉèÖÃ */

    sys_gpio_af_set(LTDC_DE_GPIO_PORT, LTDC_DE_GPIO_PIN, 14);       /* LTDC_DE½Å, AF14 */
    sys_gpio_af_set(LTDC_VSYNC_GPIO_PORT, LTDC_VSYNC_GPIO_PIN, 14); /* LTDC_VSYNC½Å, AF14 */
    sys_gpio_af_set(LTDC_HSYNC_GPIO_PORT, LTDC_HSYNC_GPIO_PIN, 14); /* LTDC_HSYNC½Å, AF14 */
    sys_gpio_af_set(LTDC_CLK_GPIO_PORT, LTDC_CLK_GPIO_PIN, 14);     /* LTDC_CLK½Å, AF14 */

    /* ÒÔÏÂÊÇLTDC Êý¾ÝÒý½ÅµÄÅäÖÃ */


    sys_gpio_set(GPIOG, SYS_GPIO_PIN6 | SYS_GPIO_PIN11,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC Êý¾ÝÒý½ÅÄ£Ê½ÉèÖÃ */

    sys_gpio_set(GPIOH, 0X7F << 9,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC Êý¾ÝÒý½ÅÄ£Ê½ÉèÖÃ */

    sys_gpio_set(GPIOI, 7 << 0 | 0XF << 4,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LTDC Êý¾ÝÒý½ÅÄ£Ê½ÉèÖÃ */

    sys_gpio_af_set(GPIOG, SYS_GPIO_PIN6 | SYS_GPIO_PIN11, 14);     /* LTDC Êý¾Ý½Å, AF14 */
    sys_gpio_af_set(GPIOH, 0X7F << 9, 14);                          /* LTDC Êý¾Ý½Å, AF14 */
    sys_gpio_af_set(GPIOI, 7 << 0 | 0XF << 4, 14);                  /* LTDC Êý¾Ý½Å, AF14 */

#endif

    RCC->APB3ENR |= 1 << 3;             /* ¿ªÆôLTDCÊ±ÖÓ */
    RCC->AHB4ENR |= 0X7 << 6;           /* Ê¹ÄÜPG/PH/PIÊ±ÖÓ */
    LTDC_BL(1);
#if 0
    if (lcdid == 0X4342)
    {
        lcdltdc.pwidth = 480;       /* Ãæ°å¿í¶È,µ¥Î»:ÏñËØ */
        lcdltdc.pheight = 272;      /* Ãæ°å¸ß¶È,µ¥Î»:ÏñËØ */
        lcdltdc.hsw = 1;            /* Ë®Æ½Í¬²½¿í¶È */
        lcdltdc.vsw = 1;            /* ´¹Ö±Í¬²½¿í¶È */
        lcdltdc.hbp = 40;           /* Ë®Æ½ºóÀÈ */
        lcdltdc.vbp = 8;            /* ´¹Ö±ºóÀÈ */
        lcdltdc.hfp = 5;            /* Ë®Æ½Ç°ÀÈ */
        lcdltdc.vfp = 8;            /* ´¹Ö±Ç°ÀÈ */
        ltdc_clk_set(300, 25, 33);  /* ÉèÖÃÏñËØÊ±ÖÓ  9Mhz */
    }
    else if (lcdid == 0X7084)
    {
        lcdltdc.pwidth = 800;       /* Ãæ°å¿í¶È,µ¥Î»:ÏñËØ */
        lcdltdc.pheight = 480;      /* Ãæ°å¸ß¶È,µ¥Î»:ÏñËØ */
        lcdltdc.hsw = 1;            /* Ë®Æ½Í¬²½¿í¶È */
        lcdltdc.vsw = 1;            /* ´¹Ö±Í¬²½¿í¶È */
        lcdltdc.hbp = 46;           /* Ë®Æ½ºóÀÈ */
        lcdltdc.vbp = 23;           /* ´¹Ö±ºóÀÈ */
        lcdltdc.hfp = 210;          /* Ë®Æ½Ç°ÀÈ */
        lcdltdc.vfp = 22;           /* ´¹Ö±Ç°ÀÈ */
        ltdc_clk_set(300, 25, 9);   /* ÉèÖÃÏñËØÊ±ÖÓ 33M(Èç¹û¿ªË«ÏÔ,ÐèÒª½µµÍDCLKµ½:18.75Mhz  300/4/4,²Å»á±È½ÏºÃ) */
    }
    else if (lcdid == 0X7016)
    {
        lcdltdc.pwidth = 1024;      /* Ãæ°å¿í¶È,µ¥Î»:ÏñËØ */
        lcdltdc.pheight = 600;      /* Ãæ°å¸ß¶È,µ¥Î»:ÏñËØ */
        lcdltdc.hsw = 20;           /* Ë®Æ½Í¬²½¿í¶È */
        lcdltdc.vsw = 3;            /* ´¹Ö±Í¬²½¿í¶È */
        lcdltdc.hbp = 140;          /* Ë®Æ½ºóÀÈ */
        lcdltdc.vbp = 20;           /* ´¹Ö±ºóÀÈ */
        lcdltdc.hfp = 160;          /* Ë®Æ½Ç°ÀÈ */
        lcdltdc.vfp = 12;           /* ´¹Ö±Ç°ÀÈ */
        ltdc_clk_set(300, 25, 6);   /* ÉèÖÃÏñËØÊ±ÖÓ  50Mhz */
    }
    else if (lcdid == 0X7018)
    {
        lcdltdc.pwidth = 1280;      /* Ãæ°å¿í¶È,µ¥Î»:ÏñËØ */
        lcdltdc.pheight = 800;      /* Ãæ°å¸ß¶È,µ¥Î»:ÏñËØ */
        /* ÆäËû²ÎÊý´ý¶¨ */
    }
    else if (lcdid == 0X4384)
    {
        lcdltdc.pwidth = 800;       /* Ãæ°å¿í¶È,µ¥Î»:ÏñËØ */
        lcdltdc.pheight = 480;      /* Ãæ°å¸ß¶È,µ¥Î»:ÏñËØ */
        lcdltdc.hbp = 88;           /* Ë®Æ½ºóÀÈ */
        lcdltdc.hfp = 40;           /* Ë®Æ½Ç°ÀÈ */
        lcdltdc.hsw = 48;           /* Ë®Æ½Í¬²½¿í¶È */
        lcdltdc.vbp = 32;           /* ´¹Ö±ºóÀÈ */
        lcdltdc.vfp = 13;           /* ´¹Ö±Ç°ÀÈ */
        lcdltdc.vsw = 3;            /* ´¹Ö±Í¬²½¿í¶È */
        //ltdc_clk_set(300, 25, 9);   /* ÉèÖÃÏñËØÊ±ÖÓ 33M */
    }
    else if (lcdid == 0X1018)       /* 10.1´ç1280*800 RGBÆÁ */
    {
        lcdltdc.pwidth = 1280;      /* Ãæ°å¿í¶È,µ¥Î»:ÏñËØ */
        lcdltdc.pheight = 800;      /* Ãæ°å¸ß¶È,µ¥Î»:ÏñËØ */
        lcdltdc.hbp = 140;          /* Ë®Æ½ºóÀÈ */
        lcdltdc.hfp = 10;           /* Ë®Æ½Ç°ÀÈ */
        lcdltdc.hsw = 10;           /* Ë®Æ½Í¬²½¿í¶È */
        lcdltdc.vbp = 10;           /* ´¹Ö±ºóÀÈ */
        lcdltdc.vfp = 10;           /* ´¹Ö±Ç°ÀÈ */
        lcdltdc.vsw = 3;            /* ´¹Ö±Í¬²½¿í¶È */
        ltdc_clk_set(300, 25, 6);   /* ÉèÖÃÏñËØÊ±ÖÓ  50Mhz */
    }

    if (lcdid == 0X1018)
    {
        tempreg = 1 << 28;          /* ÏñËØÊ±ÖÓ¼«ÐÔ:·´Ïò */
    }else
    {
        tempreg = 0 << 28;          /* ÏñËØÊ±ÖÓ¼«ÐÔ:²»·´Ïò */
    }

    tempreg |= 0 << 29;             /* Êý¾ÝÊ¹ÄÜ¼«ÐÔ:µÍµçÆ½ÓÐÐ§ */
    tempreg |= 0 << 30;             /* ´¹Ö±Í¬²½¼«ÐÔ:µÍµçÆ½ÓÐÐ§ */
    tempreg |= 0 << 31;             /* Ë®Æ½Í¬²½¼«ÐÔ:µÍµçÆ½ÓÐÐ§ */
    LTDC->GCR = tempreg;            /* ÉèÖÃÈ«¾Ö¿ØÖÆ¼Ä´æÆ÷ */
    tempreg = (lcdltdc.vsw - 1) << 0;   /* ´¹Ö±Âö¿í-1 */
    tempreg |= (lcdltdc.hsw - 1) << 16; /* Ë®Æ½Âö¿í-1 */
    LTDC->SSCR = tempreg;               /* ÉèÖÃÍ¬²½´óÐ¡ÅäÖÃ¼Ä´æÆ÷ */

    tempreg = (lcdltdc.vsw + lcdltdc.vbp - 1) << 0;     /* ÀÛ¼Ó´¹Ö±ºóÑØ=´¹Ö±Âö¿í+´¹Ö±ºóÑØ-1 */
    tempreg |= (lcdltdc.hsw + lcdltdc.hbp - 1) << 16;   /* ÀÛ¼ÓË®Æ½ºóÑØ=Ë®Æ½Âö¿í+Ë®Æ½ºóÑØ-1 */
    LTDC->BPCR = tempreg;                               /* ÉèÖÃºóÑØÅäÖÃ¼Ä´æÆ÷ */

    tempreg = (lcdltdc.vsw + lcdltdc.vbp + lcdltdc.pheight - 1) << 0;   /* ÀÛ¼ÓÓÐÐ§¸ß¶È=´¹Ö±Âö¿í+´¹Ö±ºóÑØ+´¹Ö±·Ö±æÂÊ-1 */
    tempreg |= (lcdltdc.hsw + lcdltdc.hbp + lcdltdc.pwidth - 1) << 16;  /* ÀÛ¼ÓÓÐÐ§¿í¶È=Ë®Æ½Âö¿í+Ë®Æ½ºóÑØ+Ë®Æ½·Ö±æÂÊ-1 */
    LTDC->AWCR = tempreg;                                               /* ÉèÖÃÓÐÐ§¿í¶ÈÅäÖÃ¼Ä´æÆ÷ */

    tempreg = (lcdltdc.vsw + lcdltdc.vbp + lcdltdc.pheight + lcdltdc.vfp - 1) << 0;     /* ×Ü¸ß¶È=´¹Ö±Âö¿í+´¹Ö±ºóÑØ+´¹Ö±·Ö±æÂÊ+´¹Ö±Ç°ÀÈ-1 */
    tempreg |= (lcdltdc.hsw + lcdltdc.hbp + lcdltdc.pwidth + lcdltdc.hfp - 1) << 16;    /* ×Ü¿í¶È=Ë®Æ½Âö¿í+Ë®Æ½ºóÑØ+Ë®Æ½·Ö±æÂÊ+Ë®Æ½Ç°ÀÈ-1 */
    LTDC->TWCR = tempreg;   /* ÉèÖÃ×Ü¿í¶ÈÅäÖÃ¼Ä´æÆ÷ */

    LTDC->BCCR = LTDC_BACKLAYERCOLOR;   /* ÉèÖÃ±³¾°²ãÑÕÉ«¼Ä´æÆ÷(RGB888¸ñÊ½) */
    ltdc_switch(1);         /* ¿ªÆôLTDC */


    #endif

	LTDC->GCR = 0;
	LTDC->SSCR = (VSW-1)|((HSW-1)<<16);
	LTDC->BPCR = (VSW+VBP-1)|((HSW+HBP-1)<<16);
	LTDC->AWCR = (VSW+VBP+HEIGHT-1)|((HSW+HBP+WIDTH-1)<<16);
	LTDC->TWCR = (VSW+VBP+HEIGHT+VFP-1)|((HSW+HBP+WIDTH+HFP-1)<<16);
	LTDC->BCCR = 0x00000000;
	LTDC->GCR |= 1<<0;

	LTDC_Layer1->CFBAR = layer1_buf;
	LTDC_Layer1->PFCR = 0;						//ARGB
	LTDC_Layer1->CACR = 255;
	LTDC_Layer1->DCCR = 0;
	LTDC_Layer1->BFCR = (0b110<<8)|(0b111);

	LTDC_Layer1->WHPCR = (((LTDC->BPCR & 0x0FFF0000) >> 16) + 1)|((((LTDC->BPCR & 0x0FFF0000) >> 16) + 1 + WIDTH)<<16);
	LTDC_Layer1->WVPCR = ((LTDC->BPCR & 0X7FF) + 1)|(((LTDC->BPCR & 0X7FF) + 1 + HEIGHT)<<16);
	LTDC_Layer1->CFBLR = (WIDTH * 4 << 16) | (WIDTH * 4 + 7);
	LTDC_Layer1->CFBLNR = HEIGHT;
	LTDC_Layer1->CR = 1;
	LTDC->SRCR |= 1 << 0;


#if 0

#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
    g_ltdc_framebuf[0] = (uint32_t *)ltdc_lcd_framebuf;
    lcdltdc.pixsize = 4;    /* Ã¿¸öÏñËØÕ¼4¸ö×Ö½Ú */
#else
    g_ltdc_framebuf[0] = (uint32_t *)ltdc_lcd_framebuf;
    //g_ltdc_framebuf[1]=(uint32_t*)&ltdc_lcd_framebuf1;
    lcdltdc.pixsize = 2;    /* Ã¿¸öÏñËØÕ¼2¸ö×Ö½Ú */
#endif

    /* ²ãÅäÖÃ */
    ltdc_layer_parameter_config(0, (uint32_t)g_ltdc_framebuf[0], LTDC_PIXFORMAT, 255, 0, 6, 7, 0X000000);   /* ²ã²ÎÊýÅäÖÃ */
    ltdc_layer_window_config(0, 0, 0, lcdltdc.pwidth, lcdltdc.pheight);                                     /* ²ã´°¿ÚÅäÖÃ,ÒÔLCDÃæ°å×ø±êÏµÎª»ù×¼,²»ÒªËæ±ãÐÞ¸Ä! */

    //ltdc_layer_parameter_config(1,(uint32_t)g_ltdc_framebuf[1],LTDC_PIXFORMAT,127,0,6,7,0X000000);        /* ²ã²ÎÊýÅäÖÃ */
    //ltdc_layer_window_config(1,0,0,lcdltdc.pwidth,lcdltdc.pheight);                                       /* ²ã´°¿ÚÅäÖÃ,ÒÔLCDÃæ°å×ø±êÏµÎª»ù×¼,²»ÒªËæ±ãÐÞ¸Ä! */


    //ltdc_display_dir(0);              /* Ä¬ÈÏÊúÆÁ£¬ÔÚLCD_Initº¯ÊýÀïÃæÉèÖÃ */
    //ltdc_select_layer(0);               /* Ñ¡ÔñµÚ1²ã */
                             /* µãÁÁ±³¹â */
    //ltdc_clear(0XFFFFFFFF);             /* ÇåÆÁ */
#endif



}






