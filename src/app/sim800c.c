#include "sim800c.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "malloc.h"
#include "string.h"
#include "usart3.h"
#include "timer.h"
#include "logging.h"
#include "utils.h"



//usmart֧�ֲ���
//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������USART3_RX_STA;
//     1,����USART3_RX_STA;
void sim_at_response(u8 mode)
{
    if(USART3_RX_STA&0X8000)        //���յ�һ��������
    { 
        USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
        printf("%s",USART3_RX_BUF); //���͵�����
        if(mode)USART3_RX_STA=0;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//ATK-SIM800C �������(���Ų��ԡ����Ų��ԡ�GPRS����,��������)���ô���

//SIM800C���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//����,�ڴ�Ӧ������λ��(str��λ��)
u8* sim800c_check_cmd(u8 *str)
{
    char *strx=0;
    if(USART3_RX_STA&0X8000)        //���յ�һ��������
    { 
        USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
        LOGD("get response=%s", (const char*)USART3_RX_BUF);
        strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
    }
    return (u8*)strx;
}
//SIM800C��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 sim800c_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{

    u8 res=0; 
    USART3_RX_STA=0;
    
    LOGD("send cmd=[%s]", cmd);
    
    if((u32)cmd<=0XFF)
    {
        while((USART2->SR&0X40)==0);//�ȴ���һ�����ݷ������  
        USART2->DR=(u32)cmd;
    }else u3_printf("%s\r\n",cmd);//��������

    if(ack&&waittime)       //��Ҫ�ȴ�Ӧ��
    {
        while(--waittime)   //�ȴ�����ʱ
        {
            delay_ms(10);
            if(USART3_RX_STA&0X8000)//���յ��ڴ���Ӧ����
            {
                if(sim800c_check_cmd(ack))break;//�õ���Ч���� 
                USART3_RX_STA=0;
            } 
        }
        if(waittime==0)res=1; 
    }
    return res;
}


//SIM800C����hex����������mqtt��Ϣ
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 sim800c_send_hex(u8 *buf, u8 len, u8 *ack, u16 waittime)
{

    u8 res=0; 
    USART3_RX_STA=0;

    LOGD("send hex.");
    PrintHex(buf, len);
    u3_printf_hex(buf, len);//��������

    if(ack&&waittime)       //��Ҫ�ȴ�Ӧ��
    {
        while(--waittime)   //�ȴ�����ʱ
        {
            delay_ms(10);
            if(USART3_RX_STA&0X8000)//���յ��ڴ���Ӧ����
            {
                if(sim800c_check_cmd(ack))break;//�õ���Ч���� 
                USART3_RX_STA=0;
            } 
        }
        if(waittime==0)res=1; 
    }
    return res;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//GPRS���Բ��ִ���

const u8 *modetbl[2]={"TCP","UDP"};//����ģʽ
//tcp/udp����
//����������,��ά������
//mode:0:TCP����;1,UDP����)
//ipaddr:ip��ַ
//port:�˿� 
void sim800c_tcpudp_test(u8 mode,u8* ipaddr,u8* port)
{ 
    u8 *p,*p1,*p2,*p3;
    u16 timex=0;
    u8 connectsta=0;            //0,????;1,????;2,????; 
    u8 hbeaterrcnt=0;           //�������������,����5�������ź���Ӧ��,����������
    p=mymalloc(SRAMIN,100);     //����100�ֽ��ڴ�
    p1=mymalloc(SRAMIN,100);    //����100�ֽ��ڴ�

    USART3_RX_STA=0;
    sprintf((char*)p,"AT+CIPSTART=\"%s\",\"%s\",\"%s\"",modetbl[mode],ipaddr,port);
    if(sim800c_send_cmd(p,"OK",500))return;     //��������
    while(1)
    { 
        if(connectsta==0&&(timex%200)==0)//���ӻ�û������ʱ��,ÿ2���ѯһ��CIPSTATUS.
        {
            sim800c_send_cmd("AT+CIPSTATUS","OK",500);  //��ѯ����״̬
            if(strstr((const char*)USART3_RX_BUF,"CLOSED"))connectsta=2;
            if(strstr((const char*)USART3_RX_BUF,"CONNECT OK"))connectsta=1;
        }
        if(connectsta==2||hbeaterrcnt>8)//�����ж���,��������8������û����ȷ���ͳɹ�,����������
        {
            sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",500);   //�ر�����
            sim800c_send_cmd("AT+CIPSHUT","SHUT OK",500);       //�ر��ƶ����� 
            //sim800c_send_cmd(p,"OK",500);                     //������������
            connectsta=0;   
            hbeaterrcnt=0;
            break; //�ر����ӣ���������
        }
        if(connectsta==1&&timex>=600)//����������ʱ��,ÿ6�뷢��һ������
        {
            timex=0;
            if(sim800c_send_cmd("AT+CIPSEND",">",200)==0)//��������
            {
                sim800c_send_cmd("keepalive",0,0);  //��������:0X00  
                delay_ms(20);                       //�������ʱ
                sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,�������ݷ���,����һ�δ���  
            }else sim800c_send_cmd((u8*)0X1B,0,0);  //ESC,ȡ������      
            hbeaterrcnt++; 
            printf("hbeaterrcnt:%d\r\n",hbeaterrcnt);//������Դ���
        } 
        delay_ms(10);
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
        timex++; 
    } 
    myfree(SRAMIN,p);
    myfree(SRAMIN,p1);
}


//SIM800C GPRS����
//���ڲ���TCP/UDP����
//����ֵ:0,����
//����,�������
u8 sim800c_gprs_test(void)
{
    const u8 *port="8086";  //�˿ڹ̶�Ϊ8086,����ĵ���8086�˿ڱ���������ռ�õ�ʱ��,���޸�Ϊ�������ж˿�
    u8 mode=0;              //0,TCP����;1,UDP����
    u8 ipbuf[16]="124.90.130.2";//IP����
    printf("sim800c_gprs_test. mode=[%s] ip=[%s:%s]\r\n", (u8*)modetbl[mode], ipbuf, (u8*)port);
    sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",100);   //�ر�����
    sim800c_send_cmd("AT+CIPSHUT","SHUT OK",100);       //�ر��ƶ����� 
    if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",1000))return 1;             //����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
    if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",1000))return 2;//����PDP������,��������Э��,��������Ϣ
    if(sim800c_send_cmd("AT+CGATT=1","OK",500))return 3;                    //����GPRSҵ��
    if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500))return 4;        //����ΪGPRS����ģʽ
    if(sim800c_send_cmd("AT+CIPHEAD=1","OK",500))return 5;                  //���ý���������ʾIPͷ(�����ж�������Դ)   

    u3_printf("AT+CLDTMF=2,\"%s\"\r\n", ipbuf); 

    sim800c_tcpudp_test(mode, ipbuf, (u8*)port);

    return 0;
} 


