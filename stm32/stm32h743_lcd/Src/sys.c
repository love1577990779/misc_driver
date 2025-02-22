/**
 ****************************************************************************************************
 * @file        sys.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-16
 * @brief       ϵͳ��ʼ������(����ʱ������/�жϹ���/GPIO���õ�)
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20230316
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "sys.h"


/**
 * @brief       �����ж�������ƫ�Ƶ�ַ
 * @param       baseaddr: ��ַ
 * @param       offset: ƫ����
 * @retval      ��
 */
void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset)
{
    /* ����NVIC��������ƫ�ƼĴ���,VTOR��9λ����,��[8:0]���� */
    SCB->VTOR = baseaddr | (offset & (uint32_t)0xFFFFFE00);
}

/**
 * @brief       ����NVIC����
 * @param       group: 0~4,��5��, ��ϸ���ͼ�: sys_nvic_init��������˵��
 * @retval      ��
 */
static void sys_nvic_priority_group_config(uint8_t group)
{
    uint32_t temp, temp1;
    temp1 = (~group) & 0x07;/* ȡ����λ */
    temp1 <<= 8;
    temp = SCB->AIRCR;      /* ��ȡ��ǰ������ */
    temp &= 0X0000F8FF;     /* �����ǰ���� */
    temp |= 0X05FA0000;     /* д��Կ�� */
    temp |= temp1;
    SCB->AIRCR = temp;      /* ���÷��� */
}

/**
 * @brief       ����NVIC(��������/��ռ���ȼ�/�����ȼ���)
 * @param       pprio: ��ռ���ȼ�(PreemptionPriority)
 * @param       sprio: �����ȼ�(SubPriority)
 * @param       ch: �жϱ��(Channel)
 * @param       group: �жϷ���
 *   @arg       0, ��0: 0λ��ռ���ȼ�, 4λ�����ȼ�
 *   @arg       1, ��1: 1λ��ռ���ȼ�, 3λ�����ȼ�
 *   @arg       2, ��2: 2λ��ռ���ȼ�, 2λ�����ȼ�
 *   @arg       3, ��3: 3λ��ռ���ȼ�, 1λ�����ȼ�
 *   @arg       4, ��4: 4λ��ռ���ȼ�, 0λ�����ȼ�
 * @note        ע�����ȼ����ܳ����趨����ķ�Χ! ����������벻���Ĵ���
 * @retval      ��
 */
void sys_nvic_init(uint8_t pprio, uint8_t sprio, uint8_t ch, uint8_t group)
{
    uint32_t temp;
    sys_nvic_priority_group_config(group);  /* ���÷��� */
    temp = pprio << (4 - group);
    temp |= sprio & (0x0f >> group);
    temp &= 0xf;                            /* ȡ����λ */
    NVIC->ISER[ch / 32] |= 1 << (ch % 32);  /* ʹ���ж�λ(Ҫ����Ļ�,����ICER��ӦλΪ1����) */
    NVIC->IP[ch] |= temp << 4;              /* ������Ӧ���ȼ����������ȼ� */
}

/**
 * @brief       �ⲿ�ж����ú���, ֻ���GPIOA~GPIOK
 * @note        �ú������Զ�������Ӧ�ж�, �Լ�������
 * @param       p_gpiox: GPIOA~GPIOK, GPIOָ��
 * @param       pinx: 0X0000~0XFFFF, ����λ��, ÿ��λ����һ��IO, ��0λ����Px0, ��1λ����Px1, ��������. ����0X0101, ����ͬʱ����Px0��Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 * @param       tmode: 1~3, ����ģʽ
 *   @arg       SYS_GPIO_FTIR, 1, �½��ش���
 *   @arg       SYS_GPIO_RTIR, 2, �����ش���
 *   @arg       SYS_GPIO_BTIR, 3, �����ƽ����
 * @retval      ��
 */
