//#include "Common.h"
#include "ql_stdlib.h"
#include "ql_type.h"
#include "ql_uart.h"
#include "ql_system.h"

u8 TCP_Server(void);
void EDP_Loop(u8 ch);
u8 Connect_RequestType1(u8 *devid, u8 *api_key);
u8 Ping_Server(void);
u8 Save_GpsToOneNet(void);
u8 Send_DataToOneNet(void );
s32 DoSend(s32 sockfd, const u8 *buffer, u32 len);
void edptest(void );
void GPSToOneNet_Hander(void );
void DATAToOneNet_Hander(void );
void MeterDATAToOneNet_Hander(void );
 void MeterRecordSendOneNet(u8 ucMethod,u8 ucBit,u8 ucflag);

void FOTA_Program(void );

s32 EDP_GetAuth(void );
void TCP_Close(void );

#define  DEP_DATA     1
#define  DEP_GPS       0
#define  DEP_METER   2
typedef struct{
	u16 domain[1];
	u8   id[9];
	u8 appkey[29];
}DEV_PARAM;


extern DEV_PARAM dev_param;
#define     Firewarm_Updata_READY    (MSG_ID_USER_START + 0x101)