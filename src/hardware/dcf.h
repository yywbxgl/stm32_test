#ifndef __DCF_H
#define __DCF_H	 
#include "sys.h"

//�������˿ڶ���
#define DCF PBout(0)	// ��ŷ��ӿ�		   

void DCF_Init(void);	//��ʼ��

//�򿪵�ŷ���ʹ��ǰ��DCF_Init��ʼ��
void DCF_Set(void);	

//�رյ�ŷ���ʹ��ǰ��DCF_Init��ʼ��
void DCF_Reset(void);


#endif