//���Խ�����UI
void sim800c_mtest_ui()
{
    u8 *p, *p1, *p2;
    p=mymalloc(SRAMIN,50);//����50���ֽڵ��ڴ�
    USART3_RX_STA=0;
    if(sim800c_send_cmd("AT+CGMI","OK",200)==0)//��ѯ����������
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");
        p1[0]=0;//���������
        sprintf((char*)p,"������:%s",USART3_RX_BUF+2);
        printf("sim800c_mtest_ui. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    } 
    if(sim800c_send_cmd("AT+CGMM","OK",200)==0)//��ѯģ������
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n"); 
        p1[0]=0;//���������
        sprintf((char*)p,"ģ���ͺ�:%s",USART3_RX_BUF+2);
        printf("sim800c_mtest_ui. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    } 
    if(sim800c_send_cmd("AT+CGSN","OK",200)==0)//��ѯ��Ʒ���к�
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");//���һس�
        p1[0]=0;//��������� 
        sprintf((char*)p,"���к�:%s",USART3_RX_BUF+2);
        printf("sim800c_mtest_ui. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    }
    if(sim800c_send_cmd("AT+CNUM","+CNUM",200)==0)//��ѯ��������
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF),",");
        p2=(u8*)strstr((const char*)(p1+2),"\"");
        p2[0]=0;//���������
        sprintf((char*)p,"��������:%s",p1+2);
        printf("sim800c_mtest_ui. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    }
    myfree(SRAMIN,p); 
}

//GSM��Ϣ��ʾ(�ź�����,��ص���,����ʱ��)
//����ֵ:0,����
//����,�������
u8 sim800c_gsminfo_show()
{
    u8 *p,*p1,*p2;
    u8 res=0;
    p=mymalloc(SRAMIN,50);//����50���ֽڵ��ڴ�
    USART3_RX_STA=0;
    if(sim800c_send_cmd("AT+CPIN?","OK",200))res|=1<<0; //��ѯSIM���Ƿ���λ 
    USART3_RX_STA=0;  
    if(sim800c_send_cmd("AT+COPS?","OK",200)==0)        //��ѯ��Ӫ������
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF),"\""); 
        if(p1)//����Ч����
        {
            p2=(u8*)strstr((const char*)(p1+1),"\"");
            p2[0]=0;//���������           
            sprintf((char*)p,"��Ӫ��:%s",p1+1);
            printf("sim800c_gsminfo_show. %s\r\n", (char*)p);
        } 
        USART3_RX_STA=0;        
    }else res|=1<<1;
    if(sim800c_send_cmd("AT+CSQ","+CSQ:",200)==0)       //��ѯ�ź�����
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF),":");
        p2=(u8*)strstr((const char*)(p1),",");
        p2[0]=0;//���������
        sprintf((char*)p,"�ź�����:%s",p1+2);
        printf("sim800c_gsminfo_show. %s\r\n", (char*)p);
        USART3_RX_STA=0;
    }else res|=1<<2;
    if(sim800c_send_cmd("AT+CBC","+CBC:",200)==0)       //��ѯ��ص���
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF),",");
        p2=(u8*)strstr((const char*)(p1+1),",");
        p2[0]=0;p2[5]=0;//���������
        sprintf((char*)p,"��ص���:%s%%  %smV",p1+1,p2+1);
        printf("sim800c_gsminfo_show. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    }else res|=1<<3; 
    if(sim800c_send_cmd("AT+CCLK?","+CCLK:",200)==0)        //��ѯ��ص���
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF),"\"");
        p2=(u8*)strstr((const char*)(p1+1),":");
        p2[3]=0;//���������
        sprintf((char*)p,"����ʱ��:%s",p1+1);
        printf("sim800c_gsminfo_show. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    }else res|=1<<4; 
    myfree(SRAMIN,p); 
    return res;
}

