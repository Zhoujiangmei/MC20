
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
  // ��״̬
  u8         State;          //��״̬����λָʾ���ݱ��ṹ������Щ�����ǿ���
  
  // ͨ����������
  Time_TYPE       Time;           //����ʱ��
  Latitude_TYPE 	Latitude;       //γ��
  Longitude_TYPE  Longitude;     //����

  double lfLatitude;
  double lfLongitude;
  				  
  // RMC����
  int16_t         Speed;          //�ٶ�*10����λ��knots
  int16_t         Course;         //�Ƕ�*10����λ��degrees
  
  // GGA����
  int16_t         Altitude;       //�߳�*10����λ��m
  
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