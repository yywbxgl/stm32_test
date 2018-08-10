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
//LED���Խӿ�
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

//���������Խӿ�
void beep_test(void)
{
    delay_init();
    BEEP_Init();

    BEEP_Set();  //�򿪷�����
    delay_ms(500);
    BEEP_Reset(); //�رշ�����
}


//RTC���Դ���
void rtc_test(void)
{
    delay_init();
    RTC_Init();             //RTC��ʼ��
    printf("Now Time:%d-%d-%d %d:%d:%d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//�������ʱ�� 
}


void usart_test(void)
{
    // ���ڲ��Դ���
    u16 t;
    u16 len;
    u16 times=0;
    while(1)
    {
        if(USART_RX_STA&0x8000)
        {                      
            len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
            printf("\r\n�����͵���ϢΪ:\r\n\r\n");
            for(t=0;t<len;t++)
            {
              USART_SendData(USART1, USART_RX_BUF[t]);//�򴮿�1��������
              while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
            }
            printf("\r\n\r\n");//���뻻��
            USART_RX_STA=0;
        }
        else
        {
            times++;
            if(times%5000==0)
            {
                printf("\r\nս��STM32������ ����ʵ��\r\n");
                printf("����ԭ��@ALIENTEK\r\n\r\n");
            }
            if(times%200==0)
                printf("����������,�Իس�������\r\n");
            if(times%30==0)
                LED0=!LED0;//��˸LED,��ʾϵͳ��������.
            delay_ms(10);
        }
    }
}


//IC����д����
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
        
        status = Request(RF_CMD_REQUEST_ALL);       //Ѱ��
        if(status != FM1702_OK)
        {
            LOGD("δ��⵽��Ƭ");
            continue;
        }

        status = AntiColl();                        //��ͻ���
        if(status != FM1702_OK)
        {
            LOGE("��Ƭ��ͻ");
            continue;
        }

        status=Select_Card();                       //ѡ��
        if(status != FM1702_OK)
        {
            LOGE("ѡ��Ƭʧ��");
            continue;
        }

        status = Load_keyE2_CPY(DefaultKey);          //��������
        if(status != TRUE)
        {
            LOGE("��������ʧ��");
            continue;
        }

        status = Authentication(UID, 7, RF_CMD_AUTH_LA);//��֤1����keyA
        if(status != FM1702_OK)
        {
            LOGE("��֤����ʧ��");
            continue;
        }

        char card_id[12] ={0};
        sprintf(card_id, "%02x%02x%02x%02x", UID[0], UID[1], UID[2], UID[3]);
        LOGI("��ƬΨһ�ţ�%s", card_id);

        status=MIF_READ(buf,28);             //��������ȡ7����0�����ݵ�buffer[0]-buffer[15]
        if(status != FM1702_OK)
        {
            LOGE("��������ʧ��");
            continue;
        }
        else
        {
            ic_on_flag = 1;
            g_ICCard_Value = (buf[0]<<8)|buf[1];
            LOGI("�������ݳɹ������= %d\r\n", g_ICCard_Value);
        }

        buf[1] -= 1;
        status=MIF_Write(buf,28);       //д������buffer[0]-buffer[15]д��1����0��
        if(status == FM1702_OK)
        {
            //��д�ɹ�������LED
            LOGI("д���ݳɹ�\r\n");
        }

    }
}


