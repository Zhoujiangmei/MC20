#include "ril.h"
#include "ril_util.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_system.h"
#include "ql_uart.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ril_gps.h"

#include "debug.h"

#include "ril.h"
#include "ril_util.h"
#include "ql_trace.h"
#include "ql_system.h"
#include "ql_uart.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ril_gps.h"


#include "appGPS.h"
#include "common.h"
//#include "EdpDemo.h"


u32 uiUSRLat = 0;
u32 uiUSRLong = 0;
 static u8 s_GPSDataValidNum = 3;
  static GPS_DATA_TYPE s_GPSData;

nmeaINFO g_Nmeainfo;
nmeaPARSER g_Nmeaparser;



static void trace(const char *str, size_t str_size) {

}
static void error(const char *str, size_t str_size) {

}



int s_inUseGpsSatellitesNum = 0;
int s_inViewGpsSatellitesNum = 0;

void GPS_PraseSatellitesNum(nmeaINFO Nmeainfo)
{
    if((g_Nmeainfo.smask & GPGSV) != 0)
    {   
        s_inUseGpsSatellitesNum = g_Nmeainfo.satinfo.inuse;
        s_inViewGpsSatellitesNum = g_Nmeainfo.satinfo.inview;
    }
}

int GPS_ReadSatellitesinUse(void)
{
    return s_inUseGpsSatellitesNum;
}

int GPS_ReadSatellitesinView(void)
{
    return s_inViewGpsSatellitesNum;
}
void GPS_CleanSatellitesNum(void)
{
        s_inUseGpsSatellitesNum = 0;
        s_inViewGpsSatellitesNum = 0;
}


void NumTo2Ascii(char *pszStr,int iNum)
{
    pszStr[1] = (iNum % 10) + '0' ;
    pszStr[0] = (iNum / 10) + '0' ;
}

void GPS_NmeaPacketLocalDat(const nmeaINFO *pNmeainfo, GPS_DATA_TYPE *pGPSData)
{
    int iTmpYear,iTmpMon;
		char szLon[9],szLat[8];
    pGPSData->State       = GPS_GETDATA_RMC;   //作为起点接收数据

    pGPSData->lfLatitude    = pNmeainfo->lat;
    pGPSData->lfLongitude   = pNmeainfo->lon;
    Ql_sprintf(szLat,"%8d",pNmeainfo->lat*10000);
    Ql_sprintf(szLon,"%9d",pNmeainfo->lon*10000);
    Ql_strncpy((uint8 *)&pGPSData->Latitude.mmmm,(szLat),8);	//mystrrev
    Ql_strncpy((uint8 *)&pGPSData->Longitude.mmmm,(szLon),9);//mystrrev
    pGPSData->Speed       = pNmeainfo->speed;
    pGPSData->Course      = pNmeainfo->direction;
    pGPSData->Altitude    = pNmeainfo->elv;
    iTmpYear = pNmeainfo->utc.year - 100 ;
    iTmpMon =  pNmeainfo->utc.mon + 1 ;
    NumTo2Ascii( pGPSData->Time.Year , iTmpYear );
    NumTo2Ascii( pGPSData->Time.Mon , iTmpMon );
    NumTo2Ascii( pGPSData->Time.Day , pNmeainfo->utc.day );
    NumTo2Ascii( pGPSData->Time.Hour , pNmeainfo->utc.hour );  
    NumTo2Ascii(pGPSData->Time.Min, pNmeainfo->utc.min);
    NumTo2Ascii(pGPSData->Time.Sec, pNmeainfo->utc.sec);


}


void GPS_PowerOn(void )
{
     s32 iRet = 0;
     iRet = RIL_GPS_Open(1);

    if(RIL_AT_SUCCESS != iRet) 
   {
        APP_DEBUG("Power on GPS fail, iRet = %d.\r\n", iRet);
   }
   else
   {
       APP_DEBUG("Power on GPS Successful.\r\n");
   }
 
}


