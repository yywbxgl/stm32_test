#include "test.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "led.h"
#include "beep.h"
#include "usmart.h"
#include "fm1702.h"
#include "rtc.h"

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

    LED0 = 0;
    Init_FM1702();
    LED0 = 1;

    try = 5;
    while(--try)
    {
        
        status = Request(RF_CMD_REQUEST_ALL);       //Ѱ��
        printf("fm1702_test request find card. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status = AntiColl();                        //��ͻ���
        printf("fm1702_test AntiColl. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status=Select_Card();                       //ѡ��
        printf("fm1702_test Select_Card. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status = Load_keyE2_CPY(DefaultKey);          //��������
        printf("fm1702_test Load_keyE2_CPY. ret=%d\r\n", status);
        if(status != TRUE)         continue;

        status = Authentication(UID, 1, RF_CMD_AUTH_LA);//��֤1����keyA
        printf("fm1702_test Authentication. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status=MIF_READ(buf,4);             //��������ȡ1����0�����ݵ�buffer[0]-buffer[15]
        printf("fm1702_test MIF_READ. ret=%d\r\n", status);
        //status=MIF_Write(buffer,4);       //д������buffer[0]-buffer[15]д��1����0��
        if(status == FM1702_OK)
        {
            //��д�ɹ�������LED
            printf("fm1702_test, read and write success.\r\n");
            delay_ms(10000);
        }
        
        delay_ms(1000);

    }
}