void sys_nvic_ex_config(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t tmode)
{
    uint8_t offset;
    uint32_t gpio_num = 0;      /* gpio���, 0~10, ����GPIOA~GPIOK */
    uint32_t pinpos = 0, pos = 0, curpin = 0;

    gpio_num = ((uint32_t)p_gpiox - (uint32_t)GPIOA) / 0X400 ;/* �õ�gpio��� */
    RCC->APB4ENR |= 1 << 1;     /* SYSCFGEN = 1,ʹ��SYSCFGʱ�� */

    for (pinpos = 0; pinpos < 16; pinpos++)
    {
        pos = 1 << pinpos;      /* һ����λ��� */
        curpin = pinx & pos;    /* ��������Ƿ�Ҫ���� */

        if (curpin == pos)      /* ��Ҫ���� */
        {
            offset = (pinpos % 4) * 4;
            SYSCFG->EXTICR[pinpos / 4] &= ~(0x000F << offset);  /* ���ԭ�����ã����� */
            SYSCFG->EXTICR[pinpos / 4] |= gpio_num << offset;   /* EXTI.BITxӳ�䵽gpiox.bitx */

            EXTI_D1->IMR1 |= 1 << pinpos;   /* ����line BITx�ϵ��ж�(���Ҫ��ֹ�жϣ��򷴲�������) */

            if (tmode & 0x01) EXTI->FTSR1 |= 1 << pinpos;       /* line bitx���¼��½��ش��� */
            if (tmode & 0x02) EXTI->RTSR1 |= 1 << pinpos;       /* line bitx���¼��������ش��� */
        }
    }
}

/**
 * @brief       GPIO���ù���ѡ������
 * @param       p_gpiox: GPIOA~GPIOK, GPIOָ��
 * @param       pinx: 0X0000~0XFFFF, ����λ��, ÿ��λ����һ��IO, ��0λ����Px0, ��1λ����Px1, ��������. ����0X0101, ����ͬʱ����Px0��Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 * @param       afx:0~15, ����AF0~AF15.
 *              AF0~15�������(��������г����õ�, ��ϸ�����STM32H743xx�����ֲ�, Table 9~19):
 *   @arg       AF0: MCO/SWD/SWCLK/RTC;        AF1: TIM1/2/TIM16/17/LPTIM1;     AF2: TIM3~5/TIM12/HRTIM1/SAI1;   AF3: TIM8/LPTIM2~5/HRTIM1/LPUART1;
 *   @arg       AF4: I2C1~I2C4/TIM15/USART1;   AF5: SPI1~SPI6/CEC;              AF6: SPI3/SAI1~3/UART4/I2C4;     AF7: SPI2/3/6/USART1~3/6/UART7/SDIO1;
 *   @arg       AF8: USART4/5/8/SPDIF/SAI2/4;  AF9; FDCAN1~2/TIM13/14/LCD/QSPI; AF10: USB_OTG1/2/SAI2/4/QSPI;    AF11: ETH/UART7/SDIO2/I2C4/COMP1/2;
 *   @arg       AF12: FMC/SDIO1/OTG2/LCD;      AF13: DCIM/DSI/LCD/COMP1/2;      AF14: LCD/UART5;                 AF15: EVENTOUT;
 * @retval      ��
 */
void sys_gpio_af_set(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t afx)
{
    uint32_t pinpos = 0, pos = 0, curpin = 0;;

    for (pinpos = 0; pinpos < 16; pinpos++)
    {
        pos = 1 << pinpos;      /* һ����λ��� */
        curpin = pinx & pos;    /* ��������Ƿ�Ҫ���� */

        if (curpin == pos)      /* ��Ҫ���� */
        {
            p_gpiox->AFR[pinpos >> 3] &= ~(0X0F << ((pinpos & 0X07) * 4));
            p_gpiox->AFR[pinpos >> 3] |= (uint32_t)afx << ((pinpos & 0X07) * 4);
        }
    }
}

