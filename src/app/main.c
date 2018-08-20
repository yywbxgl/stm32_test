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


    u16 time_t=0;
    while(1)
    {
        if(g_state == INIT)
        {
            //�������������ʾ��ǰ�豸״̬
            memset(g_Digitron, INIT, sizeof(g_Digitron));
            setOnFlag();
            
            if (connect_to_server() == TRUE){
                g_state = TCP_OK;
            }
        }
        else if(g_state == TCP_OK)
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

            //��ѯ������Ϣ
            if (recv_mqtt_message() == TRUE){
                u8 trade = parse_mqtt_message();
                if (trade == 1){
                    //����������
                    ;
                }else if (trade == 6){
                    //����ɨ����������
                    ;
                }else if (trade == 0){
                    //���������ϱ���������
                    ;
                }
            }

            //�ȴ�IC��
            if (time_t % 20 == 0){
                if (scan_for_card() == TRUE){
                    g_state = ON_IC;
                }
            }

            //ÿ��g_heart����һ������
            if (time_t % (g_heart*100) == 0)
            {
                if (send_keep_alive_mesaage() == FALSE){
                    g_state = TCP_OK;
                }
            }

        }
        else if(g_state == ON_IC){
            
            //�������������ʾ��ǰ�豸״̬
            //memset(g_Digitron, ON_IC, sizeof(g_Digitron));
            setOnFlag();

            //���Ϳ�ʼ�������������ˮ
            if(send_start_consume_mesaage() == TRUE){
                g_state = IC_CONSUME;
                g_consume_time = 0;  //��ʼ�Ʒ�
                DCF_Set();           //�򿪵�ŷ�
            }else{
                g_state = WAIT_IC;
            }

        }
        else if(g_state == IC_CONSUME){
            
            setOnFlag();  //������ܣ���ʾ�������
            if (card_runing() == FALSE)
            {
               //����ǰ����һ����������
               send_finish_consume_mesaage();
               g_state = WAIT_IC;
               DCF_Reset();         //�رյ�ŷ�
               g_consume_time = 0;  //�����Ʒ�
            }

            //ÿ��g_logRate����һ������Ϣ
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






