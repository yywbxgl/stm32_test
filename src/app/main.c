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


u8 mqtt_msg[300]={0}; //mqtt��Ϣ��
u8 send_cmd[20]= {0};

int main(void)
{
    u8 t = 0;
    u8 error_count = 5;
    u8 ipbuf[16]= HOST_IP;//IP����
    const u8 *port= HOST_PORT;  //�˿ڹ̶�Ϊ8086,����ĵ���8086�˿ڱ���������ռ�õ�ʱ��,���޸�Ϊ�������ж˿�
    u8 mode= 0;              //0,TCP����;1,UDP����
    u16 len;

    delay_init();           //��ʱ������ʼ��   
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(115200);      //���Դ�ӡ���ڣ���ʼ��Ϊ115200
    usart3_init(115200);    //��ʼ������3,��SIM800Cͨ��
    usmart_dev.init(SystemCoreClock/1000000);   //���ڵ������USMART��ʼ��
    RTC_Init();             //RTC��ʼ��
    DCF_Init();             //��ŷ���ʼ��
    DPinit();               //����ܳ�ʼ��
    
    LOGI("hardware init finish.");

    //DCF_Set();

    //fm1702_test();

    //usart_test();
 
    //rtc_test();

    //sim800c_test();

    //fm1702_test();

    //���������������
    while(connect_to_server(mode, ipbuf, (u8*)port))
    {
        delay_ms(1000);     
    }

    //����mqtt_connect����
    len=mqtt_connect_message(mqtt_msg, CLIENTID , USRNAME, PASSWD);//id,�û���������
    //LOGD("send len = %d\r\n", len);
    LOGI("mqtt connect... \r\n");
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//���յ����ֽ���
    if(sim800c_send_cmd(send_cmd,">",200)==0)//��������
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                      //�������ʱ
        //sim800c_send_cmd((u8*)0X1A,0,0);   //CTRL+Z,�������ݷ���,����һ�δ���
        delay_ms(1000);                      //�������ʱ
    }


    len=mqtt_subscribe_message(mqtt_msg,TOPIC,1,1);//����test����
    //printf("send len = %d\r\n", len);
    LOGI("mqtt_subscribe... \r\n");
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//���յ����ֽ���
    if(sim800c_send_cmd(send_cmd,">",200)==0)//��������
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                 //�������ʱ
        //sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,�������ݷ���,����һ�δ���
        delay_ms(1000);               //�������ʱ
    }


    //������ʱ����ÿ������ٷ�һ��ping��
    //TIM3_Int_Init(5000,56000);

    while(1)
    {
        if(t%25 == 0)
        {
            len=mqtt_publish_message(mqtt_msg, TOPIC, "device_sun_smile", 0);
            //LOGI("send len = %d\r\n", len);
            sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//���յ����ֽ���
            if(sim800c_send_cmd(send_cmd,">",200)==0)//��������
            {
                LOGI("mqtt_publish...");
                //PrintHex(mqtt_msg,len);
                //u3_printf_hex(mqtt_msg, len);
                if (sim800c_send_hex(mqtt_msg, len, "SEND OK", 200)==1)
                {
                    error_count--;
                    //�������ӷ�����
                }
                USART3_RX_STA=0;
                // delay_ms(1000);                      //�������ʱ
                // sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,�������ݷ���,����һ�δ���
            }
        }

        if(USART3_RX_STA&0X8000)        //���յ�һ��������
        {
           USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;  //��ӽ�����               
           if(strstr((const char*)USART3_RX_BUF,"\r\n"))//���յ�TCP/UDP����
           {
                LOGI("recv data:%s", (const char*)USART3_RX_BUF);//���յ����ֽ���
           }
           USART3_RX_STA=0;
        }
        
        delay_ms(200);
        t++;
    }

#if 0
    while(1)
    {
        if(USART3_RX_STA&0X8000)        //���յ�һ��������
        {
           USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;  //��ӽ�����               
           if( strstr((const char*)USART3_RX_BUF,"\r\n"))//���յ�TCP/UDP����
           {
                LOGI("recv data:%s", (const char*)USART3_RX_BUF);//���յ����ֽ���
           }
           USART3_RX_STA=0;
        }
        delay_ms(400);

    }

#endif



}