/**
 * @brief       GPIOͨ������
 * @param       p_gpiox: GPIOA~GPIOK, GPIOָ��
 * @param       pinx: 0X0000~0XFFFF, ����λ��, ÿ��λ����һ��IO, ��0λ����Px0, ��1λ����Px1, ��������. ����0X0101, ����ͬʱ����Px0��Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 *
 * @param       mode: 0~3; ģʽѡ��, ��������:
 *   @arg       SYS_GPIO_MODE_IN,  0, ����ģʽ(ϵͳ��λĬ��״̬)
 *   @arg       SYS_GPIO_MODE_OUT, 1, ���ģʽ
 *   @arg       SYS_GPIO_MODE_AF,  2, ���ù���ģʽ
 *   @arg       SYS_GPIO_MODE_AIN, 3, ģ������ģʽ
 *
 * @param       otype: 0 / 1; �������ѡ��, ��������:
 *   @arg       SYS_GPIO_OTYPE_PP, 0, �������
 *   @arg       SYS_GPIO_OTYPE_OD, 1, ��©���
 *
 * @param       ospeed: 0~3; ����ٶ�, ��������:
 *   @arg       SYS_GPIO_SPEED_LOW,  0, ����
 *   @arg       SYS_GPIO_SPEED_MID,  1, ����
 *   @arg       SYS_GPIO_SPEED_FAST, 2, ����
 *   @arg       SYS_GPIO_SPEED_HIGH, 3, ����
 *
 * @param       pupd: 0~3: ����������, ��������:
 *   @arg       SYS_GPIO_PUPD_NONE, 0, ����������
 *   @arg       SYS_GPIO_PUPD_PU,   1, ����
 *   @arg       SYS_GPIO_PUPD_PD,   2, ����
 *   @arg       SYS_GPIO_PUPD_RES,  3, ����
 *
 * @note:       ע��: ������ģʽ(��ͨ����/ģ������)��, OTYPE��OSPEED������Ч!!
 * @retval      ��
 */
void sys_gpio_set(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint32_t mode, uint32_t otype, uint32_t ospeed, uint32_t pupd)
{
    uint32_t pinpos = 0, pos = 0, curpin = 0;

    for (pinpos = 0; pinpos < 16; pinpos++)
    {
        pos = 1 << pinpos;      /* һ����λ��� */
        curpin = pinx & pos;    /* ��������Ƿ�Ҫ���� */

        if (curpin == pos)      /* ��Ҫ���� */
        {
            p_gpiox->MODER &= ~(3 << (pinpos * 2)); /* �����ԭ�������� */
            p_gpiox->MODER |= mode << (pinpos * 2); /* �����µ�ģʽ */

            if ((mode == 0X01) || (mode == 0X02))   /* ��������ģʽ/���ù���ģʽ */
            {
                p_gpiox->OSPEEDR &= ~(3 << (pinpos * 2));       /* ���ԭ�������� */
                p_gpiox->OSPEEDR |= (ospeed << (pinpos * 2));   /* �����µ��ٶ�ֵ */
                p_gpiox->OTYPER &= ~(1 << pinpos) ;             /* ���ԭ�������� */
                p_gpiox->OTYPER |= otype << pinpos;             /* �����µ����ģʽ */
            }

            p_gpiox->PUPDR &= ~(3 << (pinpos * 2)); /* �����ԭ�������� */
            p_gpiox->PUPDR |= pupd << (pinpos * 2); /* �����µ������� */
        }
    }
}

/**
 * @brief       ����GPIOĳ�����ŵ����״̬
 * @param       p_gpiox: GPIOA~GPIOK, GPIOָ��
 * @param       0X0000~0XFFFF, ����λ��, ÿ��λ����һ��IO, ��0λ����Px0, ��1λ����Px1, ��������. ����0X0101, ����ͬʱ����Px0��Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 * @param       status: 0/1, ����״̬(�����λ��Ч), ��������:
 *   @arg       0, ����͵�ƽ
 *   @arg       1, ����ߵ�ƽ
 * @retval      ��
 */
void sys_gpio_pin_set(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t status)
{
    if (status & 0X01)
    {
        p_gpiox->BSRR |= pinx;              /* ����GPIOx��pinxΪ1 */
    }
    else
    {
        p_gpiox->BSRR |= (uint32_t)pinx << 16;   /* ����GPIOx��pinxΪ0 */
    }
}

