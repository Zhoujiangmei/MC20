/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: tok.h 4 2007-08-27 13:11:03Z xtimor $
 *
 */

#ifndef __NMEA_TOK_H__
#define __NMEA_TOK_H__

#include "config.h"

#ifdef  __cplusplus
extern "C" {
#endif

//typedef char * va_list;   // TC中定义为void*
//#define _INTSIZEOF(n)     ((sizeof(n)+sizeof(int)-1)&~(sizeof(int) - 1) ) //为了满足需要内存对齐的系统
//#define va_start(ap,v)      ( ap = (va_list)&v + _INTSIZEOF(v) ) //ap指向第一个变参的位置，即将第一个变参的地址赋予ap
//#define va_arg(ap,t)        ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) ) /*获取变参的具体内容，t为变参的类型，如有多个参数，则通过移动ap的指针来获得变参的地址，从而获得内容*/
//#define va_end(ap)          ( ap = (va_list)0 )//清空va_list，即结束变参的获取




int     nmea_calc_crc(const char *buff, int buff_sz);
int     nmea_atoi(const char *str, int str_sz, int radix);
double  nmea_atof(const char *str, int str_sz);
int     nmea_printf(char *buff, int buff_sz, const char *format, ...);
int     nmea_scanf(const char *buff, int buff_sz, const char *format, ...);


#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_TOK_H__ */
