#include "digitron.h"

//?????
u8 DgNum;	//4? 0~3
u8 Digitron[8]={0,1,2,3,4,5,6,7};														//8?
u32 NumTab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};	//??	

u8 SecFlag1s;	//???
u16	SecCount1s ;		//???
/*00?????*/
//????
void delay_c()
{
	u8 d_r_count2;
	for(d_r_count2=0;d_r_count2<3;d_r_count2++)
		;
}

//????????
void SdgOffAll()
{
	GPIO_SetBits(GPIOD,GPIO_Pin_2);
	GPIO_SetBits(GPIOC,GPIO_Pin_12);
	GPIO_SetBits(GPIOC,GPIO_Pin_11);
	GPIO_SetBits(GPIOC,GPIO_Pin_10);
}

//??????? 1~4
void SdgOnSel()
{
	switch(DgNum)	  
		{			
		case 0:   	GPIO_ResetBits(GPIOD,GPIO_Pin_2);					   	//POWER2 OFF
				   break;
		case 1:	   GPIO_ResetBits(GPIOC,GPIO_Pin_12);					   	//POWER2 OFF
				   break;
		case 2:   	GPIO_ResetBits(GPIOC,GPIO_Pin_10);					   	//POWER2 OFF
				   break;
		case 3:	   GPIO_ResetBits(GPIOC,GPIO_Pin_11);					   	//POWER2 OFF
				   break;
		default:   break;
		}
}

//?????????
void Serial_write()
{
	u8 d_r_count;
	u32 data_show	=	0;
	
	data_show	=	NumTab[Digitron[DgNum]];
	data_show	<<=	8;
	
	data_show	|=	NumTab[Digitron[DgNum+4]];	
	SdgOffAll();			

	for(d_r_count=0;d_r_count<16;d_r_count++)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_4);		   	//clk=0;						   				   			
		if(data_show&0x8000)						//DATA
			GPIO_SetBits(GPIOA,GPIO_Pin_15);		   	
		else
			GPIO_ResetBits(GPIOA,GPIO_Pin_15);		   
		data_show=data_show<<1;
		delay_c();					
		GPIO_SetBits(GPIOB,GPIO_Pin_4);
		delay_c();	
	}						   

	GPIO_ResetBits(GPIOB,GPIO_Pin_4);		   			//clk=0;	
	SdgOnSel();		
}

void tim2_init()
 {
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 30; //50		0.5ms
  TIM_TimeBaseStructure.TIM_Prescaler = 720;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
  TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
  TIM_Cmd(TIM2,ENABLE);
  /* Enable the TIM2 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
 }
 
void TIM2_IRQHandler()		//T=0.5ms
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)
	{
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
		
		//?????
		
		DgNum++;
		if(DgNum>3)
			DgNum=0;				
		Serial_write();		

		//??1s
		SecCount1s++;
		if(SecCount1s>333)
		{
			SecCount1s=0;
			SecFlag1s=1;
		}
	}
}

void DPinit()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	//???	 PP??
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//GPIOA ??
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 //BEEP-->PB.8 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 	 //速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);	 				 //根据参数初始化GPIOB.8
	
	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				  	//SDG3
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				  	//SDG4
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				  	//SDG2
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				  		//SDG1
  GPIO_Init(GPIOD, &GPIO_InitStructure);

	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				  	//DATA
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				  		//CLR
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				  		//CLK
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* init2 */
  tim2_init();
  GPIO_SetBits(GPIOB,GPIO_Pin_3);					   	//CLR=1,???
}

void display(u8 disnum[8])
{
	u8 i;
	for(i=0;i<8;i++)
		Digitron[i]=disnum[i];
	/*
if(SecFlag1s)
		{
			SecFlag1s=0;
			Digitron[0]++;
			if(Digitron[0]>9)
			{
				Digitron[0]=0;				
			}

			if(Digitron[0]==0)
			  	GPIO_SetBits(GPIOB,GPIO_Pin_0);
			else if(Digitron[0]==5)
			  	GPIO_ResetBits(GPIOB,GPIO_Pin_0);
			
		}*/
}