/**
 * @brief       ��ȡGPIOĳ�����ŵ�״̬
 * @param       p_gpiox: GPIOA~GPIOK, GPIOָ��
 * @param       0X0000~0XFFFF, ����λ��, ÿ��λ����һ��IO, ��0λ����Px0, ��1λ����Px1, ��������. ����0X0101, ����ͬʱ����Px0��Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 * @retval      ��������״̬, 0, �͵�ƽ; 1, �ߵ�ƽ
 */
uint8_t sys_gpio_pin_get(GPIO_TypeDef *p_gpiox, uint16_t pinx)
{
    if (p_gpiox->IDR & pinx)
    {
        return 1;   /* pinx��״̬Ϊ1 */
    }
    else
    {
        return 0;   /* pinx��״̬Ϊ0 */
    }
}

/**
 * @brief       ִ��: WFIָ��(ִ�����ָ�����͹���״̬, �ȴ��жϻ���)
 * @param       ��
 * @retval      ��
 */
void sys_wfi_set(void)
{
    __ASM volatile("wfi");
}

/**
 * @brief       �ر������ж�(���ǲ�����fault��NMI�ж�)
 * @param       ��
 * @retval      ��
 */
void sys_intx_disable(void)
{
    __ASM volatile("cpsid i");
}

/**
 * @brief       ���������ж�
 * @param       ��
 * @retval      ��
 */
void sys_intx_enable(void)
{
    __ASM volatile("cpsie i");
}

/**
 * @brief       ����ջ����ַ
 * @note        ���ĺ�X, ����MDK��, ʵ����û�����
 * @param       addr: ջ����ַ
 * @retval      ��
 */
void sys_msr_msp(uint32_t addr)
{
    __set_MSP(addr);    /* ����ջ����ַ */
}

/**
 * @brief       �������ģʽ
 * @param       ��
 * @retval      ��
 */
void sys_standby(void)
{
    PWR->WKUPEPR &= ~(1 << 0);  /* WKUPEN0 = 0, PA0������WKUP���� */
    PWR->WKUPEPR |= 1 << 0;     /* WKUPEN0 = 1, PA0����WKUP���� */
    PWR->WKUPEPR &= ~(1 << 8);  /* WKUPP0 = 0, PA0�ߵ�ƽ����(������) */
    PWR->WKUPEPR &= ~(3 << 16); /* ���WKUPPUPDԭ�������� */
    PWR->WKUPEPR |= 2 << 16;    /* WKUPPUPD = 10, PA0���� */
    PWR->WKUPCR |= 0X3F << 0;   /* ���WKUP0~5���ѱ�־ */
    PWR->CPUCR |= 7 << 0;       /* PDDS_D1/D2/D3 = 1, ����D1/D2/D3�������˯��ģʽ(PDDS) */
    SCB->SCR |= 1 << 2;         /* ʹ��SLEEPDEEPλ (SYS->CTRL) */
    sys_wfi_set();              /* ִ��WFIָ��, �������ģʽ */
}

/**
 * @brief       ϵͳ��λ
 * @param       ��
 * @retval      ��
 */
void sys_soft_reset(void)
{
    SCB->AIRCR = 0X05FA0000 | (uint32_t)0x04;
}

/**
 * @brief       ʹ��STM32H7��L1-Cache, ͬʱ����D cache��ǿ��͸д
 * @param       ��
 * @retval      ��
 */
void sys_cache_enable(void)
{
    SCB_EnableICache(); /* ʹ��I-Cache,������core_cm7.h���涨�� */
    SCB_EnableDCache(); /* ʹ��D-Cache,������core_cm7.h���涨�� */
    SCB->CACR |= 1 << 2;/* ǿ��D-Cache͸д,�粻����͸д,ʵ��ʹ���п��������������� */
}

