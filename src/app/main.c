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
#include "app.h"




int main(void)
{

    delay_init();           //��ʱ������ʼ��
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(115200);      //���Դ�ӡ���ڣ���ʼ��Ϊ115200
    usart3_init(115200);    //��ʼ������3,��SIM800Cͨ��
    usmart_dev.init(SystemCoreClock/1000000);   //���ڵ������USMART��ʼ��
    RTC_Init();             //RTC��ʼ��
    Init_FM1702();          //fm1702��ʼ��
    DCF_Init();             //��ŷ���ʼ��
    DPinit();               //����ܳ�ʼ��
    LOGI("hardware init finish.");
    
    //usart_test();
 
    //rtc_test();

    //sim800c_test();

    //fm1702_test();

    u8 time_t=0;
    while(1)
    {
        if (g_state == INIT)
        {
            //�������������ʾ��ǰ�豸״̬
            memset(g_Digitron, INIT, sizeof(g_Digitron));
            setOnFlag();
            
            if (connect_to_server() == TRUE){
                g_state = TCP_OK;
            }
        }
        else if (g_state == TCP_OK)
        {
            //�������������ʾ��ǰ�豸״̬
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
            //�������������ʾ��ǰ�豸״̬
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
            //�ر���������ܣ��ȴ�IC����
            setOffFlag();

            if (scan_for_card() == TRUE){
                g_state = ON_IC;
            }

            USART3_RX_STA=0;
            //ÿ��g_heart����һ������
            if (time_t % g_heart == 0)
            {
                if (send_keep_alive_mesaage() == FALSE){
                    g_state = TCP_OK;
                }
            }

            if(USART3_RX_STA&0X8000)        //���յ�һ��������
            {
                USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;  //��ӽ�����
                LOGI("recv data[%d]", USART3_RX_STA&0X7FFF);//���յ����ֽ���
                PrintHex(USART3_RX_BUF, USART3_RX_STA&0X7FFF);
                USART3_RX_STA=0;
            }
        }
        else if (g_state == ON_IC){
            //������ܣ���ʾ�������
            setOnFlag();
            if (card_runing() == FALSE)
            {
                //����ǰ����һ����������
                LOGI("���ͽ�������");
                g_state = WAIT_IC;
            }

            //ÿ��g_logRate����һ������Ϣ
            if (time_t % g_logRate == 0)
            {
                LOGI("���Ϳ۷�����");
            }
        }

        delay_ms(500);
        time_t++;
    }



}






