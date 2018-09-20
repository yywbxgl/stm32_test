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

    main_loop();

}






