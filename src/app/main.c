#include "sys.h"
#include "delay.h"
#include "timer.h"
#include "usart.h"
#include "usmart.h"
#include "rtc.h"
#include "usart3.h"
#include "fm1702.h" 
#include "test.h"
#include "dcf.h"
#include "digitron.h"
#include "mqtt_app.h"
#include "malloc.h"
#include "logging.h"
#include "utils.h"
#include <string.h>
#include <jansson.h>
#include <stdlib.h>
#include "app.h"
#include "sim800c.h"
#include "MQTTPacket.h"
#include "MQTTPublish.h"




int main(void)
{

    delay_init();           //延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    uart_init(115200);      //调试打印串口，初始化为115200
    usart3_init(115200);    //初始化串口3,与SIM800C通信
    usmart_dev.init(SystemCoreClock/1000000);   //串口调试组件USMART初始化
    RTC_Init();             //RTC初始化
    Init_FM1702();          //fm1702初始化
    DCF_Init();             //电磁阀初始化
    DPinit();               //数码管初始化
    LOGI("hardware init finish.");

    //usart_test();

    //rtc_test();

    //sim800c_test();

    //fm1702_test();


    u16 time_t=0;
    while(1)
    {
        if(g_state == INIT)
        {
            //打开数码数码管显示当前设备状态
            memset(g_Digitron, INIT, sizeof(g_Digitron));
            setOnFlag();
            
            if (connect_to_server() == TRUE){
                g_state = TCP_OK;
            }
        }
        else if(g_state == TCP_OK)
        {
            //打开数码数码管显示当前设备状态
            memset(g_Digitron, TCP_OK, sizeof(g_Digitron));
            setOnFlag();
            
            if (subscribe_mqtt() == TRUE){
                g_state = MQTT_OK;
            }else if ( subscribe_mqtt() == FALSE){
                g_state = INIT; 
            }
        }
        else if(g_state == MQTT_OK)
        {
            //打开数码数码管显示当前设备状态
            memset(g_Digitron, MQTT_OK, sizeof(g_Digitron));
            setOnFlag();

            USART3_RX_STA=0;
            if (send_keep_alive_mesaage() == TRUE){
                g_state = WAIT_IC;
            }
            else{
                g_state = TCP_OK;
            }
        }
        else if(g_state == WAIT_IC){

            //关闭数码数码管，等待IC卡、
            setOffFlag();

            //查询接收消息
            if (recv_mqtt_message() == TRUE){
                u8 trade = parse_mqtt_message();
                if (trade == 1){
                    //处理保活信令
                    ;
                }else if (trade == 6){
                    //处理扫描消费信令
                    ;
                }else if (trade == 0){
                    //处理主动上报保活信令
                    ;
                }
            }

            //等待IC卡
            if (time_t % 20 == 0){
                if (scan_for_card() == TRUE){
                    g_state = ON_IC;
                }
            }

            //每隔g_heart发送一个心跳
            if (time_t % (g_heart*100) == 0)
            {
                if (send_keep_alive_mesaage() == FALSE){
                    g_state = TCP_OK;
                }
            }

        }
        else if(g_state == ON_IC){
            
            //打开数码数码管显示当前设备状态
            //memset(g_Digitron, ON_IC, sizeof(g_Digitron));
            setOnFlag();

            //发送开始消费请求，允许放水
            if(send_start_consume_mesaage() == TRUE){
                g_state = IC_CONSUME;
                g_consume_time = 0;  //开始计费
                DCF_Set();           //打开电磁阀
            }else{
                g_state = WAIT_IC;
            }

        }
        else if(g_state == IC_CONSUME){
            
            setOnFlag();  //打开数码管，显示卡内余额
            if (card_runing() == FALSE)
            {
               //结束前发送一个结束信令
               send_finish_consume_mesaage();
               g_state = WAIT_IC;
               DCF_Reset();         //关闭电磁阀
               g_consume_time = 0;  //结束计费
            }

            //每隔g_logRate发送一消费信息
            if(time_t % (g_logRate*100) == 0)
            {
               send_consume_mesaage();
            }

            recv_mqtt_message();
        }

        delay_ms(10);
        time_t++;
    }


}






