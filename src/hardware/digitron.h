#ifndef __DIGITRON_H
#define __DIGITRON_H

//����ܶ���

#include "sys.h"


extern u8 g_Digitron[8];


void DPinit(void);	//��ʼ��

void display(u8 disnum[8]);

//�ر��������ʾ
void SdgOffAll(void);


#endif
