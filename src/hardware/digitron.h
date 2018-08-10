#ifndef __DIGITRON_H
#define __DIGITRON_H

//数码管定义

#include "sys.h"


extern u8 g_Digitron[8];


void DPinit(void);	//初始化

void display(u8 disnum[8]);

//关闭数码管显示
void SdgOffAll(void);


#endif
