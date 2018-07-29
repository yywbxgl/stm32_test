#include "dcf.h"
/* 数据类型定义 */
#define uchar	unsigned char
#define uint	unsigned int

//初始化PB0为输出口.并使能这个口的时钟		    
//电磁阀初始化
void DCF_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能GPIOB端口时钟
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 //BEEP-->PB.8 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 	 //速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);	 				 //根据参数初始化GPIOB.8
 
 GPIO_ResetBits(GPIOB,GPIO_Pin_0);						 //输出0，关闭蜂鸣器输出

}


//打开电磁�
//DCF_STA_OK     		电磁阀正常
//DCF_OPEN_ERR			电磁阀开启失败
//DCF_CLOSE_ERR			电磁阀无法关闭�
void DCF_Set(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_0);						//输出1，打开电磁阀输出
}

//关闭电磁阀
//DCF_STA_OK     		电磁阀正常
//DCF_OPEN_ERR			电磁阀开启失败
//DCF_CLOSE_ERR			电磁阀无法关闭
void DCF_Reset(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_0);						//输出0，关闭电磁阀输出
}
/*
//电磁阀检测
//DCF_STA_OK     		电磁阀正常
//DCF_OPEN_ERR			电磁阀开启失败
//DCF_CLOSE_ERR			电磁阀无法关闭
unsigned char DCF_Check(void)
{
}
*/

