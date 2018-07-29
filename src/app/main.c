#include "sys.h"
#include "delay.h"
#include "time.h"
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

int main(void)
{

    delay_init();           //延时函数初始化   
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    uart_init(115200);      //串口初始化为115200
    usart3_init(115200);    //初始化串口3,与SIM800C通信
    usmart_dev.init(SystemCoreClock/1000000);   //串口调试组件USMART初始化
    RTC_Init();             //RTC初始化
    DCF_Init();             //电磁阀初始化
    DPinit();               //数码管初始化
    printf("Now Time:%d-%d-%d %d:%d:%d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//输出闹铃时间 

    //DCF_Set();

    //fm1702_test();

    //usart_test();
 
    //rtc_test();

    sim800c_test();

    //fm1702_test();

}




