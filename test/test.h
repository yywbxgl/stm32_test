#ifndef __TEST_H
#define __TEST_H	 


#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "led.h"
#include "beep.h"
#include "usmart.h"



//LED测试接口，设置 LED状态
void led0_set(u8 sta);


//LED测试接口，设置 LED状态
void led1_set(u8 sta);


//LED测试接口
void led_test(void);


//蜂鸣器测试接口
void beep_test(void);


#endif
