#include "string.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_uart.h"
#include "ql_fs.h"
#include "ql_error.h"
#include "ql_system.h"
#include "ql_stdlib.h"

#include "function.h"
#include "debug.h"


//#define __TEST_FOR_RAM_FILE__
//#define __TEST_FOR_UFS__
#define __TEST_FOR_MEMORY_CARD__

#if defined (__TEST_FOR_RAM_FILE__)
#define  PATH_ROOT    		((u8 *)"RAM:")
#elif defined (__TEST_FOR_UFS__)
#define  PATH_ROOT    		((u8 *)"myroot")
#else   //!__TEST_FOR_MEMORY_CARD__
#define  PATH_ROOT    		((u8 *)"SD:")
#endif


#define LENGTH 100


firmwareInfo firmwareInfomation;
u8 filename[] = "test.txt";
u8 datafile[] = "data.txt";

void parameterConfigure(void )
{
    s32 ret = -1;
    s32 handle = -1;
    u32 writenLen = 0;
    u32 readenLen = 0;
    s32 filesize = 0;
    s32 position = 0;    
    bool isdir = FALSE;
       
    u8 strBuf[LENGTH] = {0};
    u8 filePath[LENGTH] = {0};
    u8 fileNewPath[LENGTH] = {0};
    u8  writeBuffer[LENGTH] = {"This is test data!"};
    u8  file_buff[LENGTH] ={0};
	
	 #if defined (__TEST_FOR_RAM_FILE__)
	    Enum_FSStorage storage = Ql_FS_RAM;
        #elif defined (__TEST_FOR_UFS__)
	     Enum_FSStorage storage = Ql_FS_UFS;
        #elif defined (__TEST_FOR_MEMORY_CARD__)
	     Enum_FSStorage storage = Ql_FS_SD;
        #else
	     APP_DEBUG("\r\n<--Format does not define.-->\r\n");
       #endif

      ret = Ql_FS_Format(storage);
      if(ret==QL_RET_OK)
      {
            APP_DEBUG("\r\n<-- Ql_FS_Format(storage=%d).Format OK! -->\r\n",storage);
      }
      else
      {
           APP_DEBUG("\r\n<--Format error.-->\r\n");
      }	

//#if 0
     Ql_strncpy(filePath,filename,Ql_strlen(filename));
     #ifdef __TEST_FOR_RAM_FILE__
               handle = Ql_FS_OpenRAMFile(filePath,QL_FS_CREATE|QL_FS_READ_WRITE, LENGTH);
     #else
               handle = Ql_FS_Open(filePath,QL_FS_READ_WRITE );
     #endif
     if(handle<0)
     {
           #ifdef __TEST_FOR_RAM_FILE__
                   handle = Ql_FS_OpenRAMFile(filePath,QL_FS_CREATE_ALWAYS, LENGTH);
          #else
                   handle = Ql_FS_Open(filePath,QL_FS_CREATE_ALWAYS); //if file does not exist ,creat it
          #endif
          if(handle>0)
          {
                  APP_DEBUG("\r\n<-- Create file (%s) OK! handle =%d -->\r\n",filePath,handle);
          }
          else
          {
                  APP_DEBUG("\r\n<-- failed!! Create file (%s) fail-->",filePath);
          }
     }
     else
     {
             APP_DEBUG("\r\n<--The file has exist-->\r\n");
     }
     Ql_FS_Close(handle);
     handle=-1;

#if 0
    #ifdef __TEST_FOR_RAM_FILE__
            handle = Ql_FS_OpenRAMFile(filePath,QL_FS_CREATE|QL_FS_READ_WRITE, LENGTH);
    #else
            handle = Ql_FS_Open(filePath,QL_FS_READ_WRITE);
   #endif
   if(handle<0)
   {
           APP_DEBUG("\r\n<--The file does not exist,handle:%d-->\r\n",handle);

   }
   else
  {
         APP_DEBUG("\r\n<--Please write:-->\r\n");
         //Ql_strncpy(firmwareInfomation.name,"MC20",Ql_strlen("MC20"));
	  //Ql_strncpy(firmwareInfomation.version,"0000",Ql_strlen("0000"));
         Ql_strcpy(firmwareInfomation.name,"MC20");
	  Ql_strcpy(firmwareInfomation.version,"0000");
         Ql_sprintf(file_buff,"firm_name:%s\r\nfirm_version:%s\r\n",firmwareInfomation.name,firmwareInfomation.version);
	  APP_DEBUG("%s\r\n",file_buff);
	  //Ql_sprintf(file_buff,"firm_version:%s\r\n",firmwareInfomation.version);
        ret = Ql_FS_Seek(handle,0,QL_FS_FILE_END);  //seek end 
       ret = Ql_FS_Write(handle, file_buff, Ql_strlen(file_buff), &writenLen);
      APP_DEBUG("\r\n<--!! Ql_FS_Write  ret =%d  writenLen =%d-->\r\n",ret,writenLen);
      Ql_FS_Flush(handle); //fflush
      Ql_FS_Close(handle);//close the file
      handle=-1;
		                             
   }

#endif    
}

