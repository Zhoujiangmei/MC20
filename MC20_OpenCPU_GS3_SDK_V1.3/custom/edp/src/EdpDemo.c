

#include <Ql_stdlib.h >
#include "custom_feature_def.h"


#include "ql_error.h"
#include "ql_trace.h"
#include "ql_system.h"

#include "Ql_gprs.h"
#include "Ql_socket.h"
#include "Ql_time.h"

#include "EdpKit.h"
#include "EdpDemo.h"
#include "debug.h"
#include "appGPS.h"
#include "local_time.h"
#include "app_adc.h"
#include "appOTA_http.h"
#include "Function.h"
#include "Meter.h"

//extern s32 (*Ql_sprintf)(char *, const char *, ...);
DEV_PARAM dev_param;
//firmwareInfo upInfoTemp;

static ST_GprsConfig m_GprsConfig = {
    "CMNET",    // APN name
    "",         // User name for APN
    "",         // Password for APN
    0,
    NULL,
    NULL,
};


/************************************************************************/
/* Definition for Server IP Address and Socket Port Number              */
/************************************************************************/
 static s32 pdpCntxtId = -1;

static u8  m_SrvADDR[20] = "183.230.40.39\0";
static u32 m_SrvPort = 876;

#define  SOC_RECV_BUFFER_LEN  1460
static s32 m_GprsActState    = 0;   // GPRS PDP activation state, 0= not activated, 1=activated
static s32 m_SocketId        = -1;  // Store socket Id that returned by Ql_SOC_Create()
static s32 m_SocketConnState = 0;   // Socket connection state, 0= disconnected, 1=connected
static u8  m_SocketRcvBuf[SOC_RECV_BUFFER_LEN];


/************************************************************************/
/* Declarations for GPRS and TCP socket callback                        */
/************************************************************************/

