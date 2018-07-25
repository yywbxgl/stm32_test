#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "led.h"
#include "beep.h"
#include "usmart.h"
#include "rtc.h"
#include "usart3.h"
#include "sim800c.h"
#include "fm1702.h"	
#include "test.h"


int main(void)
{
    delay_init();           //��ʱ������ʼ��	  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(115200);      //���ڳ�ʼ��Ϊ115200
    usart3_init(115200);    //��ʼ������3,��SIM800Cͨ��
    LED_Init();             //LED�˿ڳ�ʼ��
    KEY_Init();             //��ʼ���밴�����ӵ�Ӳ���ӿ�
    usmart_dev.init(SystemCoreClock/1000000);   //���ڵ������USMART��ʼ��
    RTC_Init();             //RTC��ʼ��
    printf("Now Time:%d-%d-%d %d:%d:%d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//�������ʱ�� 

    //usart_test();

    //rtc_test();

    sim800c_test();

    //fm1702_test();

}




