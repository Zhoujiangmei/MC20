#include "custom_feature_def.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_uart.h"
#include "ql_error.h"
#include "ql_gprs.h"
#include "fota_main.h"
#include "ql_stdlib.h"
#include "ql_timer.h"

#include "Debug.h"
#include "appOTA_http.h"

#define APN      "cmnet"
#define USERID   ""
#define PASSWD   ""

//struct firmwareInfo softInfo;
//#define APP_BIN_URL   "http://183.230.40.32:8080/b?p=116384&k=46h2Mz1DkCim_XLO8ndSd3M04Ko0EDi-D9cqMQ0VgCQ"
//#define APP_BIN_URL   "http://183.230.40.33:80/software/b?p=112983&k=xJTKrmBe3hfPUjyOq_00ImN67cj2FpAoyiqeQSh9pAOFFLf2OkiDRtCFBnrfzohu"
//#define APP_BIN_URL   "http://api.heclouds.com/software/b?p=112983&k=xJTKrmBe3hfPUjyOq_00ImN67cj2FpAoyiqeQSh9pAOFFLf2OkiDRtCFBnrfzohu"


 u8 m_URL_Buffer[URL_LEN] = {0};


void OTA_main(void )
{
      ST_GprsConfig apnCfg;
      Ql_memcpy(apnCfg.apnName,   APN, Ql_strlen(APN));
      Ql_memcpy(apnCfg.apnUserId, USERID, Ql_strlen(USERID));
      Ql_memcpy(apnCfg.apnPasswd, PASSWD, Ql_strlen(PASSWD));
               
      //http://hostname:port/filePath/fileName
      
     //Ql_sprintf(m_URL_Buffer, "%s",APP_BIN_URL);
     APP_DEBUG("\r\n<-- URL:%s-->\r\n",m_URL_Buffer);
                    
     Ql_FOTA_StartUpgrade(m_URL_Buffer, &apnCfg, NULL);
}