//
// This callback function is invoked when GPRS drops down.
static void Callback_GPRS_Deactived(u8 contextId, s32 errCode, void* customParam );
//
// This callback function is invoked when the socket connection is disconnected by server or network.
static void Callback_Socket_Close(s32 socketId, s32 errCode, void* customParam );
//
// This callback function is invoked when socket data arrives.
static void Callback_Socket_Read(s32 socketId, s32 errCode, void* customParam );
//
// This callback function is invoked in the following case:
// The return value is less than the data length to send when calling Ql_SOC_Send(), which indicates
// the socket buffer is full. Application should stop sending socket data till this callback function
// is invoked, which indicates application can continue to send data to socket.
static void Callback_Socket_Write(s32 socketId, s32 errCode, void* customParam );






 u8 TCP_Server(void )
{

    s32 ret;
  
    ST_PDPContxt_Callback callback_gprs_func = {
        NULL,
        Callback_GPRS_Deactived
    };
    ST_SOC_Callback callback_soc_func = {
        NULL,
        Callback_Socket_Close,
        NULL,
        Callback_Socket_Read,    
        Callback_Socket_Write
    };

    /*1  Register GPRS callback*/
    pdpCntxtId = Ql_GPRS_GetPDPContextId();
    if (GPRS_PDP_ERROR == pdpCntxtId)
    {
        APP_DEBUG("No PDP context is available\r\n");
        return ret;
    }

    APP_DEBUG("<--pdpCntxtId  %d -->\r\n", pdpCntxtId);
    ret = Ql_GPRS_Register(pdpCntxtId, &callback_gprs_func, NULL);
    if ((GPRS_PDP_SUCCESS == ret) || (GPRS_PDP_ALREADY == ret))
    {
        APP_DEBUG("<-- Register GPRS callback function -->\r\n");
    }else{
        APP_DEBUG("<-- Fail to register GPRS, cause=%d. -->\r\n", ret);
        return FALSE;
    }

 
    /*Configure PDP*/


    ret = Ql_GPRS_Config(pdpCntxtId, &m_GprsConfig);
    if (GPRS_PDP_SUCCESS == ret)
    {
        APP_DEBUG("<-- Configure PDP context -->\r\n");
    }else{
        APP_DEBUG("<-- Fail to configure GPRS PDP, cause=%d. -->\r\n", ret);
        return FALSE;
    }
	
    /*Activate GPRS PDP context*/
    APP_DEBUG("<-- Activating GPRS... -->\r\n");
    ret = Ql_GPRS_ActivateEx(pdpCntxtId, TRUE);
    if (ret == GPRS_PDP_SUCCESS)
    {
        m_GprsActState = 1;
        APP_DEBUG("<-- Activate GPRS successfully. -->\r\n\r\n");
    }else{
        APP_DEBUG("<-- Fail to activate GPRS, cause=%d. -->\r\n\r\n", ret);
        return FALSE;
    }

    /*Register Socket callback*/
    ret = Ql_SOC_Register(callback_soc_func, NULL);
    if (SOC_SUCCESS == ret)
    {
        APP_DEBUG("<-- Register socket callback function -->\r\n");
    }else{
        APP_DEBUG("<-- Fail to register socket callback, cause=%d. -->\r\n", ret);
        return FALSE;
    }

    //5. Create socket
    m_SocketId = Ql_SOC_Create(pdpCntxtId, SOC_TYPE_TCP);
    if (m_SocketId >= 0)
    {
        APP_DEBUG("<-- Create socket successfully, socket id=%d. -->\r\n", m_SocketId);
    }else{
        APP_DEBUG("<-- Fail to create socket, cause=%d. -->\r\n", m_SocketId);
        return FALSE;
    }		

    //6. Connect to server
    {
        //6.1 Convert IP format
        u8 m_ipAddress[4]; 
        Ql_memset(m_ipAddress,0,5);
        ret = Ql_IpHelper_ConvertIpAddr(m_SrvADDR, (u32 *)m_ipAddress);
        if (SOC_SUCCESS == ret) // ip address is xxx.xxx.xxx.xxx
        {
            APP_DEBUG("<-- Convert Ip Address successfully,m_ipaddress=%d,%d,%d,%d -->\r\n",m_ipAddress[0],m_ipAddress[1],m_ipAddress[2],m_ipAddress[3]);
        }else{
            APP_DEBUG("<-- Fail to convert IP Address --> \r\n");
            return FALSE;
        }

        //6.2 Connect to server
        APP_DEBUG("<-- Connecting to server(IP:%d.%d.%d.%d, port:%d)... -->\r\n", m_ipAddress[0],m_ipAddress[1],m_ipAddress[2],m_ipAddress[3], m_SrvPort);
        ret = Ql_SOC_ConnectEx(m_SocketId,(u32) m_ipAddress, m_SrvPort, TRUE);
        if (SOC_SUCCESS == ret)
        {
            m_SocketConnState = 1;
            APP_DEBUG("<-- Connect to server successfully -->\r\n");
	     return m_SocketConnState;
        }else{
            APP_DEBUG("<-- Fail to connect to server, cause=%d -->\r\n", ret);
            APP_DEBUG("<-- Close socket.-->\r\n");
            Ql_SOC_Close(m_SocketId);
            m_SocketId = -1;
            return FALSE;
        }
    }

return 0;
}


void TCP_Close(void )
{
      s32 ret,ackNum;

     //8. Close socket
     ret = Ql_SOC_Close(m_SocketId);
     APP_DEBUG("<-- Close socket[%d], cause=%d --> \r\n", m_SocketId, ret);
    m_SocketId = -1;

     //9. Deactivate GPRS
     APP_DEBUG("<-- Deactivating GPRS... -->\r\n");
     ret = Ql_GPRS_DeactivateEx(pdpCntxtId, TRUE);
     APP_DEBUG("<-- Deactivated GPRS, cause=%d -->\r\n\r\n", ret);
}


