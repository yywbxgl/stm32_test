#include "sys.h"
#include "delay.h"
#include "timer.h"
#include "usart.h"
#include "key.h"
#include "beep.h"
#include "usmart.h"
#include "rtc.h"
#include "usart3.h"
#include "sim800c.h"
#include "fm1702.h" 
#include "test.h"
#include "dcf.h"
#include "digitron.h"
#include "mqtt_app.h"
#include "malloc.h"
#include "logging.h"
#include "utils.h"
#include "commands.h"
#include <string.h>
#include <jansson.h>


u8 mqtt_msg[300]={0}; //mqtt消息包
u8 send_cmd[20]= {0};

int main(void)
{
    u8 t = 0;
    u8 error_count = 5;
    u8 ipbuf[16]= HOST_IP;//IP缓存
    const u8 *port= HOST_PORT;  //端口固定为8086,当你的电脑8086端口被其他程序占用的时候,请修改为其他空闲端口
    u8 mode= 0;              //0,TCP连接;1,UDP连接
    u16 len;

    delay_init();           //延时函数初始化   
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    uart_init(115200);      //调试打印串口，初始化为115200
    usart3_init(115200);    //初始化串口3,与SIM800C通信
    usmart_dev.init(SystemCoreClock/1000000);   //串口调试组件USMART初始化
    RTC_Init();             //RTC初始化
    DCF_Init();             //电磁阀初始化
    DPinit();               //数码管初始化
    
    LOGI("hardware init finish.");

    //DCF_Set();

    //fm1702_test();

    //usart_test();
 
    //rtc_test();

    //sim800c_test();

    //fm1702_test();

    //与服务器建立握手
    while(connect_to_server(mode, ipbuf, (u8*)port))
    {
        delay_ms(1000);     
    }

    //发起mqtt_connect请求
    len=mqtt_connect_message(mqtt_msg, CLIENTID , USRNAME, PASSWD);//id,用户名和密码
    //LOGD("send len = %d\r\n", len);
    LOGI("mqtt connect... \r\n");
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                      //必须加延时
        //sim800c_send_cmd((u8*)0X1A,0,0);   //CTRL+Z,结束数据发送,启动一次传输
        delay_ms(1000);                      //必须加延时
    }


    len=mqtt_subscribe_message(mqtt_msg,TOPIC,1,1);//订阅test主题
    //printf("send len = %d\r\n", len);
    LOGI("mqtt_subscribe... \r\n");
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                 //必须加延时
        //sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,结束数据发送,启动一次传输
        delay_ms(1000);               //必须加延时
    }


    //开启定时器，每间隔多少发一个ping包
    //TIM3_Int_Init(5000,56000);

    while(1)
    {
        if(t%25 == 0)
        {
            len=mqtt_publish_message(mqtt_msg, TOPIC, "device_sun_smile", 0);
            //LOGI("send len = %d\r\n", len);
            sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
            if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
            {
                LOGI("mqtt_publish...");
                //PrintHex(mqtt_msg,len);
                //u3_printf_hex(mqtt_msg, len);
                if (sim800c_send_hex(mqtt_msg, len, "SEND OK", 200)==1)
                {
                    error_count--;
                    //重新连接服务器
                }
                USART3_RX_STA=0;
                // delay_ms(1000);                      //必须加延时
                // sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,结束数据发送,启动一次传输
            }
        }

        if(USART3_RX_STA&0X8000)        //接收到一次数据了
        {
           USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;  //添加结束符               
           if(strstr((const char*)USART3_RX_BUF,"\r\n"))//接收到TCP/UDP数据
           {
                LOGI("recv data:%s", (const char*)USART3_RX_BUF);//接收到的字节数
           }
           USART3_RX_STA=0;
        }
        
        delay_ms(200);
        t++;
    }

#if 0
    while(1)
    {
        if(USART3_RX_STA&0X8000)        //接收到一次数据了
        {
           USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;  //添加结束符               
           if( strstr((const char*)USART3_RX_BUF,"\r\n"))//接收到TCP/UDP数据
           {
                LOGI("recv data:%s", (const char*)USART3_RX_BUF);//接收到的字节数
           }
           USART3_RX_STA=0;
        }
        delay_ms(400);

    }

#endif



}






