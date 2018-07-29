#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "beep.h"
#include "usmart.h"
#include "rtc.h"
#include "usart3.h"
#include "sim800c.h"
#include "fm1702.h"	
#include "test.h"
#include "dcf.h"

#include "digitron.h"

u8 disnum[8]={0,0,5,7,2,4,6,8};//�������ʾ����

int main(void)
{
    
	
		delay_init();           //��ʱ������ʼ��	  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(115200);      //���ڳ�ʼ��Ϊ115200
    usart3_init(115200);    //��ʼ������3,��SIM800Cͨ��
    usmart_dev.init(SystemCoreClock/1000000);   //���ڵ������USMART��ʼ��
    RTC_Init();             //RTC��ʼ��
	  DCF_Init(); 						//��ŷ���ʼ��

		DPinit();									//����ܳ�ʼ��
		printf("Now Time:%d-%d-%d %d:%d:%d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//�������ʱ�� 
	 
/*	while(1)
		{		
			
			printf("Now Time:%d-%d-%d %d:%d:%d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//�������ʱ�� 
//			DCF_Set();
//			delay_ms(1000);
			fm1702_test();
					
			//��ʾ��ʱ��
			disnum[7] = calendar.sec%10;
			disnum[6] = calendar.sec/10;
		  
			disnum[5] = calendar.min%10;
			disnum[4] = calendar.min/10;
			
			disnum[3] = calendar.hour%10;
			disnum[2] = calendar.hour/10;			
	
			disnum[1] = calendar.w_date%10;
			disnum[0] = calendar.w_date/10;				
			display(disnum);
		}*/
    //usart_test();

    //rtc_test();
while(1)
    sim800c_test();

 //   fm1702_test();

}