static void Callback_GPRS_Deactived(u8 contextId, s32 errCode, void* customParam )
{
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: deactivated GPRS successfully.-->\r\n"); 
    }else{
        APP_DEBUG("<--CallBack: fail to deactivate GPRS, cause=%d)-->\r\n", errCode); 
    }
    if (1 == m_GprsActState)
    {
        m_GprsActState = 0;
        APP_DEBUG("<-- GPRS drops down -->\r\n"); 
    }
}
//
//
// This callback function is invoked when the socket connection is disconnected by server or network.
//
static void Callback_Socket_Close(s32 socketId, s32 errCode, void* customParam )
{
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: close socket successfully.-->\r\n"); 
    }
    else if(errCode == SOC_BEARER_FAIL)
    {   
        APP_DEBUG("<--CallBack: fail to close socket,(socketId=%d,error_cause=%d)-->\r\n", socketId, errCode); 
    }else{
        APP_DEBUG("<--CallBack: close socket failure,(socketId=%d,error_cause=%d)-->\r\n", socketId, errCode); 
    }
    if (1 == m_SocketConnState)
    {
        APP_DEBUG("<-- Socket connection is disconnected -->\r\n"); 
        APP_DEBUG("<-- Close socket at module side -->\r\n"); 
        Ql_SOC_Close(socketId);
        m_SocketConnState = 0;
    }
}
//
//
// This callback function is invoked when socket data arrives.
// The program should call Ql_SOC_Recv to read all data out of the socket buffer.
//
static void Callback_Socket_Read(s32 socketId, s32 errCode, void* customParam )
{
    s32 ret,i,recv_len;
    s32 offset = 0;
    s32  rtn;
    u8 mtype;
    EdpPacket* pkg;
    UpdateInfoList* up_info = NULL;
    RecvBuffer* recv_buf = NULL;
    u8 updataFlag = 0;
    s32 iRet;
	
    if (errCode)
    {
        APP_DEBUG("<-- Close socket -->\r\n");
        Ql_SOC_Close(socketId);
        m_SocketId = -1;
        return;
    }

    Ql_memset(m_SocketRcvBuf, 0, SOC_RECV_BUFFER_LEN);
    do
    {
        ret = Ql_SOC_Recv(socketId, m_SocketRcvBuf + offset, SOC_RECV_BUFFER_LEN - offset);
        APP_DEBUG("ret=%d\r\n",ret);
        if((ret < SOC_SUCCESS) && (ret != SOC_WOULDBLOCK))
        {
            APP_DEBUG("<-- Fail to receive data, cause=%d.-->\r\n",ret);
            APP_DEBUG("<-- Close socket.-->\r\n");
            Ql_SOC_Close(socketId);
            m_SocketId = -1;
            break;
        }
        else if(SOC_WOULDBLOCK == ret)  // Read finish
        {
            APP_DEBUG("<-- Receive data from server,len(%d):%s\r\n", offset, m_SocketRcvBuf);
	     recv_buf = NewBuffer();
	     APP_DEBUG(" recv_buf->_write_pos = %d,  buf->_capacity = %d, ret = %d, ret ,offset = %d\r\n ",recv_buf->_write_pos, recv_buf->_capacity,ret,offset);
	     WriteBytes(recv_buf, m_SocketRcvBuf, offset);
	     while(1){
	        if ((pkg = GetEdpPacket(recv_buf)) == 0)
		 {
		     break;
	        }
                   APP_DEBUG("GetEdpPacket\r\n");
		     mtype = EdpPacketType(pkg);	 
		     switch(mtype)
		    {
		       case CONNRESP: 
			      APP_DEBUG("CONNRESP\r\n");
                           rtn = UnpackConnectResp(pkg);
                           APP_DEBUG("recv connect resp, rtn: %d\n", rtn);
                     break;

                    case UPDATERESP:					
			    APP_DEBUG("UPDATERESP\r\n");
                         UnpackUpdateResp(pkg, &up_info);
                         while (up_info){
                         APP_DEBUG("name = %s\n", up_info->name);
                         APP_DEBUG("version = %s\n", up_info->version);
                         APP_DEBUG("url = %s\nmd5 = ", up_info->url);
			    //Ql_sprintf(m_URL_Buffer,"%s",up_info->url);//firmwareInfomation

			   if( Ql_strcmp(firmwareInfomation.version,up_info->version) )
			   {
			        updataFlag = 1;
			        Ql_sprintf(firmwareInfomation.name,"%s",up_info->name);
			        Ql_sprintf(firmwareInfomation.version,"%s",up_info->version);
				 Ql_sprintf(m_URL_Buffer,"%s",up_info->url);//firmwareInfomation
			        APP_DEBUG("name = %s\r\n", firmwareInfomation.name);
			        APP_DEBUG("version = %s\r\n", firmwareInfomation.version);
			        APP_DEBUG("url = %s\nmd5 = ", up_info->url);				 
			   }

				  
                         for (i=0; i<32; ++i){
                              APP_DEBUG("%c", (char)up_info->md5[i]);
                        }
                        APP_DEBUG("\n");
                       up_info = up_info->next;
                     }
                     FreeUpdateInfolist(up_info);
                   break;
		   }
		   DeleteBuffer(&pkg);
		   //break;
		}
		APP_DEBUG("\r\n delete recv buf\r\n"); 
	       DeleteBuffer(&recv_buf);
		break;
	     } 

        else // Continue to read...
        {
            if (SOC_RECV_BUFFER_LEN == offset)  // buffer if full
            {
                APP_DEBUG("<-- Receive data from server,len(%d):%s\r\n", offset, m_SocketRcvBuf);		 
               Ql_memset(m_SocketRcvBuf, 0, SOC_RECV_BUFFER_LEN);
                offset = 0;
            }else{
                offset += ret;
            }
            continue;
        }
    } while (TRUE);

  // if(Ql_strstr(m_URL_Buffer,"http://"))
   {
       //Close socket
       ret = Ql_SOC_Close(m_SocketId);
       APP_DEBUG("<-- Close socket[%d], cause=%d --> \r\n", m_SocketId, ret);

       //Deactivate GPRS
      APP_DEBUG("<-- Deactivating GPRS... -->\r\n");
      ret = Ql_GPRS_DeactivateEx(pdpCntxtId, TRUE);
      APP_DEBUG("<-- Deactivated GPRS, cause=%d -->\r\n\r\n", ret);
	if(1 == updataFlag)
	{
	      updataFlag = 0;
             APP_DEBUG("send msg to main_task_id\r\n");
             Ql_OS_SendMessage(main_task_id, Firewarm_Updata_READY, 1, 1);
             if(iRet <0)
                {
                    APP_DEBUG("\r\n<--failed!!, Ql_OS_SendMessagefail,  ret=%d-->\r\n", iRet);
                }
                APP_DEBUG("\r\n<--Ql_OS_SendMessage ret=%d-->\r\n", iRet);
	     OTA_main();
       }
   }     
}
//
//
// This callback function is invoked in the following case:
// The return value is less than the data length to send when calling Ql_SOC_Send(), which indicates
// the socket buffer is full. Application should stop sending socket data till this callback function
// is invoked, which indicates application can continue to send data to socket.
static void Callback_Socket_Write(s32 socketId, s32 errCode, void* customParam)
{
    if (errCode < 0)
    {
        APP_DEBUG("<-- Socket error(error code:%d), close socket.-->\r\n", errCode);
        Ql_SOC_Close(socketId);
        m_SocketId = -1;        
    }else{
        APP_DEBUG("<-- You can continue to send data to socket -->\r\n");
    }
}

