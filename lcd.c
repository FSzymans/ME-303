
#include "lcd.h"
#include "stdlib.h"

#include "Carte2026-432.h"

// =========== Librairie SPI directement dans LCD ======================

#define SPI_SCLK BIT3    // P6.3, fil sur P2.3 pour arriver à la carte
#define SPI_MOSI BIT4    // P6.4

#define SPI_MOSI_SET    P6OUT |= SPI_MOSI
#define SPI_MOSI_CLR    P6OUT &= ~SPI_MOSI

#define SPI_SCLK_SET    P2OUT |= SPI_SCLK
#define SPI_SCLK_CLR    P2OUT &= ~SPI_SCLK

#define SPI_MOSI_CLR_SCLK_CLR    P6OUT &=~(SPI_MOSI | SPI_SCLK);

/*****************************************************************************
 * @name       :void  SPI_WriteByte(u8 Data)
 * @date       :2018-08-09
 * @function   :Write a byte of data using STM32's hardware SPI
 * @parameters :SPIx: SPI type,x for 1,2,3
                Byte:Data to be written
 * @retvalue   :Data received by the bus
******************************************************************************/
void s_SPI_WriteByte(u8 Data) // soft, sans optimisation
{
    unsigned char i=0;
    for(i=8;i>0;i--)
    {
      if(Data&0x80)
      SPI_MOSI_SET;
      else SPI_MOSI_CLR;

      SPI_SCLK_CLR;
      SPI_SCLK_SET;
      Data<<=1;
    }
}

void fast_s_SPI_WriteByte(u8 Data) { // soft, avec optimisation
    if(Data&0x80) { SPI_MOSI_SET; SPI_SCLK_CLR; }
    else { SPI_MOSI_CLR_SCLK_CLR; }
    SPI_SCLK_SET; Data<<=1;

    if(Data&0x80) { SPI_MOSI_SET; SPI_SCLK_CLR; }
    else { SPI_MOSI_CLR_SCLK_CLR; }
    SPI_SCLK_SET; Data<<=1;

    if(Data&0x80) { SPI_MOSI_SET; SPI_SCLK_CLR; }
    else { SPI_MOSI_CLR_SCLK_CLR; }
    SPI_SCLK_SET; Data<<=1;

    if(Data&0x80) { SPI_MOSI_SET; SPI_SCLK_CLR; }
    else { SPI_MOSI_CLR_SCLK_CLR; }
    SPI_SCLK_SET; Data<<=1;

    if(Data&0x80) { SPI_MOSI_SET; SPI_SCLK_CLR; }
    else { SPI_MOSI_CLR_SCLK_CLR; }
    SPI_SCLK_SET; Data<<=1;

    if(Data&0x80) { SPI_MOSI_SET; SPI_SCLK_CLR; }
    else { SPI_MOSI_CLR_SCLK_CLR; }
    SPI_SCLK_SET; Data<<=1;

    if(Data&0x80) { SPI_MOSI_SET; SPI_SCLK_CLR; }
    else { SPI_MOSI_CLR_SCLK_CLR; }
    SPI_SCLK_SET; Data<<=1;

    if(Data&0x80) { SPI_MOSI_SET; SPI_SCLK_CLR; }
    else { SPI_MOSI_CLR_SCLK_CLR; }
    SPI_SCLK_SET; Data<<=1;
}
/*****************************************************************************
 * @name       :void SPI2_Init(void)
 * @date       :2018-08-09
 * @function   :Initialize the STM32 hardware SPI2
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void s_SPI_Init(void)
{
  P2DIR |= SPI_SCLK;
  P6DIR |= SPI_MOSI;
}

//============================================================
// Version hardware, avec fil P2.3 - P6.3
// pour utiliser P6.4 et P6.3

void SPI_Init(void) {
    P6->SEL0 |=  BIT3 | BIT4;            // P6.3 = CLK, P6.4 = MOSI
    P6->SEL1 &= ~(BIT3 | BIT4);

    EUSCI_B1->CTLW0 |= UCSWRST; // SPI reset

    EUSCI_B1->CTLW0 = // SPI configuration
        UCSWRST            |   // reset maintenu
        UCSSEL__SMCLK      |   // source SMCLK
        UCMST              |   // master
        UCSYNC             |   // SPI
        UCMODE_0           |   // 3-wire
        UCMSB              |   // MSB first
        UCCKPL             |   // CPOL = 1 (SCLK au repos à 1)
        UCCKPH             |   // CPHA = 1 (sur front montant)
        0;

    /* --- SPI speed --- */
    EUSCI_B1->BRW = 2;    // 12 MHz / 4 = 3 MHz (safe LCD)

    /* --- Release SPI --- */
    EUSCI_B1->CTLW0 &= ~UCSWRST;
}

void  SPI_WriteByte(u8 Data) {
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_TXIFG));
    EUSCI_B1->TXBUF = Data;

    while (EUSCI_B1->STATW & EUSCI_B_STATW_SPI_BUSY);
}
//============================================================

_lcd_dev lcddev;

