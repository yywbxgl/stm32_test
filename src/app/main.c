#include "sys.h"
#include "delay.h"
#include "time.h"
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
#include "mqtt_app.h"
#include "malloc.h"
#include "logging.h"
#include <string.h>


//#define HOST_IP "124.90.130.2"
//#define HOST_PORT "8086"
#define HOST_IP "114.215.73.18"
#define HOST_PORT "1883"
u8 mqtt_msg[200]={0}; //mqtt消息包
u8 send_cmd[20]= {0};

#define CLIENTID "GID_water_test@@@TEST00003"
#define USRNAME "LTAIwfxXisNo3pDh"
#define PASSWD "kqfxXvRTsAnQJHSbbDkj1B0iXfk="
#define TOPIC "water_topic_test"

u8 connect_to_server(u8 mode,u8* ipaddr,u8* port);

void PrintHex(u8 *buf,u16 len)
{
	u16 i;
	printf("\r\n");
	for(i=0;i<len;i++)
	{
		printf("%02x ",buf[i]);
	}
	printf("\r\n\r\n");
}


int main(void)
{

    u8 ipbuf[16]= HOST_IP;//IP缓存
    const u8 *port= HOST_PORT;  //端口固定为8086,当你的电脑8086端口被其他程序占用的时候,请修改为其他空闲端口
    u8 mode= 0;              //0,TCP连接;1,UDP连接
    u16 len;

    delay_init();           //延时函数初始化   
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    uart_init(115200);      //调试打印串口，初始化为115200
    usart3_init(115200);    //初始化串口3,与SIM800C通信
    usmart_dev.init(SystemCoreClock/1000000);   //串口调试组件USMART初始化
    RTC_Init();             //RTC初始化
    DCF_Init();             //电磁阀初始化
    DPinit();               //数码管初始化

    LOGD("hardware init finish.");

    //DCF_Set();

    //fm1702_test();

    //usart_test();
 
    //rtc_test();

    //sim800c_test();

    //fm1702_test();

    //与服务器建立握手
    while(connect_to_server(mode, ipbuf, (u8*)port))
    {
        delay_ms(1000);
    }

    //发起mqtt_connect请求
    len=mqtt_connect_message(mqtt_msg, CLIENTID , USRNAME, PASSWD);//id,用户名和密码
    printf("send len = %d\r\n", len);
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                      //必须加延时
        //sim800c_send_cmd((u8*)0X1A,0,0);      //CTRL+Z,结束数据发送,启动一次传输
        printf("mqtt_connect... \r\n");
        delay_ms(1000);                      //必须加延时
    }


    len=mqtt_subscribe_message(mqtt_msg,TOPIC,1,1);//订阅test主题
    printf("send len = %d\r\n", len);
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                 //必须加延时
        //sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,结束数据发送,启动一次传输
        printf("mqtt_subscribe... \r\n");
        delay_ms(1000);               //必须加延时
    }


    //开启定时器，每间隔多少发一个ping包
    while(1)
    {
        len=mqtt_publish_message(mqtt_msg, TOPIC, "device_sun_smile", 0);
        printf("send len = %d\r\n", len);
        PrintHex(mqtt_msg,len);
        sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
        if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
        {
            u3_printf_hex(mqtt_msg, len);
            delay_ms(1000);                      //必须加延时
            //sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,结束数据发送,启动一次传输
            //printf("mqtt_publish...\r\n");
            delay_ms(1000);
            delay_ms(1000);
        }
    }

#if 0
    if(USART3_RX_STA&0X8000)        //接收到一次数据了
    {
       USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;  //添加结束符 
       if(hbeaterrcnt)                         //需要检测心跳应答
       {
           if(strstr((const char*)USART3_RX_BUF,"SEND OK"))hbeaterrcnt=0;//心跳正常
       }               
       p2=(u8*)strstr((const char*)USART3_RX_BUF,"+IPD");
       if(p2)//接收到TCP/UDP数据
       {
           p3=(u8*)strstr((const char*)p2,",");
           p2=(u8*)strstr((const char*)p2,":");
           p2[0]=0;//加入结束符
           sprintf((char*)p1,"收到%s字节,内容如下",p3+1);//接收到的字节数
           printf("sim800c_tcpudp_test. %s\r\n", p1);
           printf("sim800c_tcpudp_test. %s\r\n", p2+1);
       }
       USART3_RX_STA=0;
    }