//#if 0
void EDP_Loop(u8 ch)
{
       dev_param.domain[0] = 1;
       Ql_strcpy(dev_param.id,"23181742");//调试用//23181742    23875373(http)
       Ql_strcpy(dev_param.appkey,"92Z97MHuN0uNvRUgRzMjoXw2JTE=");//调试用92Z97MHuN0uNvRUgRzMjoXw2JTE=           3z43c2Zt2=T8lLsFAnJaCyHqwVc=(http)

	switch(ch){
		case DEP_DATA:
			Connect_RequestType1(dev_param.id, dev_param.appkey);
			APP_DEBUG("<-- ping server -->\r\n");
			Ping_Server();
			
		      Send_DataToOneNet();

			break;
		case DEP_GPS:
		    // if((uiUSRLong)||(uiUSRLat))
		     	{
		     	   APP_DEBUG("<--GPS data to ONENET-->\r\n");
			   Save_GpsToOneNet();
		     	}
   
			break;
		case DEP_METER:
			APP_DEBUG("<--Meter data to ONENET-->\r\n");
			Connect_RequestType1(dev_param.id, dev_param.appkey);
			APP_DEBUG("<-- ping server -->\r\n");
			Ping_Server();
			TaxiMeterLoop();
		break;
		
		default:
			break;
	}

}


u8 Connect_RequestType1(u8 *devid, u8 *api_key)
{
char str[200] = "+0wereosdfnsdf";

    EdpPacket *send_pkg;
    APP_DEBUG("<-- ready to connect -->\r\n");


    send_pkg = PacketConnect1((const char* )devid, (const char* )api_key);
   
    if(send_pkg == NULL)
    {
        APP_DEBUG("web connent fail\r\n");
        return 0;
    }


    /* 向设备云发送连接请求 */
    APP_DEBUG("send connect to server, bytes: %d\n", send_pkg->_write_pos);

    DoSend(0, (const u8 *)send_pkg->_data, send_pkg->_write_pos);

    /* 必须释放这个内存，否则造成泄露 */
    DeleteBuffer(&send_pkg);
}
/*
 *  @brief  发送PING包维持心跳
 */