void GPS_PowerOff(void )
{
     s32 iRet = 0;
     iRet = RIL_GPS_Open(0);

    if(RIL_AT_SUCCESS != iRet) 
   {
        APP_DEBUG("Power off GPS fail, iRet = %d.\r\n", iRet);

   }
   else
   {
       APP_DEBUG("Power off GPS Successful.\r\n");
   }

}



void GPS_Locatin(void )
{

    u8 rdBuff[1000];
    u8 item[4] = {"ALL"};
    s32 iRet = 0;

    Ql_memset(rdBuff,0,sizeof(rdBuff));
    //Ql_strncpy(item,"ALL",3);

   iRet = RIL_GPS_Read(item,rdBuff);
   if(RIL_AT_SUCCESS != iRet)
  {
      APP_DEBUG("Read %s information failed.\r\n",item);
  }
   else
   {
      APP_DEBUG("%s\r\n",rdBuff);
   }

}


void GPSNmea_init(void  )
{
    nmea_property()->trace_func = &trace;
    nmea_property()->error_func = &error;

    nmea_zero_INFO(&g_Nmeainfo);
    nmea_parser_init(&g_Nmeaparser);   
}

u8 GPS_Handle(void )
{
   u8 GPSrdBuff[1024];
    u8 item[4] = {"ALL"};
    u8 GPSrdHead[10] = {"+QGNSSRD: "};
    u8 ucRead;
    s32 iRet = 0;
    nmeaPOS tmpPOS = { 0 };
    static uint8 ucVaildCnt = 0;
  // u8 gps_str[] = "$GNRMC,071514.000,A,2235.2081,N,11351.3216,E,0.00,333.73,191217,,,A*70\r\n,  $GNVTG,333.73,T,,M,0.00,N,0.00,K,A*24\r\n,  $GNGGA,071514.000,2235.2081,N,11351.3216,E,1,4,2.58,519.5,M,-3.1,M,,*54\r\n" ;                    

   
    Ql_memset(GPSrdBuff,0,sizeof(GPSrdBuff));
    Ql_strncpy(item,"ALL",3);

   iRet = RIL_GPS_Read(item,GPSrdBuff);
   if(RIL_AT_SUCCESS != iRet)
  {
      APP_DEBUG("Read %s information failed.\r\n",item);
      
  }
   else
   {
      APP_DEBUG("%s,%d\r\n",&GPSrdBuff[0],Ql_strlen(GPSrdBuff));
         ucRead = nmea_parse(&g_Nmeaparser, &GPSrdBuff[0], Ql_strlen(GPSrdBuff), &g_Nmeainfo);
   //ucRead = nmea_parse(&g_Nmeaparser, &gps_str[0], (int)Ql_strlen(gps_str), &g_Nmeainfo);			
	 
   tmpPOS.lat = nmea_ndeg2degree(g_Nmeainfo.lat);
   tmpPOS.lon = nmea_ndeg2degree(g_Nmeainfo.lon);
   GPS_PraseSatellitesNum(g_Nmeainfo);


  APP_DEBUG("nmea_parse ucRead = %d  g_Nmeainfo.lat = %lf ,g_Nmeainfo.lon = %lf \r\n"
                            ,ucRead,tmpPOS.lat,tmpPOS.lon);
  APP_DEBUG("GPS Satellites inUse = %d  inView = %d \r\n"
                            ,GPS_ReadSatellitesinUse(),GPS_ReadSatellitesinView());


        if(0 != ucRead){
            if((g_Nmeainfo.smask & GPRMC) != 0) //推荐数据类
            if((0 != g_Nmeainfo.lon) &&(0 != g_Nmeainfo.lat)){
                //ucVaildCnt ++;
               // if(ucVaildCnt >= s_GPSDataValidNum )
		 {
                    GPS_NmeaPacketLocalDat(&g_Nmeainfo, &s_GPSData);
                    uiUSRLong = tmpPOS.lon * 1000000 ;
                    uiUSRLat = tmpPOS.lat * 1000000;
					//GPS_CleanSatellitesNum();
                    ucVaildCnt = 0;
		      nmea_zero_INFO(&g_Nmeainfo);		
                    return TRUE;
                }
               // nmea_zero_INFO(&g_Nmeainfo);
            }
        }    
        
   }

   return FALSE;
}