void parameterWrite(void )
{
    s32 handle = -1,ret;
    u32 writenLen = 0;
    u8  write_buff[LENGTH] ={0};
 #ifdef __TEST_FOR_RAM_FILE__
     handle = Ql_FS_OpenRAMFile(filename,QL_FS_CREATE|QL_FS_READ_WRITE, LENGTH);
#else
    handle = Ql_FS_Open(filename,QL_FS_READ_WRITE);
#endif
if(handle<0)
{
   APP_DEBUG("\r\n<--The file does not exist,handle:%d-->\r\n",handle);
}
else
{
   APP_DEBUG("\r\n<--Please write:-->\r\n");
   ret = Ql_FS_Seek(handle,0,QL_FS_FILE_END);  //seek end 
APP_DEBUG("%s,%s\r\n", firmwareInfomation.name,firmwareInfomation.version);
Ql_sprintf(write_buff,"firm_name:%s\r\nfirm_version:%s\r\n",firmwareInfomation.name,firmwareInfomation.version);
APP_DEBUG("write_buff:%s\r\n",write_buff);
ret = Ql_FS_Write(handle, write_buff, Ql_strlen(write_buff), &writenLen);
APP_DEBUG("\r\n<--!! Ql_FS_Write  ret =%d  writenLen =%d-->\r\n",ret,writenLen);
                    
Ql_FS_Flush(handle); //fflush
Ql_FS_Close(handle);//close the file  
handle=-1;
                      
}
}

void parameterRead(void )
{
    s32 handle = -1,ret;
    u32 readenLen = 0;
    u8 position=0;
    u8  read_buff[LENGTH] ={0};
	

 #ifdef __TEST_FOR_RAM_FILE__
     handle = Ql_FS_OpenRAMFile(filename,QL_FS_CREATE|QL_FS_READ_WRITE, LENGTH);
#else
    handle = Ql_FS_Open(filename,QL_FS_READ_WRITE);
#endif
if(handle<0)
{
   APP_DEBUG("\r\n<--The file does not exist,handle:%d-->\r\n",handle);
}
else
{
   APP_DEBUG("\r\n<-- The read:-->\r\n");
                      
}

ret = Ql_FS_Seek(handle,0,QL_FS_FILE_BEGIN); 
ret = Ql_FS_Read(handle, read_buff, LENGTH-1, &readenLen);
APP_DEBUG("%s,%d",read_buff,readenLen);
while(readenLen>=(LENGTH-1))
{
    Ql_memset(read_buff, 0x0, sizeof(read_buff));
    position = Ql_FS_GetFilePosition(handle);//get postion
    ret = Ql_FS_Seek(handle,LENGTH-1,position);//seek     
    ret = Ql_FS_Read(handle, read_buff, LENGTH-1, &readenLen);
    APP_DEBUG("%s",read_buff);
}
APP_DEBUG("\r\n<--------------------------->\r\n");
Ql_FS_Close(handle);//close the file
handle=-1;
         Ql_strcpy(firmwareInfomation.name,"aaaa");
	  Ql_strcpy(firmwareInfomation.version,"aaaa");
	  APP_DEBUG("%s,%s\r\n", firmwareInfomation.name,firmwareInfomation.version);
  if(readenLen)
  {
	Ql_sscanf(read_buff, "%*[^:]:%s[^\r\n]", firmwareInfomation.name, sizeof(firmwareInfomation.name));
	Ql_sscanf(read_buff, "%*[^:]:%*[^:]:%s[^\r\n]", firmwareInfomation.version, sizeof(firmwareInfomation.version));
  }
	APP_DEBUG("%s,%s\r\n", firmwareInfomation.name,firmwareInfomation.version);
}


void CreateDataFile(void )
{
     s32 handle = -1,ret;
	 
     #ifdef __TEST_FOR_RAM_FILE__
               handle = Ql_FS_OpenRAMFile(datafile,QL_FS_CREATE|QL_FS_READ_WRITE, LENGTH);
     #else
               handle = Ql_FS_Open(datafile,QL_FS_READ_WRITE );
     #endif
     if(handle<0)
     {
           #ifdef __TEST_FOR_RAM_FILE__
                   handle = Ql_FS_OpenRAMFile(datafile,QL_FS_CREATE_ALWAYS, LENGTH);
          #else
                   handle = Ql_FS_Open(datafile,QL_FS_CREATE_ALWAYS); //if file does not exist ,creat it
          #endif
          if(handle>0)
          {
                  APP_DEBUG("\r\n<-- Create file (%s) OK! handle =%d -->\r\n",datafile,handle);
          }
          else
          {
                  APP_DEBUG("\r\n<-- failed!! Create file (%s) fail-->",datafile);
          }
     }
     else
     {
             APP_DEBUG("\r\n<--The file has exist-->\r\n");
     }
     Ql_FS_Close(handle);
     handle=-1;
}

bool  sFILE_MsgSave(u8* buf, u16 len)
{
    

}



bool sFILE_MsgLoad()
{
}