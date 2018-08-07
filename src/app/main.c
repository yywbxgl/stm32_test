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
u8 mqtt_msg[200]={0}; //mqtt��Ϣ��
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

    u8 ipbuf[16]= HOST_IP;//IP����
    const u8 *port= HOST_PORT;  //�˿ڹ̶�Ϊ8086,����ĵ���8086�˿ڱ���������ռ�õ�ʱ��,���޸�Ϊ�������ж˿�
    u8 mode= 0;              //0,TCP����;1,UDP����
    u16 len;

    delay_init();           //��ʱ������ʼ��   
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(115200);      //���Դ�ӡ���ڣ���ʼ��Ϊ115200
    usart3_init(115200);    //��ʼ������3,��SIM800Cͨ��
    usmart_dev.init(SystemCoreClock/1000000);   //���ڵ������USMART��ʼ��
    RTC_Init();             //RTC��ʼ��
    DCF_Init();             //��ŷ���ʼ��
    DPinit();               //����ܳ�ʼ��

    LOGD("hardware init finish.");

    //DCF_Set();

    //fm1702_test();

    //usart_test();
 
    //rtc_test();

    //sim800c_test();

    //fm1702_test();

    //���������������
    while(connect_to_server(mode, ipbuf, (u8*)port))
    {
        delay_ms(1000);
    }

    //����mqtt_connect����
    len=mqtt_connect_message(mqtt_msg, CLIENTID , USRNAME, PASSWD);//id,�û���������
    printf("send len = %d\r\n", len);
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//���յ����ֽ���
    if(sim800c_send_cmd(send_cmd,">",200)==0)//��������
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                      //�������ʱ
        //sim800c_send_cmd((u8*)0X1A,0,0);      //CTRL+Z,�������ݷ���,����һ�δ���
        printf("mqtt_connect... \r\n");
        delay_ms(1000);                      //�������ʱ
    }


    len=mqtt_subscribe_message(mqtt_msg,TOPIC,1,1);//����test����
    printf("send len = %d\r\n", len);
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//���յ����ֽ���
    if(sim800c_send_cmd(send_cmd,">",200)==0)//��������
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                 //�������ʱ
        //sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,�������ݷ���,����һ�δ���
        printf("mqtt_subscribe... \r\n");
        delay_ms(1000);               //�������ʱ
    }


    //������ʱ����ÿ������ٷ�һ��ping��
    while(1)
    {
        len=mqtt_publish_message(mqtt_msg, TOPIC, "device_sun_smile", 0);
        printf("send len = %d\r\n", len);
        PrintHex(mqtt_msg,len);
        sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//���յ����ֽ���
        if(sim800c_send_cmd(send_cmd,">",200)==0)//��������
        {
            u3_printf_hex(mqtt_msg, len);
            delay_ms(1000);                      //�������ʱ
            //sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,�������ݷ���,����һ�δ���
            //printf("mqtt_publish...\r\n");
            delay_ms(1000);
            delay_ms(1000);
        }
    }

#if 0
    if(USART3_RX_STA&0X8000)        //���յ�һ��������
    {
       USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;  //��ӽ����� 
       if(hbeaterrcnt)                         //��Ҫ�������Ӧ��
       {
           if(strstr((const char*)USART3_RX_BUF,"SEND OK"))hbeaterrcnt=0;//��������
       }               
       p2=(u8*)strstr((const char*)USART3_RX_BUF,"+IPD");
       if(p2)//���յ�TCP/UDP����
       {
           p3=(u8*)strstr((const char*)p2,",");
           p2=(u8*)strstr((const char*)p2,":");
           p2[0]=0;//���������
           sprintf((char*)p1,"�յ�%s�ֽ�,��������",p3+1);//���յ����ֽ���
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
    p=mymalloc(SRAMIN,100);     //����100�ֽ��ڴ�
    p1=mymalloc(SRAMIN,100);    //����100�ֽ��ڴ�

    while(sim800c_send_cmd("AT","OK",100)) //����Ƿ�Ӧ��ATָ�� 
    {   
        printf("no response. sim800cδ��ӦATָ��.\r\n");
        delay_ms(400);
    }

    while(sim800c_send_cmd("AT+CPIN?","OK",200))
    {
        printf("no response. sim800cδ��⵽SIM��.\r\n");
        delay_ms(400);
    }

    sim800c_send_cmd("ATE0","OK",200);

    USART3_RX_STA=0;
    while(sim800c_send_cmd("AT+CGSN","OK",200))
    {
        printf("no response. sim800cδ�������к�.\r\n");
        delay_ms(400);
    }
    p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");
    p1[0]=0;//��������� 
    printf("���к�: %s\r\n", USART3_RX_BUF+2);
    USART3_RX_STA=0;
		
////    if (sim800c_send_cmd("ATE0","OK",200));//������
////        printf("no response.sim800cδ�رջ���.\r\n");
////		
////    if(sim800c_send_cmd("AT+CGSN","OK",200))//��ѯSIM800C��Ʒ���к�
////    {
////        printf("no response. sim800cδ�������к�.\r\n");
////    }
////    else
////    {
////        p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");
////        p1[0]=0;//��������� 
////        printf("���к�: %s\r\n", USART3_RX_BUF+2);
////        USART3_RX_STA=0;
////    }

////    if(sim800c_send_cmd("AT+CPIN?","OK",200))//���SIM���Ƿ�׼����
////    {
////        printf("no response. sim800cδ���ر�������.\r\n");
////    }
////
////    if(sim800c_send_cmd("AT+CSQ","+CSQ:",200))//��ѯ�ź�����
////    {
////        printf("no response. sim800cδ�����ź�����\r\n");
////    }
////
////    if(sim800c_send_cmd("AT+CCLK?","+CCLK:",200))
////    {
////        printf("no response. sim800cδ��������\r\n");
////    }

    //����NTPʱ��
    //ntp_update();

    sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",100);   //�ر�����
    sim800c_send_cmd("AT+CIPSHUT","SHUT OK",100);       //�ر��ƶ����� 
    if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",1000))return 1;             //����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
    if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",1000))return 2;//����PDP������,��������Э��,��������Ϣ
    if(sim800c_send_cmd("AT+CGATT=1","OK",500))return 3;                    //����GPRSҵ��
    if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500))return 4;        //����ΪGPRS����ģʽ
    //if(sim800c_send_cmd(" AT+CIPMODE=1,\"CMNET\"","OK",500))printf("����͸��ģʽʧ��\r\n");        //����ΪGPRS����ģʽ
    if(sim800c_send_cmd("AT+CIPHEAD=1","OK",500))return 5;                  //���ý���������ʾIPͷ(�����ж�������Դ)   
    u3_printf("AT+CLDTMF=2,\"%s\"\r\n", ipaddr); 

    USART3_RX_STA=0;
    sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",ipaddr,port);
    if(sim800c_send_cmd(p,"OK",500))return 6;     //��������

    while (1)
    {
       if(connectsta==0&&timex)//���ӻ�û������ʱ��,ÿ2���ѯһ��CIPSTATUS.
       {
           sim800c_send_cmd("AT+CIPSTATUS","OK",500);  //��ѯ����״̬
           if(strstr((const char*)USART3_RX_BUF,"CLOSED"))connectsta=2;
           if(strstr((const char*)USART3_RX_BUF,"CONNECT OK"))connectsta=1;
           delay_ms(500);
           timex --;
       }
       else if (connectsta == 1)
       {
            return 0; //���ӽ����ɹ�
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