//NTP����ʱ��
void ntp_update(void)
{  
     sim800c_send_cmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK",200);//���ó��س���1
     sim800c_send_cmd("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK",200);
     sim800c_send_cmd("AT+SAPBR=1,1",0,200);//����һ��GPRS������
     delay_ms(5);
     sim800c_send_cmd("AT+CNTPCID=1","OK",200);//����CNTPʹ�õ�CID
     sim800c_send_cmd("AT+CNTP=\"202.120.2.101\",32","OK",200);//����NTP�������ͱ���ʱ��(32ʱ�� ʱ����׼ȷ)
     sim800c_send_cmd("AT+CNTP","+CNTP: 1",600);//ͬ������ʱ��
}

//SIM800C�����Գ���
void sim800c_test(void)
{
    printf ("sim800c_test. start sim800c test. \r\n");
    while(1)
    {
        while(sim800c_send_cmd("AT","OK",100))//����Ƿ�Ӧ��ATָ�� 
        {
            delay_ms(400);  
        }
        
        sim800c_send_cmd("ATE0","OK",200);//������
        
        sim800c_mtest_ui();//������������Ϣ
        
        sim800c_gsminfo_show();
        
        ntp_update();//����ͬ��ʱ��
        
        sim800c_gprs_test();    //GPRS����
        //sim800c_call_test();  //���Ų���
        //sim800c_sms_test();   //���Ų���
        //sim800c_spp_test();   //����spp����

    }

}


//���Ӱ����Ʒ�����
u8 connect_to_server(u8 mode,u8* ipaddr,u8* port)
{
    u8 p[64] = {0};
    u16 timex=6;
    u8 connectsta=0;            //0,????;1,????;2,????; 

    while(sim800c_send_cmd("AT","OK",100)) //����Ƿ�Ӧ��ATָ�� 
    {   
        LOGE("no response. sim800cδ��ӦATָ��.");
        delay_ms(400);
    }

    while(sim800c_send_cmd("AT+CPIN?","OK",200))
    {
        LOGE("no response. sim800cδ��⵽SIM��.");
        delay_ms(400);
    }

    sim800c_send_cmd("ATE0","OK",200);

    USART3_RX_STA=0;
    while(sim800c_send_cmd("AT+CGSN","OK",200))
    {
        LOGE("no response. sim800cδ�������к�.");
        delay_ms(400);
    }
    //p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");
    //p1[0]=0;//��������� 
    LOGI("��⵽���к�: %s", USART3_RX_BUF+2);
    USART3_RX_STA=0;

    if(sim800c_send_cmd("AT+CSQ","+CSQ:",200))//��ѯ�ź�����
    {
        LOGE("no response. sim800cδ�����ź�����\r\n");
    }
    

#if 0

    if(sim800c_send_cmd("AT+CGSN","OK",200))//��ѯSIM800C��Ʒ���к�
    {
        printf("no response. sim800cδ�������к�.\r\n");
    }
    else
    {
        p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");
        p1[0]=0;//��������� 
        printf("���к�: %s\r\n", USART3_RX_BUF+2);
        USART3_RX_STA=0;
    }

    if(sim800c_send_cmd("AT+CPIN?","OK",200))//���SIM���Ƿ�׼����
    {
        printf("no response. sim800cδ���ر�������.\r\n");
    }

    if(sim800c_send_cmd("AT+CCLK?","+CCLK:",200))
    {
        printf("no response. sim800cδ��������\r\n");
    }
#endif

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
           LOGI(" conncet return closed.");
           return 7;
       }
       else
       {
           break;
       }
    }
    return 7;
}











