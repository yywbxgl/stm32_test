#include "utils.h"
#include "logging.h"

//���������ֶ�ֵ��ʼ��
char g_device_code[24] = { 0};

//ָ��1  �ֶ�ֵ��ʼ��
u8 g_disable = 0;
u8 g_abnormal = 0;
u8 g_heart = 10;
u8 g_offLine = 1;
char g_pwd[24] = "test111111111";
u8 g_chargRate = 1;
u8 g_moneyRate = 7;
u8 g_logRate = 1;


//�豸��ǰҵ��״̬��ʼ��
char g_card_id[24] = "no card";
u16 g_ICCard_Value = 0;
u8 g_state = INIT;



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
