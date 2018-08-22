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
        if(g_state == INIT){
            memset(g_Digitron, INIT, sizeof(g_Digitron)); //�������������ʾ��ǰ�豸״̬
            setOnFlag();
            if (connect_to_server() == TRUE){
                g_state = TCP_OK;
            }
        }
        else if(g_state == TCP_OK){
            memset(g_Digitron, TCP_OK, sizeof(g_Digitron)); //�������������ʾ��ǰ�豸״̬
            setOnFlag();
            if (subscribe_mqtt() == TRUE){
                g_state = MQTT_OK;
            }else if ( subscribe_mqtt() == FALSE){
                g_state = INIT; 
            }
        }
        else if(g_state == MQTT_OK){
            memset(g_Digitron, MQTT_OK, sizeof(g_Digitron)); //�������������ʾ��ǰ�豸״̬
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
            setOffFlag(); //�ر���������ܣ��ȴ�IC����
            //��ѯ������Ϣ
            if (recv_mqtt_message() == TRUE){ 
                u8 trade = parse_mqtt_message();
                if (trade == 1){
                    //����������
                    ;
                }else if (trade == 6){
                    //����ɨ����������
                    if( deal_app_cousume_command() == TRUE);{
                        g_ICCard_Value = g_maxMoney;
                        display(g_ICCard_Value);
                        g_state = APP_CONSUME;
                        g_consume_time = 0;  //��ʼ�Ʒ�
                        DCF_Set();           //�򿪵�ŷ�
                    }
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
            memset(g_Digitron, ON_IC, sizeof(g_Digitron));
            setOnFlag(); //�������������ʾ��ǰ�豸״̬
            if(send_start_consume_mesaage() == TRUE){
                //���Ϳ�ʼ�������󣬵ȴ���Ӧ
                LOGI("��ʼˢ�����ѣ�");
                g_state = IC_CONSUME;
                g_consume_time = 0;  //��ʼ�Ʒ�
                DCF_Set();           //�򿪵�ŷ�
            }else{
                //�ȴ���Ƭ������λ״̬
                while(read_card() == TRUE)
                {
                    delay_ms(100);
                }
                g_state = WAIT_IC;
            }
        }
        else if(g_state == IC_CONSUME){
            setOnFlag();  //������ܣ���ʾ�������
            if (card_runing() == FALSE)
            {
               //����ǰ����һ����������
               send_consume_mesaage(1, 1);
               g_state = WAIT_IC;
               DCF_Reset();         //�رյ�ŷ�
               g_consume_time = 0;  //�����Ʒ�
            }

            if (g_ICCard_Value == 0)
            {
                LOGI("��ǰ����");
                //����ǰ����һ����������
                send_consume_mesaage(1, 1);
                g_state = WAIT_IC;
                DCF_Reset();         //�رյ�ŷ�
                g_consume_time = 0;  //�����Ʒ�
                //�ȴ���Ƭ������λ״̬
                display(g_ICCard_Value);
                while(read_card() == TRUE)
                {
                    delay_ms(100);
                }
                g_state = WAIT_IC;
            }

            //ÿ��g_logRate����һ������Ϣ
            if(time_t % (g_logRate*100) == 0)
            {
               send_consume_mesaage(1, 2);
            }

            recv_mqtt_message();
        }
        else if(g_state == APP_CONSUME){
            setOnFlag();  //������ܣ���ʾ�������
            //ÿ��g_logRate����һ������Ϣ
            if(time_t % (g_logRate*100) == 0)
            {
                send_consume_mesaage(2, 2);
            }

            if (g_ICCard_Value == 0)
            {
                LOGI("��ǰ���Ϊ0");
                send_consume_mesaage(2, 1);//�ر�ǰ���ͽ�����������
                g_state = WAIT_IC;
                DCF_Reset();         //�رյ�ŷ�
                g_consume_time = 0;  //�����Ʒ�
                display(g_ICCard_Value);
            }

            //������������ָ��8��������������
            if (recv_mqtt_message() == TRUE)
            {
                u8 trade = parse_mqtt_message();
                if(trade == 8)
                {
                    send_consume_mesaage(2, 1);
                    g_state = WAIT_IC;
                    DCF_Reset();         //�رյ�ŷ�
                    g_consume_time = 0;  //�����Ʒ�
                }
            }
            
        }
        

        delay_ms(10);
        time_t++;
    }


}