u8 Ping_Server(void)
{

    u8 ret;

    EdpPacket *send_pkg;
    APP_DEBUG("%s %d\n", __func__, __LINE__);
    /* 组装ping包 */
    send_pkg = PacketPing();

    ret = DoSend(0, (const u8 *)send_pkg->_data,
           send_pkg->_write_pos);
    //mDelay(500);
    /* 必须释放这个内存，否则造成泄露 */
    DeleteBuffer(&send_pkg);
    //mDelay(100);
	return ret;
}

u8 Save_GpsToOneNet(void)
{
//char str[300] = "POST /devices/3123852/datapoints HTTP/1.1\r\napi-key: rHPJq8oZ6ydLpfUjU0A7yMbLEFs=\r\nHost: api.heclouds.com\r\nContent-Length: 91\r\n\r\n{\"datastreams\":[{\"id\":\"gps\",\"datapoints\":[{\"value\":{\"lon\":";

       char str[512] = "POST /devices/";
       char buf[30]; 
       char domianbuf[6];

	u64  ackNum = 0;
       //u32 uiUSROLEDLat = 0;
      // u32 uiUSROLEDLong = 0;
       s32 ret;
       //uiUSROLEDLat    = 116200000;
	//uiUSROLEDLong  = 22000000;

	//APP_DEBUG("\r\nYUN_DEV_ID:%s\r\n",dev_param.id);
	//APP_DEBUG("\r\nYUN_DEV_APPKey:%s\r\n",dev_param.appkey);

	domianbuf[0] = (u8)(dev_param.domain[0]/10000+0x30);//域ID
	domianbuf[1] = (u8)((dev_param.domain[0]%10000)/1000+0x30);
	domianbuf[2] = (u8)((dev_param.domain[0]%1000)/100+0x30);
	domianbuf[3] = (u8)((dev_param.domain[0]%100)/10+0x30);
	domianbuf[4] = (u8)(dev_param.domain[0]%10+0x30);
	domianbuf[5] = '\0';
	
	Ql_strcat(str,dev_param.id); 
	Ql_strcat(str,"/datapoints HTTP/1.1\r\napi-key: ");
	Ql_strcat(str,dev_param.appkey);
	Ql_strcat(str,"\r\nHost: api.heclouds.com\r\nContent-Length: 97\r\n\r\n{\"datastreams\":[{\"id\":\"");
	Ql_strcat(str,domianbuf);
	Ql_strcat(str,"-21-g\",\"datapoints\":[{\"value\":{\"lon\":");
		
	Ql_sprintf(buf,"%-9.6f", ((float)uiUSRLong/1000000)); //保留六位小数，第三位四舍五入
	Ql_strcat(str,buf); 
	Ql_strcat(str,",\"lat\":"); 
	
	Ql_sprintf(buf,"%-8.6f", ((float)uiUSRLat/1000000)); //保留六位小数，第三位四舍五入
	Ql_strcat(str,buf); 
	Ql_strcat(str,"}}]}]}\r\n"); 

	Ql_UART_Write(UART_PORT1,str, Ql_strlen(str));
	APP_DEBUG("<-- Send total data  %d. --> \r\n",Ql_strlen(str));
	
	//if((uiUSRLong)&&(uiUSRLat))
	{
        APP_DEBUG("<-- m_SocketId  %d. --> \r\n",m_SocketId);
		 ret = Ql_SOC_Send(m_SocketId, (u8*)str, Ql_strlen(str));
		 APP_DEBUG("<-- ret return value  %d. --> \r\n",ret);
	        if (ret == Ql_strlen(str))
	        {
	            APP_DEBUG("<-- Send socket data successfully. --> \r\n");
	        }else{
	            APP_DEBUG("<-- Fail to send socket data. --> \r\n");
	            Ql_SOC_Close(m_SocketId);
	            return;
	        }
	}

       //7.2 Check ACK number
        do 
        {
            ret = Ql_SOC_GetAckNumber(m_SocketId, &ackNum);
            APP_DEBUG("<-- Current ACK Number:%llu/%d --> \r\n", ackNum, Ql_strlen(str));
            Ql_Sleep(500);
        } while (ackNum != Ql_strlen(str));
        APP_DEBUG("<-- Server has received all data --> \r\n");

     //8. Close socket
     ret = Ql_SOC_Close(m_SocketId);
     APP_DEBUG("<-- Close socket[%d], cause=%d --> \r\n", m_SocketId, ret);

     //9. Deactivate GPRS
     APP_DEBUG("<-- Deactivating GPRS... -->\r\n");
     ret = Ql_GPRS_DeactivateEx(pdpCntxtId, TRUE);
     APP_DEBUG("<-- Deactivated GPRS, cause=%d -->\r\n\r\n", ret);

	
}


