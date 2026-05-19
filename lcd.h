
#ifndef __LCD_H
#define __LCD_H		

#include <msp432p401r.h>
#include "stdlib.h"

typedef  unsigned char  u8;
typedef  unsigned short  u16;
typedef  unsigned int  u32;

//LCD
typedef struct  
{
    u16 width;
    u16 height;
    u16 id;
    u8  dir;
    u16  wramcmd;
    u16  setxcmd;
    u16  setycmd;
  u8   xoffset;    
  u8     yoffset;
  u8 zoomX;
  u8 zoomY;
}_lcd_dev;

//LCD
extern _lcd_dev lcddev;
 
#define USE_HORIZONTAL       1
  
#define LCD_W 240
#define LCD_H 320

//TFTLCD
extern u16  POINT_COLOR;
extern u16  BACK_COLOR;

#define LED      BIT5 // P5.5
#define LCD_RS   BIT5 // P6.5  = DC !
#define LCD_RST  BIT5 // P3.5
#define LCD_CS   BIT4 // P5.4

#define LCD_LED(n) (n?(P5OUT |= LED):(P5OUT &= ~LED))

#define LCD_RS_SET  P6OUT|=LCD_RS
#define LCD_RS_CLR  P6OUT&=~(LCD_RS)

#define LCD_RST_SET P3OUT|=LCD_RST
#define LCD_RST_CLR P3OUT&=~(LCD_RST)

#define WHITE       (uint16_t)~0xFFFF
#define BLACK       (uint16_t)~0x0000
#define BLUE        (uint16_t)~0x001F
#define BRED        (uint16_t)~0XF81F
#define GRED        (uint16_t)~0XFFE0
#define GBLUE       (uint16_t)~0X07FF
#define RED         (uint16_t)~0xF800
#define MAGENTA     (uint16_t)~0xF81F
#define GREEN       (uint16_t)~0x07E0
#define CYAN        (uint16_t)~0x7FFF
#define YELLOW      (uint16_t)~0xFFE0
#define BROWN       (uint16_t)~0XBC40
#define BRRED       (uint16_t)~0XFC07
#define GRAY        (uint16_t)~0X8430
#define GRAY0       (uint16_t)~0xEF7D
#define GRAY1       (uint16_t)~0x8410 // 00000 000000 00000
#define GRAY2       (uint16_t)~0x4208

// GUI
#define DARKBLUE    (uint16_t)~0X01CF  //
#define LIGHTBLUE   (uint16_t)~0X7D7C  //
#define GRAYBLUE    (uint16_t)~0X5458 //

// PANEL
#define LIGHTGREEN  (uint16_t)~0X841F //
#define LIGHTGRAY   (uint16_t)~0XEF5B // (PANNEL)
#define LGRAY       (uint16_t)~0XC618 // (PANNEL)

#define LGRAYBLUE   (uint16_t)~0XA651 //
#define LBBLUE      (uint16_t)~0X2B12 //

void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(u16 Color);
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_DrawPoint(u16 x,u16 y);//
u16  LCD_ReadPoint(u16 x,u16 y); //
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);

u16 LCD_RD_DATA(void);//
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue);
void LCD_WR_DATA(u8 data);
u16 LCD_ReadReg(u8 LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(u16 RGB_Code);
u16 LCD_ReadRAM(void);
u16 LCD_BGR2RGB(u16 c);
void LCD_SetParam(void);
void Lcd_WriteData_16Bit(u16 Data);
void LCD_direction(u8 direction );

#endif