/**
 * @brief       ʱ�����ú���
 * @param       plln: PLL1��Ƶϵ��(PLL��Ƶ), ȡֵ��Χ: 4~512.
 * @param       pllm: PLL1Ԥ��Ƶϵ��(��PLL֮ǰ�ķ�Ƶ), ȡֵ��Χ: 2~63.
 * @param       pllp: PLL1��p��Ƶϵ��(PLL֮��ķ�Ƶ), ��Ƶ����Ϊϵͳʱ��, ȡֵ��Χ: 2~128.(�ұ�����2�ı���)
 * @param       pllq: PLL1��q��Ƶϵ��(PLL֮��ķ�Ƶ), ȡֵ��Χ: 1~128.
 * @note
 *
 *              Fvco: VCOƵ��
 *              Fsys: ϵͳʱ��Ƶ��, Ҳ��PLL1��p��Ƶ���ʱ��Ƶ��
 *              Fq:   PLL1��q��Ƶ���ʱ��Ƶ��
 *              Fs:   PLL����ʱ��Ƶ��, ������HSI, CSI, HSE��.
 *              Fvco = Fs * (plln / pllm);
 *              Fsys = Fvco / pllp = Fs * (plln / (pllm * pllp));
 *              Fq   = Fvco / pllq = Fs * (plln / (pllm * pllq));
 *
 *              �ⲿ����Ϊ25M��ʱ��, �Ƽ�ֵ: plln = 160, pllm = 5, pllp = 2, pllq = 4.
 *              �õ�:Fvco = 25 * (160 / 5) = 800Mhz
 *                   Fsys = pll1_p_ck = 800 / 2 = 400Mhz
 *                   Fq   = pll1_q_ck = 800 / 4 = 200Mhz
 *
 *              H743Ĭ����Ҫ���õ�Ƶ������:
 *              CPUƵ��(rcc_c_ck) = sys_d1cpre_ck = 400Mhz
 *              rcc_aclk = rcc_hclk3 = 200Mhz
 *              AHB1/2/3/4(rcc_hclk1/2/3/4) = 200Mhz
 *              APB1/2/3/4(rcc_pclk1/2/3/4) = 100Mhz
 *              pll2_p_ck = (25 / 25) * 440 / 2) = 220Mhz
 *              pll2_r_ck = FMCʱ��Ƶ�� = ((25 / 25) * 440 / 2) = 220Mhz
 *
 * @retval      �������: 0, �ɹ�; 1, HSE����; 2, PLL1����; 3, PLL2����; 4, �л�ʱ�Ӵ���;
 */
uint8_t sys_clock_set(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    uint32_t retry = 0;
    uint8_t retval = 0;
    uint8_t swsval = 0;
    
    
    PWR->CR3 &= ~(1 << 2);      /* SCUEN = 0, ����LDOEN��BYPASSλ������ */
    PWR->D3CR |= 3 << 14;       /* VOS = 3, Scale1, 1.2V�ں˵�ѹ,FLASH���ʿ��Եõ�������� */

    while ((PWR->D3CR & (1 << 13)) == 0);   /* �ȴ���ѹ�ȶ� */

    RCC->CR |= 1 << 16; /* HSEON = 1, ����HSE */

    while (((RCC->CR & (1 << 17)) == 0) && (retry < 0X7FFF))
    {
        retry++;        /* �ȴ�HSE RDY */
    }

    if (retry == 0X7FFF)
    {
        retval = 1;     /* HSE�޷����� */
    }
    else
    {
        RCC->PLLCKSELR |= 2 << 0;           /* PLLSRC[1:0] = 2, ѡ��HSE��ΪPLL������ʱ��Դ */
        RCC->PLLCKSELR |= pllm << 4;        /* DIVM1[5:0] = pllm, ����PLL1��Ԥ��Ƶϵ�� */
        RCC->PLL1DIVR |= (plln - 1) << 0;   /* DIVN1[8:0] = plln - 1, ����PLL1�ı�Ƶϵ��, ����ֵ���1 */
        RCC->PLL1DIVR |= (pllp - 1) << 9;   /* DIVP1[6:0] = pllp - 1, ����PLL1��p��Ƶϵ��, ����ֵ���1 */
        RCC->PLL1DIVR |= (pllq - 1) << 16;  /* DIVQ1[6:0] = pllq - 1, ����PLL1��q��Ƶϵ��, ����ֵ���1 */
        RCC->PLL1DIVR |= 1 << 24;           /* DIVR1[6:0] = pllr - 1, ����PLL1��r��Ƶϵ��, ����ֵ���1, r��Ƶ������ʱ��û�õ� */
        RCC->PLLCFGR |= 2 << 2;             /* PLL1RGE[1:0] = 2, PLL1����ʱ��Ƶ����4~8Mhz֮��(25 / 5 = 5Mhz), ���޸�pllm, ��ȷ�ϴ˲��� */
        RCC->PLLCFGR |= 0 << 1;             /* PLL1VCOSEL = 0, PLL1�е�VCO��Χ, 192~836Mhz(ʵ�ʿ��Ե�960, ������480M��Ƶ����Ҫ��) */
        RCC->PLLCFGR |= 3 << 16;            /* DIVP1EN = 1, DIVQ1EN = 1, ʹ��pll1_p_ck��pll1_q_ck */
        RCC->CR |= 1 << 24;                 /* PLL1ON = 1, ʹ��PLL1 */
        retry = 0;

        while ((RCC->CR & (1 << 25)) == 0)   /* PLL1RDY = 1?, �ȴ�PLL1׼���� */
        {
            retry++;

            if (retry > 0X1FFFFF)
            {
                retval = 2; /* PLL1�޷����� */
                break;
            }
        }

        /* ����PLL2��R��Ƶ���, Ϊ220Mhz, ������TFTLCDʱ��, �ɵõ�110M��fmc_ker_ckʱ��Ƶ�� */
        RCC->PLLCKSELR |= 25 << 12;         /* DIVM2[5:0] = 25, ����PLL2��Ԥ��Ƶϵ�� */
        RCC->PLL2DIVR |= (440 - 1) << 0;    /* DIVN2[8:0] = 440 - 1, ����PLL2�ı�Ƶϵ��, ����ֵ���1 */
        RCC->PLL2DIVR |= (2 - 1) << 9;      /* DIVP2[6:0] = 2 - 1, ����PLL2��p��Ƶϵ��, ����ֵ���1 */
        RCC->PLL2DIVR |= (2 - 1) << 24;     /* DIVR2[6:0] = 2 - 1, ����PLL2��r��Ƶϵ��, ����ֵ���1 */
        RCC->PLLCFGR |= 0 << 6;             /* PLL2RGE[1:0] = 0, PLL2����ʱ��Ƶ����1~2Mhz֮��(25/25 = 1Mhz) */
        RCC->PLLCFGR |= 0 << 5;             /* PLL2VCOSEL = 0, PLL2���VCO��Χ, 192~836Mhz */
        RCC->PLLCFGR |= 1 << 19;            /* DIVP2EN = 1, ʹ��pll2_p_ck */
        RCC->PLLCFGR |= 1 << 21;            /* DIVR2EN = 1, ʹ��pll2_r_ck */
        RCC->D1CCIPR &= ~(3 << 0);          /* ���FMCSEL[1:0]ԭ�������� */
        RCC->D1CCIPR |= 2 << 0;             /* FMCSEL[1:0] = 2, FMCʱ��������pll2_r_ck */
        RCC->CR |= 1 << 26;                 /* PLL2ON = 1, ʹ��PLL2 */
        retry = 0;

        while ((RCC->CR & (1 << 27)) == 0)  /* PLL2RDY = 1?, �ȴ�PLL2׼���� */
        {
            retry++;

            if (retry > 0X1FFFFF)
            {
                retval = 3; /* PLL2�޷����� */
                break;
            }
        }

        RCC->D1CFGR |= 8 << 0;              /* HREF[3:0] = 8, rcc_hclk1/2/3/4  =  sys_d1cpre_ck / 2 = 400 / 2 = 200Mhz, ��AHB1/2/3/4 = 200Mhz */
        RCC->D1CFGR |= 0 << 8;              /* D1CPRE[2:0] = 0, sys_d1cpre_ck = sys_clk/1 = 400 / 1 = 400Mhz, ��CPUʱ�� = 400Mhz */
        RCC->CFGR |= 3 << 0;                /* SW[2:0] = 3, ϵͳʱ��(sys_clk)ѡ������pll1_p_ck, ��400Mhz */
        retry = 0;

        while (swsval != 3)                 /* �ȴ��ɹ���ϵͳʱ��Դ�л�Ϊpll1_p_ck */
        {
            swsval = (RCC->CFGR & (7 << 3)) >> 3;   /* ��ȡSWS[2:0]��״̬, �ж��Ƿ��л��ɹ� */
            retry++;

            if (retry > 0X1FFFFF)
            {
                retval = 4; /* �޷��л�ʱ�� */
                break;
            }
        }

        FLASH->ACR |= 2 << 0;               /* LATENCY[2:0] = 2, 2��CPU�ȴ�����(@VOS1 Level, maxclock = 210Mhz) */
        FLASH->ACR |= 2 << 4;               /* WRHIGHFREQ[1:0] = 2, flash����Ƶ��<285Mhz */
        RCC->D1CFGR |= 4 << 4;              /* D1PPRE[2:0] = 4,  rcc_pclk3 = rcc_hclk3/2 = 100Mhz, ��APB3 = 100Mhz */
        RCC->D2CFGR |= 4 << 4;              /* D2PPRE1[2:0] = 4, rcc_pclk1 = rcc_hclk1/2 = 100Mhz, ��APB1 = 100Mhz */
        RCC->D2CFGR |= 4 << 8;              /* D2PPRE2[2:0] = 4, rcc_pclk2 = rcc_hclk1/2 = 100Mhz, ��APB2 = 100Mhz */
        RCC->D3CFGR |= 4 << 4;              /* D3PPRE[2:0] = 4,  rcc_pclk4 = rcc_hclk4/2 = 100Mhz, ��APB4 = 100Mhz */
        
        RCC->CR |= 1 << 7;                  /* CSION = 1, ʹ��CSI, ΪIO������Ԫ�ṩʱ�� */
        RCC->APB4ENR |= 1 << 1;             /* SYSCFGEN = 1, ʹ��SYSCFGʱ�� */
        SYSCFG->CCCSR |= 1 << 0;            /* EN = 1, ʹ��IO������Ԫ */
    }

    return retval;
}