u8 Send_DataToOneNet(void )
{
     double temp;
    s32 ret;
    u64  ackNum = 0;
    EdpPacket* send_pkg;
    ST_Time stdt;
    u8 stream[100] = ",;";
    u8 stream_name[11] = { 0 };
    u8 time[20] = { 0 };
    float test_val = 29.0;
    u8 buf[10] = {0};
#if 0	
   //char str[512] = ",;temperature,2015-03-22 22:22:22,22.5;at,2013-04-22 22:22:22";
   char stream[512] = ",;temperature,2017-12-29 22:22:22,22.5";
   //send_pkg = PacketSavedataSimpleString(NULL, (const int8_t*)",;temperature,2015-03-22 22:22:22,22.5;at,2013-04-22 22:22:22");
 #endif 


    	stream_name[0] = (u8)(dev_param.domain[0]/10000+0x30);//域ID
	stream_name[1] = (u8)((dev_param.domain[0]%10000)/1000+0x30);
	stream_name[2] = (u8)((dev_param.domain[0]%1000)/100+0x30);
	stream_name[3] = (u8)((dev_param.domain[0]%100)/10+0x30);
	stream_name[4] = (u8)(dev_param.domain[0]%10+0x30);
	stream_name[5] = '-';
	stream_name[6] = ' ';
	stream_name[7] = ' ';
	stream_name[8] = '-';
	stream_name[9] = ' ';
	stream_name[10] = '\0';


       stdt =  Get_Local_Time();
       stdt.year = stdt.year % 100;
	time[0] = '2';
	time[1] = '0';
	time[2] = (u8)(stdt.year/10+0x30);
	time[3] = (u8)(stdt.year%10+0x30);
	time[4] = '-';
	time[5] = (u8)(stdt.month/10+0x30);
	time[6] = (u8)(stdt.month%10+0x30);
	time[7] = '-';
	time[8] = (u8)(stdt.day/10+0x30);
	time[9] = (u8)(stdt.day%10+0x30);
	time[10] = ' ';
	time[11] = (u8)(stdt.hour/10+0x30);
	time[12] = (u8)(stdt.hour%10+0x30);
	time[13] = ':';
	time[14] = (u8)(stdt.minute/10+0x30);
	time[15] = (u8)(stdt.minute%10+0x30);
	time[16] = ':';
	time[17] = (u8)(stdt.second/10+0x30);
	time[18] = (u8)(stdt.second%10+0x30);
	time[19] = '\0';

	Ql_strcat((char *)stream,(char *)stream_name);
	Ql_strcat((char *)stream,",");
	Ql_strcat((char *)stream,(char *)time);
	Ql_strcat((char *)stream,",");

	stream[8] = 0x30;//节点ID
	stream[9] = 0x34;
	stream[11] = 'V';
	stream[33] = '\0';
	
	Ql_sprintf(buf,"%-5.2f", Power_voltage);
	Ql_strcat((char *)stream,buf);

    APP_DEBUG("send data %s\r\n",stream);
    send_pkg = PacketSavedataSimpleString(NULL, (const int8_t*)stream);
    DoSend(m_SocketId, (const u8 *)send_pkg->_data, send_pkg->_write_pos);
    DeleteBuffer(&send_pkg);

	 do 
        {
            ret = Ql_SOC_GetAckNumber(m_SocketId, &ackNum);
            APP_DEBUG("<-- Current ACK Number:%llu/%d --> \r\n", ackNum, Ql_strlen(stream)+6);
            Ql_Sleep(500);
        } while (ackNum != Ql_strlen(stream)+6+2+51);
        APP_DEBUG("<-- Server has received all data --> \r\n");


     //8. Close socket
     ret = Ql_SOC_Close(m_SocketId);
     APP_DEBUG("<-- Close socket[%d], cause=%d --> \r\n", m_SocketId, ret);

     //9. Deactivate GPRS
     APP_DEBUG("<-- Deactivating GPRS... -->\r\n");
     ret = Ql_GPRS_DeactivateEx(pdpCntxtId, TRUE);
     APP_DEBUG("<-- Deactivated GPRS, cause=%d -->\r\n\r\n", ret);
    
}


 void MeterRecordSendOneNet(u8 ucMethod,u8 ucBit,u8 ucflag)
{

	EdpPacket* send_pkg;
	u8 stream_name[11] = { 0 };
	u8 stream[100] = ",;";
	u8 time[20] = { 0 };
	u8 ucFlag = 0;
	u16 Tmplen = 0;
	u8 buf[2] = {0x30,0x31};
	s32 ret;
       u64 ackNum;
	    APP_DEBUG("\r\nYUN_DEV_ID:%s\r\n",dev_param.id);
	    APP_DEBUG("\r\nYUN_DEV_APPKey:%s\r\n",dev_param.appkey);


	   Ql_sprintf(&stream_name[0],"%05d-  - ",dev_param.domain[0]);
	   Ql_strcat(stream,stream_name);
	    Ql_strcat(stream,",");
		Tmplen = Ql_strlen(stream);
	//    APP_DEBUG("Tmplen = %d",Tmplen);

	    //载客命令
	    if((ucBit&0x01) != 0){
	    	stream[8] = '4';//节点ID:53
	    	stream[9] = (u8)(0x33);
	    	stream[11] = 'z';
	    	stream[Tmplen] = '\0';
	    	Ql_strcat(stream,buf);
		APP_DEBUG("stream = %s,len = %d\r\n",stream,Ql_strlen(stream));
		
	        if(ucMethod == EDPDATASEND)
		{
	        	send_pkg = PacketSavedataSimpleString(NULL, (const int8_t*)stream);
	        	ucFlag =DoSend(0, (const u8 *)send_pkg->_data, send_pkg->_write_pos);
	        	DeleteBuffer(&send_pkg);
			 do 
		        {
		            ret = Ql_SOC_GetAckNumber(m_SocketId, &ackNum);
		            APP_DEBUG("<-- Current ACK Number:%llu/%d --> \r\n", ackNum, Ql_strlen(stream)+6+2+51);
		            Ql_Sleep(500);
		        } while (ackNum != Ql_strlen(stream)+6+2+51);
		        APP_DEBUG("<-- Server has received all data --> \r\n");	
	        }
	        
	//        if(ucFlag == 0)
	//            usrsFlash_SaveDatas(USRFLASHMSG_DAT,stream,strlen(stream),NULL,0);
	    }
	    
	    //每载客车次结算(运营数据)
	    if((ucBit&0x02) != 0){
	    	stream[8] = '4';//节点ID:43
	    	stream[9] = (u8)(0x33);
	    	stream[11] = 'm';
	    	stream[Tmplen] = '\0';
	    	Ql_strcat(stream,taxiBuf);
		APP_DEBUG("stream = %s,len = %d\r\n",stream,Ql_strlen(stream));
		
	        if(ucMethod == EDPDATASEND){
	        	send_pkg = PacketSavedataSimpleString(NULL, (const int8_t*)stream);
	        	ucFlag =DoSend(0, (const u8 *)send_pkg->_data, send_pkg->_write_pos);
	        	DeleteBuffer(&send_pkg);
			 do 
		        {
		            ret = Ql_SOC_GetAckNumber(m_SocketId, &ackNum);
		            APP_DEBUG("<-- Current ACK Number:%llu/%d --> \r\n", ackNum, Ql_strlen(stream)+6);
		            Ql_Sleep(500);
		        } while (ackNum != Ql_strlen(stream)+6+2+51);
		        APP_DEBUG("<-- Server has received all data --> \r\n");				
	        //if(ucFlag == 0)
	            //usrsFlash_SaveDatas(USRFLASHMSG_DAT,stream,strlen(stream),NULL,0);
	    }

	}

     //8. Close socket
     ret = Ql_SOC_Close(m_SocketId);
     APP_DEBUG("<-- Close socket[%d], cause=%d --> \r\n", m_SocketId, ret);

     //9. Deactivate GPRS
     APP_DEBUG("<-- Deactivating GPRS... -->\r\n");
     ret = Ql_GPRS_DeactivateEx(pdpCntxtId, TRUE);
     APP_DEBUG("<-- Deactivated GPRS, cause=%d -->\r\n\r\n", ret);
 }


