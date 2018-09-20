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
#include <string.h>
#include <stdlib.h>
#include "logging.h"
#include "utils.h"
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

    main_loop();

}






