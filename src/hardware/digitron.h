#ifndef __DIGITRON_H
#define __DIGITRON_H

//数码管定义

#include "sys.h"


extern u8 g_Digitron[8];


void DPinit(void);	//初始化

//打开数码管显示
void setOnFlag(void);

//关闭数码管显示
void setOffFlag(void);

//数码管显示金额
void display(u32 money);


#endif
