#ifndef __DIGITRON_H
#define __DIGITRON_H

//����ܶ���

#include "sys.h"


extern u8 g_Digitron[8];


void DPinit(void);	//��ʼ��

//���������ʾ
void setOnFlag(void);

//�ر��������ʾ
void setOffFlag(void);

//�������ʾ���
void display(u32 money);


#endif
