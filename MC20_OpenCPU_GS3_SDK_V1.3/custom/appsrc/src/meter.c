#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_trace.h"
#include "ql_uart.h"
#include "ql_system.h"
#include "ql_type.h"

#include "Debug.h"
#include "Meter.h"
#include "EdpDemo.h"
#include "EdpKit.h"
#include "Meter.h"

u8 taxiBuf[50] = {0};
u8 taxiMeterFlag = 0;
u8 taxiPassengerFlag = 0;



static Enum_SerialPort m_MeterUartPort  = UART_PORT2;
static u8 m_RxBuf_Uart2[SERIAL_RX_BUFFER_LEN];
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);

void MeterSerialInit(void )
{
    s32 ret;
    ret = Ql_UART_Register(m_MeterUartPort, CallBack_UART_Hdlr, NULL);
    if (ret < QL_RET_OK)
    {
        //APP_DEBUG("Fail to register serial port[%d], ret=%d\r\n", m_MeterUartPort, ret);
    }
    ret = Ql_UART_Open(m_MeterUartPort, 115200, FC_NONE);
    if (ret < QL_RET_OK)
    {
       // APP_DEBUG("Fail to open serial port[%d], ret=%d\r\n", m_MeterUartPort, ret);
    }
}


//将将两个字符组合成一个十六进制数
//bytebuf数组元素是hexbuf元素的两倍
void twobytes2onehex(u8* bytebuf,u8* hexbuf)
{
  u8 i,j,k,temp[2];

  for(k = 0;k < sizeof(hexbuf);k++){
    j = k*2;
    for(i=0;i<2;i++)
    {
      if(bytebuf[j+i]>='0' && bytebuf[j+i]<='9') temp[i]=bytebuf[j+i]-'0';
      else if(bytebuf[j+i]>='A' && bytebuf[j+i]<='F') temp[i]=bytebuf[j+i]-'A'+0x0A;
      else if(bytebuf[j+i]>='a' && bytebuf[j+i]<='f') temp[i]=bytebuf[j+i]-'a'+0x0a;
      else return;
    }
    hexbuf[k] = temp[0]*16 + temp[1];
  }
  
}


//累加和校验
static int AccumulatingAndChecking(unsigned char buf[],long bytestotal)
{
int buff = 0;
int i;

    for (i = 0; i < bytestotal; i++){
        buff = (buff + (int) buf[i]) % 256;
    }
    return buff;
    
}

//计价器通讯解析
static void MeterSerialDataAnalysis(u8 *data, u16 n_byte)
{
	u8 rr_data[280] ={0};
	u16 flow_num = 0,i;
	u8 *p;
	u8 len = 0;
	u8 buf[2] = {0},buf1[1] = {0};
	u8 crcbuf[2] = {0},crcbuf1[1] = {0};

	flow_num = n_byte;

	for(i=0;i<flow_num;i++){
	     rr_data[i] = data[i];
      }
		

       for(i=0;i<flow_num;i++){
            if((rr_data[i]==0x02)&&(rr_data[i+1]==0x35)&&(rr_data[i+2]==0x38)){//载客命令
                p=rr_data+i;
                len = 2;
                if(*(p+len+3) == 0x03){
                    crcbuf[0] = *(p+len+4);
                    crcbuf[1] = *(p+len+5);
                    twobytes2onehex(crcbuf,crcbuf1);
                    if(crcbuf1[0] == (u8)AccumulatingAndChecking(p+1,(len+2))){
                        if((0x30 == *(p+3))&&(0x30 == *(p+4))){
                            taxiPassengerFlag = 1;
                        }
                    }
                }        
		}
				
		if((rr_data[i]==0x02)&&(rr_data[i+1]==0x35)&&(rr_data[i+2]==0x30)){//每载客车次结算
		     p=rr_data+i;
                   buf[0] = *(p+3);
                   buf[1] = *(p+4); 
                  twobytes2onehex(buf,buf1);
                  len = buf1[0]*2;
                  if(*(p+len+5) == 0x03){
                      crcbuf[0] = *(p+len+6);
                       crcbuf[1] = *(p+len+7);
                      twobytes2onehex(crcbuf,crcbuf1);
                    if(crcbuf1[0] == (u8)AccumulatingAndChecking(p+1,(len+4))){
                        Ql_strncpy(taxiBuf,p+5,len);
                        taxiMeterFlag = 1;
                    }
                }
	      }
      }	

}


