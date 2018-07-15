#ifndef __BEEP_H
#define __BEEP_H	 
#include "sys.h"

//蜂鸣器端口定义
#define BEEP PBout(8)	// BEEP,蜂鸣器接口		   

void BEEP_Init(void);	//初始化

//打开蜂鸣器，使用前先BEEP_Init初始化
void BEEP_Set(void);	

//关闭蜂鸣器，使用前先BEEP_Init初始化
void BEEP_Reset(void);


#endif

