#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "led.h"
#include "beep.h"
#include "usmart.h"
#include "rtc.h"


 int main(void)
 {		
	u8 t=0;	
	 
	delay_init();	       //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
 	LED_Init();			 //LED�˿ڳ�ʼ��
	KEY_Init();          //��ʼ���밴�����ӵ�Ӳ���ӿ�
	usmart_dev.init(SystemCoreClock/1000000);	//���ڵ������USMART��ʼ��
	RTC_Init();	  //RTC��ʼ��


	while(1)
	{
		if(t!=calendar.sec)
		{
			t=calendar.sec;
			printf("Now Time:%d-%d-%d %d:%d:%d\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//�������ʱ�� 
		}
		   
		delay_ms(10);	   
	}
	 
	 
// ���ڲ��Դ���
//	u16 t;  
//	u16 len;	
//	u16 times=0;
// 	while(1)
//	{
//		if(USART_RX_STA&0x8000)
//		{					   
//			len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
//			printf("\r\n�����͵���ϢΪ:\r\n\r\n");
//			for(t=0;t<len;t++)
//			{
//				USART_SendData(USART1, USART_RX_BUF[t]);//�򴮿�1��������
//				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
//			}
//			printf("\r\n\r\n");//���뻻��
//			USART_RX_STA=0;
//		}
//		else
//		{
//			times++;
//			if(times%5000==0)
//			{
//				printf("\r\nս��STM32������ ����ʵ��\r\n");
//				printf("����ԭ��@ALIENTEK\r\n\r\n");
//			}
//			if(times%200==0)
//				printf("����������,�Իس�������\r\n");  
//			if(times%30==0)
//				LED0=!LED0;//��˸LED,��ʾϵͳ��������.
//			delay_ms(10);   
//		}
//	}	 
// }

 
 // ����,LED,���������Դ���
 // int main(void)
 // {
 	// vu8 key=0;	
	// delay_init();   //��ʱ������ʼ��	  
 	// LED_Init();		//LED�˿ڳ�ʼ��
	// KEY_Init();     //��ʼ���밴�����ӵ�Ӳ���ӿ�
	// BEEP_Init();    //��ʼ���������˿�
	// LED0=0;		    //�ȵ������
	
	// while(1)
	// {
 		// key=KEY_Scan(0);	//�õ���ֵ
	   	// if(key)
		// {						   
			// switch(key)
			// {				 
				// case WKUP_PRES:	//���Ʒ�����
					// BEEP=!BEEP;
					// break;
				// case KEY2_PRES:	//����LED0��ת
					// LED0=!LED0;
					// break;
				// case KEY1_PRES:	//����LED1��ת	 
					// LED1=!LED1;
					// break;
				// case KEY0_PRES:	//ͬʱ����LED0,LED1��ת 
					// LED0=!LED0;
					// LED1=!LED1;
					// break;
			// }
		// }
		// else
		// {
			// delay_ms(10); 	
		// }			
	// }	 
// }

}

