#include "test.h"



//LED设置
void led0_set(u8 sta)
{
	LED0=sta;
} 


//LED设置
void led1_set(u8 sta)
{
	LED1=sta;
} 


//LED测试接口
void led_test(void)
{
	delay_init();
	LED_Init();
	LED0 = 0;  //打开LED
	LED1 = 0;
	delay_ms(500);
	LED0 = 1; //关闭LED
	LED1 = 1;
}


//蜂鸣器测试接口
void beep_test(void)
{
	delay_init();
	BEEP_Init();
	
	BEEP_Set();  //打开蜂鸣器
	delay_ms(500);
	BEEP_Reset(); //关闭蜂鸣器
}