s32 DoSend(s32 sockfd, const u8 *buffer, u32 len)
{

       s32 ret;
	ret = Ql_SOC_Send(m_SocketId, buffer, len);
	if(len == ret)
      {
            APP_DEBUG("<-- Send socket data successfully. --> \r\n");
      }
	else
	{
	    APP_DEBUG("<-- Fail to send socket data. --> \r\n");
	}
	APP_DEBUG("<-- return value %d --> \r\n",ret);
	return ret;
}




void GPSToOneNet_Hander(void )
{
      m_SrvADDR[20] = "183.230.40.33\0";
      m_SrvPort = 80;

    if((uiUSRLong)||(uiUSRLat))
    {
       if(TCP_Server() == TRUE)
	{
		EDP_Loop(DEP_GPS);
	}
       else{
           APP_DEBUG("<-- TCP GPS server fail-->\r\n");
       }
   }
}


void DATAToOneNet_Hander(void )
{
      m_SrvADDR[20] = "183.230.40.39\0";
      m_SrvPort = 876;
       if(TCP_Server() == TRUE)
	{
		EDP_Loop(DEP_DATA);
	}
       else{
           APP_DEBUG("<-- TCP Data server fail-->\r\n");
       }
}

void MeterDATAToOneNet_Hander(void )
{
      m_SrvADDR[20] = "183.230.40.39\0";
      m_SrvPort = 876;
       if(TCP_Server() == TRUE)
	{
		EDP_Loop(DEP_METER);
	}
       else{
           APP_DEBUG("<-- TCP Data server fail-->\r\n");
       }
}
void edptest(void )
{
    APP_DEBUG("EDP test ok\r\n");
}




