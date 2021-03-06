#include "ql_stdlib.h"
#include "ql_type.h"
#include "ql_uart.h"


#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT1
#define DBG_BUF_LEN   1024
static char DBG_BUFFER[DBG_BUF_LEN];

#define SERIAL_RX_BUFFER_LEN  2048
#define APP_DEBUG(FORMAT,...) {\
    Ql_memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    Ql_sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace(DBG_BUFFER);\      
      \
    } else {\
        Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), Ql_strlen((const char *)(DBG_BUFFER)));\
        Ql_Sleep(50);\
    }\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif


/*
#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT1
#define DBG_BUF_LEN   1024
static char DBG_BUFFER[DBG_BUF_LEN];

#define SERIAL_RX_BUFFER_LEN  2048
#define APP_DEBUG(FORMAT,...) {\
    Ql_memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    Ql_sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace(DBG_BUFFER);\      
      \
    } else {\
        Ql_UART_Write((Enum_SerialPort)(UART_PORT2), (u8*)(DBG_BUFFER), Ql_strlen((const char *)(DBG_BUFFER)));\
        Ql_Sleep(50);\
    }\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif
*/

