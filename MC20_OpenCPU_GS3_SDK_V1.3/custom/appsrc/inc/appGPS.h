
#include "ql_type.h"

#include "nmea.h"


#define GPS_GETDATA_RMC                   (u8)(1<<0)
#define GPS_GETDATA_GGA                   (u8)(1<<1)

typedef struct   
{       
       u8  Indicator;
	char	dd[2];
	char	mm[2];
	char	mmmm[4];
}Latitude_TYPE;

typedef struct   
{
	u8	Indicator;
	char	ddd[3];
	char	mm[2];
	char	mmmm[4];
}Longitude_TYPE;

typedef struct   
{
    u8   Year[2];
    u8   Mon[2];
    u8   Day[2];		
    u8   Hour[2];
    u8   Min[2];
    u8   Sec[2]; 
}Time_TYPE;


typedef struct  
{
  // 总状态
  u8         State;          //总状态，按位指示数据本结构中有哪些数据是可用
  
  // 通用数据数据
  Time_TYPE       Time;           //世界时间
  Latitude_TYPE 	Latitude;       //纬度
  Longitude_TYPE  Longitude;     //经度

  double lfLatitude;
  double lfLongitude;
  				  
  // RMC数据
  int16_t         Speed;          //速度*10，单位：knots
  int16_t         Course;         //角度*10，单位：degrees
  
  // GGA数据
  int16_t         Altitude;       //高程*10，单位：m
  
}GPS_DATA_TYPE;


extern u32 uiUSRLat;
extern u32 uiUSRLong;


void GPS_PowerOn(void );
void GPS_PowerOff(void );
void GPS_Locatin(void );
void GPSNmea_init(void  );
u8 GPS_Handle(void );

void GPS_PraseSatellitesNum(nmeaINFO Nmeainfo);
int GPS_ReadSatellitesinUse(void);
int GPS_ReadSatellitesinView(void);
void GPS_CleanSatellitesNum(void);
void NumTo2Ascii(char *pszStr,int iNum);
void GPS_NmeaPacketLocalDat(const nmeaINFO *pNmeainfo, GPS_DATA_TYPE *pGPSData);