void FOTA_Program(void )
{
    s32 ret;
    u64  ackNum = 0;
    EdpPacket* send_pkg;
    UpdateInfoList* testBuf = NULL;
    testBuf = (UpdateInfoList* )Ql_MEM_Alloc(sizeof(UpdateInfoList));
    testBuf->name = "MC20";
    testBuf->version = "0000" ;
     Ql_strcpy(dev_param.id,"23181742");//调试用
     Ql_strcpy(dev_param.appkey,"92Z97MHuN0uNvRUgRzMjoXw2JTE=");//调试用
     if(TCP_Server() == TRUE)
     {
         Connect_RequestType1(dev_param.id, dev_param.appkey);
	  APP_DEBUG("<-- ping server -->\r\n");
	  Ping_Server();
	  		
     
        send_pkg = PacketUpdateReq(testBuf);
        DoSend(m_SocketId, (const u8 *)send_pkg->_data, send_pkg->_write_pos);
        DeleteBuffer(&send_pkg);
	 Ql_MEM_Free(testBuf);

	 do 
        {
            ret = Ql_SOC_GetAckNumber(m_SocketId, &ackNum);
            APP_DEBUG("<-- Current ACK Number:%llu/%d --> \r\n", ackNum, 67);
            Ql_Sleep(500);
        } while (ackNum != 14+2+51);
        APP_DEBUG("<-- Server has received all data --> \r\n");

/*
     //8. Close socket
     ret = Ql_SOC_Close(m_SocketId);
     APP_DEBUG("<-- Close socket[%d], cause=%d --> \r\n", m_SocketId, ret);

     //9. Deactivate GPRS
     APP_DEBUG("<-- Deactivating GPRS... -->\r\n");
     ret = Ql_GPRS_DeactivateEx(pdpCntxtId, TRUE);
     APP_DEBUG("<-- Deactivated GPRS, cause=%d -->\r\n\r\n", ret);
*/     
  }
}


s32 EDP_GetAuth(void )
{
    if(m_SocketId >= 0){
	return TRUE;
    }else
	return FALSE;
}
//#endif