u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;  
u16 DeviceCode;	 

/*****************************************************************************
 * @name       :void LCD_WR_REG(u8 data)
 * @date       :2018-08-09 
 * @function   :Write an 8-bit command to the LCD screen
 * @parameters :data:Command value to be written
 * @retvalue   :None
******************************************************************************/
void LCD_WR_REG(u8 data)
{    
   LCD_RS_CLR;	  
   SPI_WriteByte(data);
}

/*****************************************************************************
 * @name       :void LCD_WR_DATA(u8 data)
 * @date       :2018-08-09 
 * @function   :Write an 8-bit data to the LCD screen
 * @parameters :data:data value to be written
 * @retvalue   :None
******************************************************************************/
void LCD_WR_DATA(u8 data)
{
   LCD_RS_SET;
   SPI_WriteByte(data);
}

/*****************************************************************************
 * @name       :void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
 * @date       :2018-08-09 
 * @function   :Write data into registers
 * @parameters :LCD_Reg:Register address
                LCD_RegValue:Data to be written
 * @retvalue   :None
******************************************************************************/
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{	
  LCD_WR_REG(LCD_Reg);  
  LCD_WR_DATA(LCD_RegValue);	    		 
}	   

/*****************************************************************************
 * @name       :void LCD_WriteRAM_Prepare(void)
 * @date       :2018-08-09 
 * @function   :Write GRAM
 * @parameters :None
 * @retvalue   :None
******************************************************************************/	 
void LCD_WriteRAM_Prepare(void)
{
  LCD_WR_REG(lcddev.wramcmd);
}	 

/*****************************************************************************
 * @name       :void Lcd_WriteData_16Bit(u16 Data)
 * @date       :2018-08-09 
 * @function   :Write an 16-bit command to the LCD screen
 * @parameters :Data:Data to be written
 * @retvalue   :None
******************************************************************************/	 
void Lcd_WriteData_16Bit(u16 Data)
{	
   LCD_RS_SET;  
   SPI_WriteByte(Data>>8);
   SPI_WriteByte(Data);
}

/*****************************************************************************
 * @name       :void LCD_DrawPoint(u16 x,u16 y)
 * @date       :2018-08-09 
 * @function   :Write a pixel data at a specified location
 * @parameters :x:the x coordinate of the pixel
                y:the y coordinate of the pixel
 * @retvalue   :None
******************************************************************************/	
void LCD_DrawPoint(u16 x,u16 y)
{
  LCD_SetCursor(x,y); //
  Lcd_WriteData_16Bit(POINT_COLOR); 
}

/*****************************************************************************
 * @name       :void LCD_Clear(u16 Color)
 * @date       :2018-08-09 
 * @function   :Full screen filled LCD screen
 * @parameters :color:Filled color
 * @retvalue   :None
******************************************************************************/	
void LCD_Clear(u16 Color)
{
  unsigned int i,m;  
  LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);   
  LCD_RS_SET;
  for(i=0;i<lcddev.height;i++)
  {
    for(m=0;m<lcddev.width;m++)
    {	
	SPI_WriteByte(Color>>8);
	SPI_WriteByte(Color);
    }
  }
} 

/*****************************************************************************
 * @name       :void LCD_GPIOInit(void)
 * @date       :2018-08-09 
 * @function   :Initialization LCD screen GPIO
 * @parameters :None
 * @retvalue   :None
******************************************************************************/	
void LCD_GPIOInit(void)
{
  P5DIR|=LED; // Back-light
  P5OUT &=~LED; // Off
  
  P6DIR|=LCD_RS; // DC

  P3DIR|=LCD_RST; // ResetLow

  P5DIR|=LCD_CS;
  P5OUT &=~LCD_CS; // On
}

/*****************************************************************************
 * @name       :void LCD_RESET(void)
 * @date       :2018-08-09 
 * @function   :Reset LCD screen
 * @parameters :None
 * @retvalue   :None
******************************************************************************/	
void LCD_RESET(void)
{
	LCD_RST_CLR;
	delay_ms(20);	
	LCD_RST_SET;
	delay_ms(20);
}

