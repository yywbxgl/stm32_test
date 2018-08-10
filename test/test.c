#include "test.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "led.h"
#include "beep.h"
#include "usmart.h"
#include "fm1702.h"
#include "rtc.h"
#include "logging.h"
#include "utils.h"
#include "message.h"


//extern char* g_card_id;

extern u16 g_ICCard_Value;
extern u8 ic_on_flag = 0;



/*
//LED测试接口
void led_test(void)
{
    LED_Init();
    delay_init();
    LED0 = !LED0;
    LED1 = !LED1;
    delay_ms(500);
    LED0 = !LED0;
    LED1 = !LED1;
}
*/

//蜂鸣器测试接口
void beep_test(void)
{
    delay_init();
    BEEP_Init();

    BEEP_Set();  //打开蜂鸣器
    delay_ms(500);
    BEEP_Reset(); //关闭蜂鸣器
}


//RTC测试代码
void rtc_test(void)
{
    delay_init();
    RTC_Init();             //RTC初始化
    printf("Now Time:%d-%d-%d %d:%d:%d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//输出闹铃时间 
}


void usart_test(void)
{
    // 串口测试代码
    u16 t;
    u16 len;
    u16 times=0;
    while(1)
    {
        if(USART_RX_STA&0x8000)
        {                      
            len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
            printf("\r\n您发送的消息为:\r\n\r\n");
            for(t=0;t<len;t++)
            {
              USART_SendData(USART1, USART_RX_BUF[t]);//向串口1发送数据
              while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
            }
            printf("\r\n\r\n");//插入换行
            USART_RX_STA=0;
        }
        else
        {
            times++;
            if(times%5000==0)
            {
                printf("\r\n战舰STM32开发板 串口实验\r\n");
                printf("正点原子@ALIENTEK\r\n\r\n");
            }
            if(times%200==0)
                printf("请输入数据,以回车键结束\r\n");
            if(times%30==0)
                LED0=!LED0;//闪烁LED,提示系统正在运行.
            delay_ms(10);
        }
    }
}


//IC卡读写测试
void fm1702_test(void)
{
    unsigned char status, try;
    unsigned char buf[16], DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
    Init_FM1702();
    LOGD("Init_FM1702 finish");
    try = 5;
    while(try--)
    {
        delay_ms(500);
        
        status = Request(RF_CMD_REQUEST_ALL);       //寻卡
        if(status != FM1702_OK)
        {
            LOGD("未检测到卡片");
            continue;
        }

        status = AntiColl();                        //冲突检测
        if(status != FM1702_OK)
        {
            LOGE("卡片冲突");
            continue;
        }

        status=Select_Card();                       //选卡
        if(status != FM1702_OK)
        {
            LOGE("选择卡片失败");
            continue;
        }

        status = Load_keyE2_CPY(DefaultKey);          //加载密码
        if(status != TRUE)
        {
            LOGE("加载密码失败");
            continue;
        }

        status = Authentication(UID, 7, RF_CMD_AUTH_LA);//验证1扇区keyA
        if(status != FM1702_OK)
        {
            LOGE("验证扇区失败");
            continue;
        }

        char card_id[12] ={0};
        sprintf(card_id, "%02x%02x%02x%02x", UID[0], UID[1], UID[2], UID[3]);
        LOGI("卡片唯一号：%s", card_id);

        status=MIF_READ(buf,28);             //读卡，读取7扇区0块数据到buffer[0]-buffer[15]
        if(status != FM1702_OK)
        {
            LOGE("读卡数据失败");
            continue;
        }
        else
        {
            ic_on_flag = 1;
            g_ICCard_Value = (buf[0]<<8)|buf[1];
            LOGI("读卡数据成功，余额= %d\r\n", g_ICCard_Value);
        }

        buf[1] -= 1;
        status=MIF_Write(buf,28);       //写卡，将buffer[0]-buffer[15]写入1扇区0块
        if(status == FM1702_OK)
        {
            //读写成功，点亮LED
            LOGI("写数据成功\r\n");
        }

    }
}


