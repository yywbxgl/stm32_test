#include "dcf.h"
/* �������Ͷ��� */
#define uchar	unsigned char
#define uint	unsigned int

//��ʼ��PB0Ϊ�����.��ʹ������ڵ�ʱ��		    
//��ŷ���ʼ��
void DCF_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��GPIOB�˿�ʱ��
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 //BEEP-->PB.8 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 	 //�ٶ�Ϊ50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);	 				 //���ݲ�����ʼ��GPIOB.8
 
 GPIO_ResetBits(GPIOB,GPIO_Pin_0);						 //���0���رշ��������

}


//�򿪵�ŷ
//DCF_STA_OK     		��ŷ�����
//DCF_OPEN_ERR			��ŷ�����ʧ��
//DCF_CLOSE_ERR			��ŷ��޷��رէ
void DCF_Set(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_0);						//���1���򿪵�ŷ����
}

//�رյ�ŷ�
//DCF_STA_OK     		��ŷ�����
//DCF_OPEN_ERR			��ŷ�����ʧ��
//DCF_CLOSE_ERR			��ŷ��޷��ر�
void DCF_Reset(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_0);						//���0���رյ�ŷ����
}
/*
//��ŷ����
//DCF_STA_OK     		��ŷ�����
//DCF_OPEN_ERR			��ŷ�����ʧ��
//DCF_CLOSE_ERR			��ŷ��޷��ر�
unsigned char DCF_Check(void)
{
}
*/

