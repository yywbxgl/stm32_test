#ifndef __TEST_H
#define __TEST_H	 


#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "led.h"
#include "beep.h"
#include "usmart.h"



//LED���Խӿڣ����� LED״̬
void led0_set(u8 sta);


//LED���Խӿڣ����� LED״̬
void led1_set(u8 sta);


//LED���Խӿ�
void led_test(void);


//���������Խӿ�
void beep_test(void);


#endif