/*****************************************************************************
 * @name       :void LCD_RESET(void)
 * @date       :2018-08-09 
 * @function   :Initialization LCD screen
 * @parameters :None
 * @retvalue   :None
******************************************************************************/	 	 
void LCD_Init(void)
{  
	SPI_Init(); // SPI
	LCD_GPIOInit();//LCD GPIO									 
 	LCD_RESET(); // LCD
//************* ST7789 **********//	
	LCD_WR_REG(0x36); 
	LCD_WR_DATA(0x00);

	LCD_WR_REG(0x3A); 
	LCD_WR_DATA(0x05);

	LCD_WR_REG(0xB2);
	LCD_WR_DATA(0x0C);
	LCD_WR_DATA(0x0C);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x33);
	LCD_WR_DATA(0x33);

	LCD_WR_REG(0xB7); 
	LCD_WR_DATA(0x35);  

	LCD_WR_REG(0xBB);
	LCD_WR_DATA(0x19);

	LCD_WR_REG(0xC0);
	LCD_WR_DATA(0x2C);

	LCD_WR_REG(0xC2);
	LCD_WR_DATA(0x01);

	LCD_WR_REG(0xC3);
	LCD_WR_DATA(0x12);   

	LCD_WR_REG(0xC4);
	LCD_WR_DATA(0x20);  

	LCD_WR_REG(0xC6); 
	LCD_WR_DATA(0x0F);    

	LCD_WR_REG(0xD0); 
	LCD_WR_DATA(0xA4);
	LCD_WR_DATA(0xA1);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA(0xD0);
	LCD_WR_DATA(0x04);
	LCD_WR_DATA(0x0D);
	LCD_WR_DATA(0x11);
	LCD_WR_DATA(0x13);
	LCD_WR_DATA(0x2B);
	LCD_WR_DATA(0x3F);
	LCD_WR_DATA(0x54);
	LCD_WR_DATA(0x4C);
	LCD_WR_DATA(0x18);
	LCD_WR_DATA(0x0D);
	LCD_WR_DATA(0x0B);
	LCD_WR_DATA(0x1F);
	LCD_WR_DATA(0x23);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA(0xD0);
	LCD_WR_DATA(0x04);
	LCD_WR_DATA(0x0C);
	LCD_WR_DATA(0x11);
	LCD_WR_DATA(0x13);
	LCD_WR_DATA(0x2C);
	LCD_WR_DATA(0x3F);
	LCD_WR_DATA(0x44);
	LCD_WR_DATA(0x51);
	LCD_WR_DATA(0x2F);
	LCD_WR_DATA(0x1F);
	LCD_WR_DATA(0x1F);
	LCD_WR_DATA(0x20);
	LCD_WR_DATA(0x23);

	LCD_WR_REG(0x21); 

	LCD_WR_REG(0x11); 
	//Delay (120); 

	LCD_WR_REG(0x29); 	
    LCD_direction(USE_HORIZONTAL);// LCD
    LCD_LED(1);//
	LCD_Clear(BLACK);//
	// LCD_LED(1);//
	lcddev.zoomX = lcddev.zoomY = 1;
}
 
/*****************************************************************************
 * @name       :void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
 * @date       :2018-08-09 
 * @function   :Setting LCD display window
 * @parameters :xStar:the beginning x coordinate of the LCD display window
								yStar:the beginning y coordinate of the LCD display window
								xEnd:the end x coordinate of the LCD display window
								yEnd:the end y coordinate of the LCD display window
 * @retvalue   :None
******************************************************************************/ 
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
{	
	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA((xStar+lcddev.xoffset)>>8);
	LCD_WR_DATA(xStar+lcddev.xoffset);		
	LCD_WR_DATA((xEnd+lcddev.xoffset)>>8);
	LCD_WR_DATA(xEnd+lcddev.xoffset);

	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA((yStar+lcddev.yoffset)>>8);
	LCD_WR_DATA(yStar+lcddev.yoffset);		
	LCD_WR_DATA((yEnd+lcddev.yoffset)>>8);
	LCD_WR_DATA(yEnd+lcddev.yoffset);

	LCD_WriteRAM_Prepare();	// GRAM			
}   

/*****************************************************************************
 * @name       :void LCD_SetCursor(u16 Xpos, u16 Ypos)
 * @date       :2018-08-09 
 * @function   :Set coordinate value
 * @parameters :Xpos:the  x coordinate of the pixel
								Ypos:the  y coordinate of the pixel
 * @retvalue   :None
******************************************************************************/ 
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	  	    			
	LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);	
} 

/*****************************************************************************
 * @name       :void LCD_direction(u8 direction)
 * @date       :2018-08-09 
 * @function   :Setting the display direction of LCD screen
 * @parameters :direction:0-0 degree
                          1-90 degree
													2-180 degree
													3-270 degree
 * @retvalue   :None
******************************************************************************/ 
void LCD_direction(u8 direction)
{ 
			lcddev.setxcmd=0x2A;
			lcddev.setycmd=0x2B;
			lcddev.wramcmd=0x2C;
	switch(direction){		  
		case 0:						 	 		
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;	
			lcddev.xoffset=0;
			lcddev.yoffset=0;
			LCD_WriteReg(0x36,0);//BGR==1,MY==0,MX==0,MV==0
		break;
		case 1:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
			lcddev.xoffset=0;
			lcddev.yoffset=0;
			LCD_WriteReg(0x36,(1<<6)|(1<<5));//BGR==1,MY==1,MX==0,MV==1
		break;
		case 2:						 	 		
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;
      lcddev.xoffset=0;
			lcddev.yoffset=80;			
			LCD_WriteReg(0x36,(1<<6)|(1<<7));//BGR==1,MY==0,MX==0,MV==0
		break;
		case 3:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
			lcddev.xoffset=80;
			lcddev.yoffset=0;
			LCD_WriteReg(0x36,(1<<7)|(1<<5));//BGR==1,MY==1,MX==0,MV==1
		break;	
		default:break;
	}		
}	 
