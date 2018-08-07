/************************************日志等级控制************************************
*
*   定义显示的日志级别

*****************************************************************************************/
#pragma once
#ifndef __LOG_PRINT_H_
#define __LOG_PRINT_H_
#include <stdio.h>
#include <stdlib.h>
//#include <time.h>
#include "rtc.h"
#ifdef __cplusplus
extern "C"
{
#endif
/*控制日志输出到控制台或者文件，注释此行日志输出到控制台*/
//#define DEBUG_FILE    "log.dat"

/*改变此行控制日志输出级别*/
#define LOG_LEVEL   LOG_LEVEL_VERBOSE

#define LOG_LEVEL_VERBOSE   1   /*所有*/
#define LOG_LEVEL_DEBUG     2   /*调试*/
#define LOG_LEVEL_INFO      3   /*信息*/
#define LOG_LEVEL_WARNING   4   /*警告*/
#define LOG_LEVEL_ERROR     5   /*错误*/
#define LOG_LEVEL_FATAL     6   /*致命错误*/
#define LOG_LEVEL_SILENT    7   /*静默*/

#define TITLE_VERBOSE       "V: "
#define TITLE_DEBUG         "Debug: "
#define TITLE_INFO          "Info: "
#define TITLE_WARNING       "Warn: "
#define TITLE_ERROR         "Error: "
#define TITLE_FATAL         "Fatel: "

#ifndef LOG_LEVEL
#   define LOG_LEVEL    LOG_LEVEL_VERBOSE
#endif

#define LOG_NOOP    (void) 0

#ifndef _FILE_POINTER_
#define _FILE_POINTER_
    static  FILE *fp_log;   
#endif

#ifndef _TIME_POINTER_
#define _TIME_POINTER_
     //static time_t timep;
     //static struct tm *p;
#endif

#ifdef DEBUG_FILE
#   define  LOG_PRINT(level,fmt,...)    \
        fp_log=fopen(DEBUG_FILE,"a");   \
        fprintf(fp_log,\
        "[%d-%d %d:%d:%d] [%s:%u] " level fmt "\r\n",\
        calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec,\
        __FILE__,__LINE__,##__VA_ARGS__);   \
        fclose(fp_log); \
        fp_log=NULL
#else
#   define  LOG_PRINT(level,fmt,...)    \
        fprintf(stdout,\
        "[%d:%d:%d][%s:%u] " level fmt "\r\n",\
        calendar.hour,calendar.min,calendar.sec,\
        __FILE__,__LINE__,##__VA_ARGS__)
#endif


#if LOG_LEVEL_VERBOSE >= LOG_LEVEL
#   define LOG_VERBOSE(fmt,...) printf(fmt,##__VA_ARGS__)
#else
#   define LOG_VERBOSE(...) LOG_NOOP
#endif

#define LOGV(format,...)    LOG_VERBOSE(format,##__VA_ARGS__)

#if LOG_LEVEL_DEBUG >= LOG_LEVEL
#   define  LOG_DEBUG(fmt,...)  LOG_PRINT(TITLE_DEBUG,fmt,##__VA_ARGS__)
#else
#   define  LOG_DEBUG(...)  LOG_NOOP
#endif

#define LOGD(format,...)    LOG_DEBUG(format,##__VA_ARGS__)

#if LOG_LEVEL_INFO>= LOG_LEVEL
#   define  LOG_INFO(fmt,...)   LOG_PRINT(TITLE_INFO,fmt,##__VA_ARGS__)
#else
#   define  LOG_INFO(...)   LOG_NOOP
#endif

#define LOGI(format,...)    LOG_INFO(format,##__VA_ARGS__)

#if LOG_LEVEL_WARNING >= LOG_LEVEL
#   define  LOG_WARN(fmt,...)   LOG_PRINT(TITLE_WARNING,fmt,##__VA_ARGS__)
#else
#   define  LOG_WARN(...)   LOG_NOOP
#endif

#define LOGW(format,...)    LOG_WARN(format,##__VA_ARGS__)

#if LOG_LEVEL_ERROR >= LOG_LEVEL
#   define  LOG_ERROR(fmt,...)  LOG_PRINT(TITLE_ERROR,fmt,##__VA_ARGS__)
#else
#   define  LOG_ERROR(...)  LOG_NOOP
#endif

#define LOGE(format,...)    LOG_ERROR(format,##__VA_ARGS__)

#if LOG_LEVEL_FATAL >= LOG_LEVEL
#   define  LOG_FATAL(fmt,...)  LOG_PRINT(TITLE_FATAL,fmt,##__VA_ARGS__)
#else
#   define  LOG_FATAL(...)  LOG_NOOP
#endif

#define LOGF(format,...)    LOG_FATAL(format,##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif

