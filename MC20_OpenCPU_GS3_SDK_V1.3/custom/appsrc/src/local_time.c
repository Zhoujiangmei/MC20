
#include "ql_type.h"
#include "ql_trace.h"

#include "ql_system.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_uart.h"


#include "local_time.h"
#include "Debug.h"



void Time_Config(void )
{
     s32 ret;
    ST_Time time;
    //set local time
    //time.year = 2017;
   // time.month = 12;
   // time.day = 26;
   // time.hour = 11;
   // time.minute = 12;
   // time.second = 18;
    time.timezone = 22; // +05:30, one digit expresses a quarter of an hour
    ret = Ql_SetLocalTime(&time);
    APP_DEBUG("<-- Ql_SetLocalTime(%d.%02d.%02d %02d:%02d:%02d timezone=%02d)=%d -->\n\r", 
        time.year, time.month, time.day, time.hour, time.minute, time.second, time.timezone, ret);
}

ST_Time Get_Local_Time(void )
{
    //get local time
    ST_Time time;
    if((Ql_GetLocalTime(&time)))
    {
        time.hour = (time.hour + 8) % 24;
        APP_DEBUG("\r\n<--Local time successfuly determined: %i.%i.%i %i:%i:%i timezone=%i-->\r\n", time.day, time.month, time.year, time.hour, time.minute, time.second,time.timezone);
	

    }
    else
    {
        APP_DEBUG("\r\n<--failed !! Local time not determined -->\r\n");
	
    }
	return time;
}
