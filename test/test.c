#include "test.h"



//LED����
void led0_set(u8 sta)
{
	LED0=sta;
} 


//LED����
void led1_set(u8 sta)
{
	LED1=sta;
} 


//LED���Խӿ�
void led_test(void)
{
	delay_init();
	LED_Init();
	LED0 = 0;  //��LED
	LED1 = 0;
	delay_ms(500);
	LED0 = 1; //�ر�LED
	LED1 = 1;
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


