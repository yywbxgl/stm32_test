#include "timer.h"
#include "led.h"
#include "usart.h"
#include "logging.h"
#include "sim800c.h"
#include "mqtt_app.h"
#include "utils.h"
#include "usart3.h"
#include "delay.h"


extern vu16 USART3_RX_STA;
//extern u8* mqtt_msg;


//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

    //定时器TIM3初始化
    TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
    TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位

    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

    //中断优先级NVIC设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级3级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


    TIM_Cmd(TIM3, ENABLE);  //使能TIMx
}

//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
    {
        u16 len;
        u8 mqtt_msg[200]={0}; //mqtt消息包
        u8 send_cmd[20]= {0};
        LOGI("mqtt publish... \r\n");
        len=mqtt_publish_message(mqtt_msg, TOPIC, "device_sun_smile", 0);
        //LOGI("send len = %d\r\n", len);
        sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
        if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
        {
            PrintHex(mqtt_msg,len);
            u3_printf_hex(mqtt_msg, len);
            //delay_ms(1000);                      //必须加延时
            //sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,结束数据发送,启动一次传输
            //delay_ms(1000);
            //delay_ms(1000);
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志
        //delay_ms(1000);
    }
}




//配置TIM7预装载周期值
void TIM7_SetARR(u16 period)
{
    TIM_SetCounter(TIM7,0);//计数器清空
    TIM7->ARR&=0x00;//先清预装载周期值为0
    TIM7->ARR|= period;//更新预装载周期值
}

//定时器7中断服务程序
//用于串口3数据接收超时，超时之后认为一次数据接收完全
void TIM7_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//是更新中断
    {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update  );  //清除TIM7更新中断标志    
        USART3_RX_STA|=1<<15;	//标记接收完成
        TIM_Cmd(TIM7, DISABLE);  //关闭TIM7 
    }
}
 
//通用定时器7中断初始化
//这里时钟选择为APB1的2倍，而APB1为42M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz 
//通用定时器中断初始化
//这里始终选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数		 
void TIM7_Int_Init(u16 arr,u16 psc)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7时钟使能    

    //定时器TIM7初始化
    TIM_TimeBaseStructure.TIM_Period = arr;                     //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
    TIM_TimeBaseStructure.TIM_Prescaler =psc;                   //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);             //根据指定的参数初始化TIMx的时间基数单位

    TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE );                   //使能指定的TIM7中断,允许更新中断

    TIM_Cmd(TIM7,ENABLE);//开启定时器7

    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
}

