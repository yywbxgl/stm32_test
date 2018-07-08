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
	 
	delay_init();	       //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 //串口初始化为115200
 	LED_Init();			 //LED端口初始化
	KEY_Init();          //初始化与按键连接的硬件接口
	usmart_dev.init(SystemCoreClock/1000000);	//串口调试组件USMART初始化
	RTC_Init();	  //RTC初始化


	while(1)
	{
		if(t!=calendar.sec)
		{
			t=calendar.sec;
			printf("Now Time:%d-%d-%d %d:%d:%d\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//输出闹铃时间 
		}
		   
		delay_ms(10);	   
	}
	 
	 
// 串口测试代码
//	u16 t;  
//	u16 len;	
//	u16 times=0;
// 	while(1)
//	{
//		if(USART_RX_STA&0x8000)
//		{					   
//			len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
//			printf("\r\n您发送的消息为:\r\n\r\n");
//			for(t=0;t<len;t++)
//			{
//				USART_SendData(USART1, USART_RX_BUF[t]);//向串口1发送数据
//				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
//			}
//			printf("\r\n\r\n");//插入换行
//			USART_RX_STA=0;
//		}
//		else
//		{
//			times++;
//			if(times%5000==0)
//			{
//				printf("\r\n战舰STM32开发板 串口实验\r\n");
//				printf("正点原子@ALIENTEK\r\n\r\n");
//			}
//			if(times%200==0)
//				printf("请输入数据,以回车键结束\r\n");  
//			if(times%30==0)
//				LED0=!LED0;//闪烁LED,提示系统正在运行.
//			delay_ms(10);   
//		}
//	}	 
// }

 
 // 按键,LED,蜂鸣器测试代码
 // int main(void)
 // {
 	// vu8 key=0;	
	// delay_init();   //延时函数初始化	  
 	// LED_Init();		//LED端口初始化
	// KEY_Init();     //初始化与按键连接的硬件接口
	// BEEP_Init();    //初始化蜂鸣器端口
	// LED0=0;		    //先点亮红灯
	
	// while(1)
	// {
 		// key=KEY_Scan(0);	//得到键值
	   	// if(key)
		// {						   
			// switch(key)
			// {				 
				// case WKUP_PRES:	//控制蜂鸣器
					// BEEP=!BEEP;
					// break;
				// case KEY2_PRES:	//控制LED0翻转
					// LED0=!LED0;
					// break;
				// case KEY1_PRES:	//控制LED1翻转	 
					// LED1=!LED1;
					// break;
				// case KEY0_PRES:	//同时控制LED0,LED1翻转 
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

