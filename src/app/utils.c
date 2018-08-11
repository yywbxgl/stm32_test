#include "utils.h"
#include "logging.h"

//公共部分字段值初始化
char g_device_code[24] = { 0};

//指令1  字段值初始化
u8 g_disable = 0;
u8 g_abnormal = 0;
u8 g_heart = 5;
u8 g_offLine = 1;
char g_pwd[24] = "11";
u8 g_chargRate = 1;
u8 g_moneyRate = 7;
u8 g_logRate = 1;


//设备当前业务状态初始化
char g_card_id[24] = "no card";
u16 g_ICCard_Value = 0;
u8 g_state = INIT;

char g_orderNo[24] = {0};
char g_serverCardNo[9] = {0};

u16 g_consume_time = 0;

void PrintHex(u8 *buf,u16 len)
{
    u16 i;
    LOGV("\r\n");
    for(i=0;i<len;i++)
    {
        LOGV("%02x ",buf[i]);
    }
    LOGV("\r\n\r\n");
}