/**
 * @brief       ϵͳʱ�ӳ�ʼ������
 * @param       plln: PLL1��Ƶϵ��(PLL��Ƶ), ȡֵ��Χ: 4~512.
 * @param       pllm: PLL1Ԥ��Ƶϵ��(��PLL֮ǰ�ķ�Ƶ), ȡֵ��Χ: 2~63.
 * @param       pllp: PLL1��p��Ƶϵ��(PLL֮��ķ�Ƶ), ��Ƶ����Ϊϵͳʱ��, ȡֵ��Χ: 2~128.(�ұ�����2�ı���)
 * @param       pllq: PLL1��q��Ƶϵ��(PLL֮��ķ�Ƶ), ȡֵ��Χ: 1~128.
 * @retval      ��
 */
void sys_stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    RCC->CR = 0x00000001;           /* ����HISON, �����ڲ�����RC�񵴣�����λȫ���� */
    RCC->CFGR = 0x00000000;         /* CFGR���� */
    RCC->D1CFGR = 0x00000000;       /* D1CFGR���� */
    RCC->D2CFGR = 0x00000000;       /* D2CFGR���� */
    RCC->D3CFGR = 0x00000000;       /* D3CFGR���� */
    RCC->PLLCKSELR = 0x00000000;    /* PLLCKSELR���� */
    RCC->PLLCFGR = 0x00000000;      /* PLLCFGR���� */
    RCC->CIER = 0x00000000;         /* CIER����, ��ֹ����RCC����ж� */

    GPV->AXI_TARG7_FN_MOD = 0x00000001;     /* ����AXI SRAM�ľ����ȡ����Ϊ1 */
    
    sys_clock_set(plln, pllm, pllp, pllq);  /* ����ʱ�� */
    sys_cache_enable();                     /* ʹ��L1 Cache */

    /* �����ж�����ƫ�� */
#ifdef  VECT_TAB_RAM
    sys_nvic_set_vector_table(D1_AXISRAM_BASE, 0x0);
#else
    sys_nvic_set_vector_table(FLASH_BANK1_BASE, 0x0);
#endif
}