#endif


}



u8 connect_to_server(u8 mode,u8* ipaddr,u8* port)
{
    u8 *p,*p1;
    u16 timex=6;
    u8 connectsta=0;            //0,????;1,????;2,????; 
    p=mymalloc(SRAMIN,100);     //申请100字节内存
    p1=mymalloc(SRAMIN,100);    //申请100字节内存

    while(sim800c_send_cmd("AT","OK",100)) //检测是否应答AT指令 
    {   
        printf("no response. sim800c未回应AT指令.\r\n");
        delay_ms(400);
    }

    while(sim800c_send_cmd("AT+CPIN?","OK",200))
    {
        printf("no response. sim800c未检测到SIM卡.\r\n");
        delay_ms(400);
    }

    sim800c_send_cmd("ATE0","OK",200);

    USART3_RX_STA=0;
    while(sim800c_send_cmd("AT+CGSN","OK",200))
    {
        printf("no response. sim800c未返回序列号.\r\n");
        delay_ms(400);
    }
    p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");
    p1[0]=0;//加入结束符 
    printf("序列号: %s\r\n", USART3_RX_BUF+2);
    USART3_RX_STA=0;
		
////    if (sim800c_send_cmd("ATE0","OK",200));//不回显
////        printf("no response.sim800c未关闭回显.\r\n");
////		
////    if(sim800c_send_cmd("AT+CGSN","OK",200))//查询SIM800C产品序列号
////    {
////        printf("no response. sim800c未返回序列号.\r\n");
////    }
////    else
////    {
////        p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");
////        p1[0]=0;//加入结束符 
////        printf("序列号: %s\r\n", USART3_RX_BUF+2);
////        USART3_RX_STA=0;
////    }

////    if(sim800c_send_cmd("AT+CPIN?","OK",200))//检查SIM卡是否准备好
////    {
////        printf("no response. sim800c未返回本机号码.\r\n");
////    }
////
////    if(sim800c_send_cmd("AT+CSQ","+CSQ:",200))//查询信号质量
////    {
////        printf("no response. sim800c未返回信号质量\r\n");
////    }
////
////    if(sim800c_send_cmd("AT+CCLK?","+CCLK:",200))
////    {
////        printf("no response. sim800c未返回日期\r\n");
////    }

    //更新NTP时间
    //ntp_update();

    sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",100);   //关闭连接
    sim800c_send_cmd("AT+CIPSHUT","SHUT OK",100);       //关闭移动场景 
    if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",1000))return 1;             //设置GPRS移动台类别为B,支持包交换和数据交换 
    if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",1000))return 2;//设置PDP上下文,互联网接协议,接入点等信息
    if(sim800c_send_cmd("AT+CGATT=1","OK",500))return 3;                    //附着GPRS业务
    if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500))return 4;        //设置为GPRS连接模式
    //if(sim800c_send_cmd(" AT+CIPMODE=1,\"CMNET\"","OK",500))printf("设置透传模式失败\r\n");        //设置为GPRS连接模式
    if(sim800c_send_cmd("AT+CIPHEAD=1","OK",500))return 5;                  //设置接收数据显示IP头(方便判断数据来源)   
    u3_printf("AT+CLDTMF=2,\"%s\"\r\n", ipaddr); 

    USART3_RX_STA=0;
    sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",ipaddr,port);
    if(sim800c_send_cmd(p,"OK",500))return 6;     //发起连接

    while (1)
    {
       if(connectsta==0&&timex)//连接还没建立的时候,每2秒查询一次CIPSTATUS.
       {
           sim800c_send_cmd("AT+CIPSTATUS","OK",500);  //查询连接状态
           if(strstr((const char*)USART3_RX_BUF,"CLOSED"))connectsta=2;
           if(strstr((const char*)USART3_RX_BUF,"CONNECT OK"))connectsta=1;
           delay_ms(500);
           timex --;
       }
       else if (connectsta == 1)
       {
            return 0; //连接建立成功
       }
       else if (connectsta == 2)
       {
           printf(" conncet return closed.");
           return 7;
       }
       else
       {
           break;
       }
    }
		return 7;
}



