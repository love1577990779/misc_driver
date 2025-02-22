#ifndef _LCD_H_
#define _LCD_H_

#define VSW (3)
#define HSW (48)
#define VBP (32)
#define HBP (88)
#define	WIDTH (800)
#define HEIGHT (480)
#define VFP (13)
#define HFP (40)

void init_lcd(void);









/* LCD LTDCÖØÒª²ÎÊý¼¯ */
typedef struct
{
    uint32_t pwidth;        /* LCDÃæ°åµÄ¿í¶È,¹Ì¶¨²ÎÊý,²»ËæÏÔÊ¾·½Ïò¸Ä±ä,Èç¹ûÎª0,ËµÃ÷Ã»ÓÐÈÎºÎRGBÆÁ½ÓÈë */
    uint32_t pheight;       /* LCDÃæ°åµÄ¸ß¶È,¹Ì¶¨²ÎÊý,²»ËæÏÔÊ¾·½Ïò¸Ä±ä */
    uint16_t hsw;           /* Ë®Æ½Í¬²½¿í¶È */
    uint16_t vsw;           /* ´¹Ö±Í¬²½¿í¶È */
    uint16_t hbp;           /* Ë®Æ½ºóÀÈ */
    uint16_t vbp;           /* ´¹Ö±ºóÀÈ */
    uint16_t hfp;           /* Ë®Æ½Ç°ÀÈ */
    uint16_t vfp;           /* ´¹Ö±Ç°ÀÈ */
    uint8_t activelayer;    /* µ±Ç°²ã±àºÅ:0/1 */
    uint8_t dir;            /* 0,ÊúÆÁ;1,ºáÆÁ; */
    uint16_t width;         /* LCD¿í¶È */
    uint16_t height;        /* LCD¸ß¶È */
    uint32_t pixsize;       /* Ã¿¸öÏñËØËùÕ¼×Ö½ÚÊý */
} _ltdc_dev;

extern _ltdc_dev lcdltdc;   /* ¹ÜÀíLCD LTDC²ÎÊý */



#define LTDC_PIXFORMAT_ARGB8888      0X00    /* ARGB8888¸ñÊ½ */
#define LTDC_PIXFORMAT_RGB888        0X01    /* RGB888¸ñÊ½ */
#define LTDC_PIXFORMAT_RGB565        0X02    /* RGB565¸ñÊ½ */
#define LTDC_PIXFORMAT_ARGB1555      0X03    /* ARGB1555¸ñÊ½ */
#define LTDC_PIXFORMAT_ARGB4444      0X04    /* ARGB4444¸ñÊ½ */
#define LTDC_PIXFORMAT_L8            0X05    /* L8¸ñÊ½ */
#define LTDC_PIXFORMAT_AL44          0X06    /* AL44¸ñÊ½ */
#define LTDC_PIXFORMAT_AL88          0X07    /* AL88¸ñÊ½ */

/******************************************************************************************/
/* LTDC_BL/DE/VSYNC/HSYNC/CLK Òý½Å ¶¨Òå
 * LTDC_R3~R7, G2~G7, B3~B7,ÓÉÓÚÒý½ÅÌ«¶à,¾Í²»ÔÚÕâÀï¶¨ÒåÁË,Ö±½ÓÔÚltcd_initÀïÃæÐÞ¸Ä.ËùÒÔÔÚÒÆÖ²µÄÊ±ºò,
 * ³ýÁË¸ÄÕâ5¸öIO¿Ú, »¹µÃ¸Ältcd_initÀïÃæµÄR3~R7, G2~G7, B3~B7ËùÔÚµÄIO¿Ú.
 */

/* BLºÍMCUÆÁ±³¹â½Å¹²ÓÃ ËùÒÔÕâÀï²»ÓÃ¶¨Òå BLÒý½Å */
#define LTDC_BL_GPIO_PORT               GPIOB
#define LTDC_BL_GPIO_PIN                SYS_GPIO_PIN5
#define LTDC_BL_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 1; }while(0)   /* ËùÔÚIO¿ÚÊ±ÖÓÊ¹ÄÜ */

#define LTDC_DE_GPIO_PORT               GPIOF
#define LTDC_DE_GPIO_PIN                SYS_GPIO_PIN10
#define LTDC_DE_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 5; }while(0)   /* ËùÔÚIO¿ÚÊ±ÖÓÊ¹ÄÜ */

#define LTDC_VSYNC_GPIO_PORT            GPIOI
#define LTDC_VSYNC_GPIO_PIN             SYS_GPIO_PIN9
#define LTDC_VSYNC_GPIO_CLK_ENABLE()    do{ RCC->AHB4ENR |= 1 << 8; }while(0)   /* ËùÔÚIO¿ÚÊ±ÖÓÊ¹ÄÜ */

#define LTDC_HSYNC_GPIO_PORT            GPIOI
#define LTDC_HSYNC_GPIO_PIN             SYS_GPIO_PIN10
#define LTDC_HSYNC_GPIO_CLK_ENABLE()    do{ RCC->AHB4ENR |= 1 << 8; }while(0)   /* ËùÔÚIO¿ÚÊ±ÖÓÊ¹ÄÜ */

#define LTDC_CLK_GPIO_PORT              GPIOG
#define LTDC_CLK_GPIO_PIN               SYS_GPIO_PIN7
#define LTDC_CLK_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 6; }while(0)   /* ËùÔÚIO¿ÚÊ±ÖÓÊ¹ÄÜ */



/* ¶¨ÒåÑÕÉ«ÏñËØ¸ñÊ½,Ò»°ãÓÃRGB565 */
#define LTDC_PIXFORMAT              LTDC_PIXFORMAT_ARGB8888

/* ¶¨ÒåÄ¬ÈÏ±³¾°²ãÑÕÉ« */
#define LTDC_BACKLAYERCOLOR         0X00000000

/* LCDÖ¡»º³åÇøÊ×µØÖ·,ÕâÀï¶¨ÒåÔÚSDRAMÀïÃæ */
#define LTDC_FRAME_BUF_ADDR         0XC0000000

/* LTDC±³¹â¿ØÖÆ */
#define LTDC_BL(x)                  sys_gpio_pin_set(LTDC_BL_GPIO_PORT, LTDC_BL_GPIO_PIN, x)

/******************************************************************************************/



void ltdc_switch(uint8_t sw);
void ltdc_layer_switch(uint8_t layerx, uint8_t sw);
void ltdc_select_layer(uint8_t layerx);
void ltdc_display_dir(uint8_t dir);
void ltdc_draw_point(uint16_t x, uint16_t y, uint32_t color);
uint32_t ltdc_read_point(uint16_t x, uint16_t y);
void ltdc_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color);
void ltdc_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);
void ltdc_clear(uint32_t color);
uint8_t ltdc_clk_set(uint32_t pllsain, uint32_t pllsair, uint32_t pllsaidivr);
void ltdc_layer_window_config(uint8_t layerx, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height);
void ltdc_layer_parameter_config(uint8_t layerx, uint32_t bufaddr, uint8_t pixformat, uint8_t alpha, uint8_t alpha0, uint8_t bfac1, uint8_t bfac2, uint32_t bkcolor);
uint16_t ltdc_panelid_read(void);
void ltdc_init(void);
































#endif
