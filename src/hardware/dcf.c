#include "dcf.h"
/* Êý¾ÝÀàÐÍ¶¨Òå */
#define uchar	unsigned char
#define uint	unsigned int

//³õÊ¼»¯PB0ÎªÊä³ö¿Ú.²¢Ê¹ÄÜÕâ¸ö¿ÚµÄÊ±ÖÓ		    
//µç´Å·§³õÊ¼»¯
void DCF_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //Ê¹ÄÜGPIOB¶Ë¿ÚÊ±ÖÓ
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 //BEEP-->PB.8 ¶Ë¿ÚÅäÖÃ
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //ÍÆÍìÊä³ö
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 	 //ËÙ¶ÈÎª50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);	 				 //¸ù¾Ý²ÎÊý³õÊ¼»¯GPIOB.8
 
 GPIO_ResetBits(GPIOB,GPIO_Pin_0);						 //Êä³ö0£¬¹Ø±Õ·äÃùÆ÷Êä³ö

}


//´ò¿ªµç´Å·
//DCF_STA_OK     		µç´Å·§Õý³£
//DCF_OPEN_ERR			µç´Å·§¿ªÆôÊ§°Ü
//DCF_CLOSE_ERR			µç´Å·§ÎÞ·¨¹Ø±Õ§
void DCF_Set(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_0);						//Êä³ö1£¬´ò¿ªµç´Å·§Êä³ö
}

//¹Ø±Õµç´Å·§
//DCF_STA_OK     		µç´Å·§Õý³£
//DCF_OPEN_ERR			µç´Å·§¿ªÆôÊ§°Ü
//DCF_CLOSE_ERR			µç´Å·§ÎÞ·¨¹Ø±Õ
void DCF_Reset(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_0);						//Êä³ö0£¬¹Ø±Õµç´Å·§Êä³ö
}
/*
//µç´Å·§¼ì²â
//DCF_STA_OK     		µç´Å·§Õý³£
//DCF_OPEN_ERR			µç´Å·§¿ªÆôÊ§°Ü
//DCF_CLOSE_ERR			µç´Å·§ÎÞ·¨¹Ø±Õ
unsigned char DCF_Check(void)
{
}
*/

