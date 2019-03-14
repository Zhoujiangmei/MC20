#include "ql_trace.h"
#include "ql_system.h"
#include "ql_gpio.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_uart.h"

#include "debug.h"
#include "Digitron.h"

unsigned char digdispword[6]={0x3F,0x06,0x5B,0x6D,0x07,0x40}; //0,1,2,5,7,-
unsigned char aabbcc = 0xFF;


//#define DIGSDAH Ql_GPIO_SetLevel(PINNAME_RTS,PINLEVEL_HIGH)
//#define DIGSDAL	Ql_GPIO_SetLevel(PINNAME_RTS,PINLEVEL_LOW)	
 

static void delay_nus(unsigned int n)
{
	while(n--)
	{
	;
	}
}

static void delay_nms(unsigned int n)
{
   unsigned int i;
    while(n--)
    {
        for(i=0;i<100;i++);
    }

}

void DIGSDAH(void )
{
   unsigned int i;
   delay_nus(DIGBITDELAY);
   for(i = 0; i < 7; i++)
   Ql_GPIO_SetLevel(PINNAME_RTS,PINLEVEL_HIGH);
}

void DIGSDAL(void )
{
   unsigned int i;
  delay_nus(DIGBITDELAY);
   for(i = 0; i < 7; i++)
   Ql_GPIO_SetLevel(PINNAME_RTS,PINLEVEL_LOW);
}


void DIG_INIT(void)
{
/*
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(DIG_SDA_GPIO_CLK,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;//GPIO_Mode_Out_OD,GPIO_Mode_Out_PP
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_InitStructure.GPIO_Pin = DIG_SDA_PIN;
    GPIO_Init(DIG_SDA_GPIO_PORT, &GPIO_InitStructure);
*/

     Enum_PinName  gpioPin = PINNAME_RTS; 
    // Define the initial level for GPIO pin
    Enum_PinLevel gpioLvl = PINLEVEL_HIGH;

    // Initialize the GPIO pin (output high level, pull up)
    Ql_GPIO_Init(gpioPin, PINDIRECTION_OUT, gpioLvl, PINPULLSEL_DISABLE);
    APP_DEBUG("<-- Initialize GPIO pin (PINNAME_STATUS): output, high level, pull up -->\r\n");

    // Get the direction of GPIO
    APP_DEBUG("<-- Get the GPIO direction: %d -->\r\n", Ql_GPIO_GetDirection(gpioPin));

    // Get the level value of GPIO
    APP_DEBUG("<-- Get the GPIO level value: %d -->\r\n\r\n", Ql_GPIO_GetLevel(gpioPin));


}


static void tm1652_send11bit(unsigned char ucdat)
{
unsigned char i,ucFlag=0;

    //DIGSDAH();
    //Ql_GPIO_SetLevel(PINNAME_RTS,PINLEVEL_HIGH);
    //delay_nus(3*DIGBITDELAY);
    DIGSDAL();
    //delay_nus(DIGBITDELAY);	  //起始位  延时52us，用示波器测试
    for(i=0;i<8;i++)
    {
        if(ucdat&0x01) 
        {
            DIGSDAH();
            ucFlag++;
        }
        else DIGSDAL();
        //delay_nus(DIGBITDELAY);
        ucdat>>=1;
    }
    if((ucFlag%2)==0)	DIGSDAH();  //	奇偶校验位 偶数发1
    else DIGSDAL();
    //delay_nus(DIGBITDELAY);			// 延时52us，用示波器测试

    DIGSDAH();			   	   //停止位	   延时52us，用示波器测试
    //delay_nus(DIGBITDELAY);	
 
}

//void DIG_DISP(float val)
//{
//  //tm1652_send11bit(0x18);
//  //tm1652_send11bit(0x00);
//  //delay_nms(520);//延时3ms以上
//    
//  tm1652_send11bit(0x08);

//  if((0<=val)&&(0.22>val)){//百分比0.0
//    tm1652_send11bit(0x00);
//    tm1652_send11bit(0x00);
//    tm1652_send11bit(digdispword[0]);
//  }
//  if((0.22<=val)&&(0.460>val)){//百分比0.25
//    tm1652_send11bit(0x00);
//    tm1652_send11bit(digdispword[2]);
//    tm1652_send11bit(digdispword[3]);
//  }
//  if((0.5<=val)&&(0.847>val)){//百分比0.5
//    tm1652_send11bit(0x00);
//    tm1652_send11bit(digdispword[3]);
//    tm1652_send11bit(digdispword[0]);
//  }
//  if((0.849<=val)&&(1.192>val)){//百分比0.75
//    tm1652_send11bit(0x00);
//    tm1652_send11bit(digdispword[4]);
//    tm1652_send11bit(digdispword[3]);
//  }
//  if((1.2<=val)&&(3.3>=val)){//百分比1.0
//    tm1652_send11bit(digdispword[1]);
//    tm1652_send11bit(digdispword[0]);
//    tm1652_send11bit(digdispword[0]);
//  }
//  
  /*delay_nms(350);//延时3ms以上
  tm1652_send11bit(0x18);
  tm1652_send11bit(0xFE);*/