/*
static void MeterRecordSendOneNet(u8 ucMethod,u8 ucBit,u8 ucflag)
{

EdpPacket* send_pkg;
u8 stream_name[11] = { 0 };
u8 stream[100] = ",;";
u8 time[20] = { 0 };
u8 ucFlag = 0;
u16 Tmplen = 0;
u8 buf[2] = {0};
s32 ret,ackNum;

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
	
	
        if(ucMethod == EDPDATASEND)
	{
        	send_pkg = PacketSavedataSimpleString(NULL, (const int8_t*)stream);
        	ucFlag =DoSend(0, (const u8 *)send_pkg->_data, send_pkg->_write_pos);
        	DeleteBuffer(&send_pkg);
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
	APP_DEBUG("stream = %s",stream);
	
        if(ucMethod == EDPDATASEND){
        	send_pkg = PacketSavedataSimpleString(NULL, (const int8_t*)stream);
        	ucFlag =DoSend(0, (const u8 *)send_pkg->_data, send_pkg->_write_pos);
        	DeleteBuffer(&send_pkg);
        //if(ucFlag == 0)
            //usrsFlash_SaveDatas(USRFLASHMSG_DAT,stream,strlen(stream),NULL,0);
    }

}

*/

void TaxiMeterLoop(void )
{
    static u8 i = 0;
  /*  
    MeterSerialDataAnalysis(data,n_byte);
     //if(TCP_Server() == TRUE)
     {
	    if(1 == taxiPassengerFlag){       
	            MeterRecordSendOneNet(EDPDATASEND,1,1);
	            i = 1;
	            taxiPassengerFlag = 0;
	    }
	    
	    if(1 == taxiMeterFlag){	    
	            if(1 == i){
	               // MeterRecordSendOneNet(EDPDATASEND,3,0);
	    			taxiMeterFlag = 0;
	                i = 0;
	            }else{
	                //MeterRecordSendOneNet(EDPDATASEND,2,0);
	                taxiMeterFlag = 0;
	                i = 0;
	            }

	        }
      }
      else{
	  	//MeterRecordSendOneNet(EDPDATASAVE,2,0);
	       taxiMeterFlag = 0;
      	}
 */
    if(1 == taxiPassengerFlag){
        
        if(EDP_GetAuth() == TRUE){
            MeterRecordSendOneNet(EDPDATASEND,1,1);
            i = 1;
            taxiPassengerFlag = 0;
        }
    }
    
    if(1 == taxiMeterFlag){
        if(EDP_GetAuth() == TRUE){
            if(1 == i){
                MeterRecordSendOneNet(EDPDATASEND,3,0);
    			taxiMeterFlag = 0;
                i = 0;
            }else{
                MeterRecordSendOneNet(EDPDATASEND,2,0);
                taxiMeterFlag = 0;
                i = 0;
            }
        }else{
            MeterRecordSendOneNet(EDPDATASAVE,2,0);
			taxiMeterFlag = 0;
        }
    }
}


static s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/u8* pBuffer, /*[in]*/u32 bufLen)
{
    s32 rdLen = 0;
    s32 rdTotalLen = 0;
    if (NULL == pBuffer || 0 == bufLen)
    {
        return -1;
    }
    Ql_memset(pBuffer, 0x0, bufLen);
    while (1)
    {
        rdLen = Ql_UART_Read(port, pBuffer + rdTotalLen, bufLen - rdTotalLen);
        if (rdLen <= 0)  // All data is read out, or Serial Port Error!
        {
            break;
        }
        rdTotalLen += rdLen;
        // Continue to read...
    }
    if (rdLen < 0) // Serial Port Error!
    {
        APP_DEBUG("Fail to read from port[%d]\r\n", port);
        return -99;
    }
    return rdTotalLen;
}

static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
    //APP_DEBUG("CallBack_UART_Hdlr: port=%d, event=%d, level=%d, p=%x\r\n", port, msg, level, customizedPara);
    switch (msg)
    {
    case EVENT_UART_READY_TO_READ:
        {
            if (m_MeterUartPort == port)
            {
                s32 totalBytes = ReadSerialPort(port, m_RxBuf_Uart2, sizeof(m_RxBuf_Uart2));
                if (totalBytes <= 0)
                {
                    APP_DEBUG("<-- No data in UART buffer! -->\r\n");
                    return;
                }
                {// Read data from UART
                    Ql_UART_Write(m_MeterUartPort, m_RxBuf_Uart2, totalBytes);
		      MeterSerialDataAnalysis(m_RxBuf_Uart2,totalBytes);
                    MeterDATAToOneNet_Hander();

                }
            }
            break;
        }
    case EVENT_UART_READY_TO_WRITE:
        break;
    default:
        break;
    }
}
