#include <string.h>
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_timer.h"
#include "ql_uart.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_system.h"

#include "debug.h"
#include "apptimer.h"
#include "appGPS.h"
#include "EdpDemo.h"
#include "app_adc.h"

#define TIMEOUT_COUNT 60
static u32 Stack_timer = 0x102; // timerId =99; timerID is Specified by customer, but must ensure the timer id is unique in the opencpu task
static u32 ST_Interval = 1000;
static s32 m_param1 = 1;

static u32 GP_timer = 0x101;
static u32 GPT_Interval =1000;
static s32 m_param2 = 9;

static int  s_iPassTask = 1;

static void Timer_handler(u32 timerId, void* param);



void timer_init(void )
{
    s32 ret;

    //register  a timer
    ret = Ql_Timer_Register(Stack_timer, Timer_handler, &m_param1);
    if(ret <0)
    {
        APP_DEBUG("\r\n<--failed!!, Ql_Timer_Register: timer(%d) fail ,ret = %d -->\r\n",Stack_timer,ret);
    }
    APP_DEBUG("\r\n<--Register: timerId=%d, param = %d,ret = %d -->\r\n", Stack_timer ,m_param1,ret); 


    //start a timer,repeat=true;
    ret = Ql_Timer_Start(Stack_timer,ST_Interval,TRUE);
    if(ret < 0)
    {
        APP_DEBUG("\r\n<--failed!! stack timer Ql_Timer_Start ret=%d-->\r\n",ret);        
    }
    APP_DEBUG("\r\n<--stack timer Ql_Timer_Start(ID=%d,Interval=%d,) ret=%d-->\r\n",Stack_timer,ST_Interval,ret);     
}


void timer_start(void )
{
     s32 ret;
     ret = Ql_Timer_Start(Stack_timer,ST_Interval,TRUE);
     if(ret < 0)
     {
         APP_DEBUG("\r\n<--failed!! stack timer Ql_Timer_Start ret=%d-->\r\n",ret);        
     }
     APP_DEBUG("\r\n<--stack timer Ql_Timer_Start(ID=%d,Interval=%d,) ret=%d-->\r\n",Stack_timer,ST_Interval,ret);
}


// timer callback function
void Timer_handler(u32 timerId, void* param)
{
     s32 iRet, ret;
    *((s32*)param) +=1;
    if(Stack_timer == timerId)
    {
        APP_DEBUG("<-- stack Timer_handler, param:%d -->\r\n", *((s32*)param));
        // stack_timer repeat 
        if(*((s32*)param) >= TIMEOUT_COUNT)
        {
           // Ql_Timer_Stop(Stack_timer);
            *((s32*)param) = 0;
            /*
              GPS_PowerOn();
	      if(GPS_Handle() == TRUE)
	      	{	
                  GPSToOneNet_Hander();
		    Ql_Sleep(1000);
	      	}
	       
              ADC_Start();
	       Ql_Sleep(1500);
	       DATAToOneNet_Hander();
		//GPS_PowerOff();
		*/
		//ADC_Start();
		
		//DATAToOneNet_Hander();
		//FOTA_Program();
           // s_iPassTask = subtask1_id;  //subtask1_id  main_task_id
           //iRet = Ql_OS_SendMessage(s_iPassTask, MSG_ID_GPS_READY, m_param1, m_param2);  

           //Ql_OS_SendMessage(main_task_id, Firewarm_Updata_READY, 1, 1); 
        }        
    }
    
}