//  //delay_nms(520);//延时3ms以上
//    
//}

void DIG_DISP(float val)
{

  Ql_GPIO_SetLevel(PINNAME_RTS,PINLEVEL_HIGH);

   /*
  if((0<=val)&&(0.22>val)){//百分比0.0

    tm1652_send11bit(0x08);
    tm1652_send11bit(0x00);
    tm1652_send11bit(0x00);
    tm1652_send11bit(digdispword[0]);
  }
  else if((0.22<=val)&&(0.495>val)){//百分比0.25

    tm1652_send11bit(0x08);
    tm1652_send11bit(0x00);
    tm1652_send11bit(digdispword[2]);
    tm1652_send11bit(digdispword[3]);
  }
 else  if((0.495<=val)&&(0.842>val)){//百分比0.5
    tm1652_send11bit(0x08);
    tm1652_send11bit(0x00);
    tm1652_send11bit(digdispword[3]);
    tm1652_send11bit(digdispword[0]);
  }
  else if((0.842<=val)&&(1.194>val)){//百分比0.75
    tm1652_send11bit(0x08);
    tm1652_send11bit(0x00);
    tm1652_send11bit(digdispword[4]);
    tm1652_send11bit(digdispword[3]);
  }
 else  if((1.194<=val)&&(3.3>=val)){//百分比1.0
    tm1652_send11bit(0x08);
    tm1652_send11bit(digdispword[1]);
    tm1652_send11bit(digdispword[0]);
    tm1652_send11bit(digdispword[0]);
  }
 else
 {
 }
*/


	
  if((0<=val)&&(0.23>val)){//百分比0.0

    tm1652_send11bit(0x08);
    tm1652_send11bit(0x00);
    tm1652_send11bit(0x00);
    tm1652_send11bit(digdispword[0]);
  }
  else if((0.23<=val)&&(0.66>val)){//百分比0.25

    tm1652_send11bit(0x08);
    tm1652_send11bit(0x00);
    tm1652_send11bit(digdispword[2]);
    tm1652_send11bit(digdispword[3]);
  }
 else  if((0.66<=val)&&(1.0>val)){//百分比0.5
    tm1652_send11bit(0x08);
    tm1652_send11bit(0x00);
    tm1652_send11bit(digdispword[3]);
    tm1652_send11bit(digdispword[0]);
  }
  else if((1.0<=val)&&(1.44>val)){//百分比0.75
    tm1652_send11bit(0x08);
    tm1652_send11bit(0x00);
    tm1652_send11bit(digdispword[4]);
    tm1652_send11bit(digdispword[3]);
  }
 else  if((1.44<=val)&&(1.586>=val)){//百分比1.0
    tm1652_send11bit(0x08);
    tm1652_send11bit(digdispword[1]);
    tm1652_send11bit(digdispword[0]);
    tm1652_send11bit(digdispword[0]);
  }
 else
 {
 }
 

    
}

void DIG_DISP_CLR(void)
{
    
	tm1652_send11bit(0x08);

	tm1652_send11bit(0x00);
	tm1652_send11bit(0x00);
	tm1652_send11bit(0x00);

//	delay_nms(350);//延时3ms以上
//	tm1652_send11bit(0x18);
//	tm1652_send11bit(0xFE);
    
}

void DIG_Brightness_Control(unsigned int level)
{

	//delay_nms(350);//延时3ms以上
	Ql_Sleep(1);
	tm1652_send11bit(0x18);
    switch(level){
        case 1:
            tm1652_send11bit(0xF0);
            break;
        case 2:
            tm1652_send11bit(0xF8);
            break;
        case 3:
            tm1652_send11bit(0xF4);
            break;
        case 4:
            tm1652_send11bit(0xFC);
            break;
        case 5:
            tm1652_send11bit(0xF2);
            break;
        case 6:
            tm1652_send11bit(0xFA);
            break;
        case 7:
            tm1652_send11bit(0xF6);
            break;
        case 8:
            tm1652_send11bit(0xFE);
            break;
        default:
            break;
    }
    
}

void DIG_Brightness_Control_Test(void)
{
static unsigned int i;

    if(8 == i)i = 0;
    i++;
    DIG_Brightness_Control(i);
    
}


void testTime(void )
{

	DIGSDAH();

  DIGSDAL();
;

}

