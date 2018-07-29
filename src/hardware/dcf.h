#ifndef __DCF_H
#define __DCF_H	 
#include "sys.h"

//蜂鸣器端口定义
#define DCF PBout(0)	// 电磁阀接口		   

void DCF_Init(void);	//初始化

//打开电磁阀，使用前先DCF_Init初始化
void DCF_Set(void);	

//关闭电磁阀，使用前先DCF_Init初始化
void DCF_Reset(void);


#endif
