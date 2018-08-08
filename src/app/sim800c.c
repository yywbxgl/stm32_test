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



//usmart支持部分
//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零USART3_RX_STA;
//     1,清零USART3_RX_STA;
void sim_at_response(u8 mode)
{
    if(USART3_RX_STA&0X8000)        //接收到一次数据了
    { 
        USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
        printf("%s",USART3_RX_BUF); //发送到串口
        if(mode)USART3_RX_STA=0;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//ATK-SIM800C 各项测试(拨号测试、短信测试、GPRS测试,蓝牙测试)共用代码

//SIM800C发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//其他,期待应答结果的位置(str的位置)
u8* sim800c_check_cmd(u8 *str)
{
    char *strx=0;
    if(USART3_RX_STA&0X8000)        //接收到一次数据了
    { 
        USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
        LOGD("get response=%s", (const char*)USART3_RX_BUF);
        strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
    }
    return (u8*)strx;
}
//SIM800C发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 sim800c_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{

    u8 res=0; 
    USART3_RX_STA=0;
    
    LOGD("send cmd=[%s]", cmd);
    
    if((u32)cmd<=0XFF)
    {
        while((USART2->SR&0X40)==0);//等待上一次数据发送完成  
        USART2->DR=(u32)cmd;
    }else u3_printf("%s\r\n",cmd);//发送命令

    if(ack&&waittime)       //需要等待应答
    {
        while(--waittime)   //等待倒计时
        {
            delay_ms(10);
            if(USART3_RX_STA&0X8000)//接收到期待的应答结果
            {
                if(sim800c_check_cmd(ack))break;//得到有效数据 
                USART3_RX_STA=0;
            } 
        }
        if(waittime==0)res=1; 
    }
    return res;
}


//SIM800C发送hex二进制数据mqtt消息
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 sim800c_send_hex(u8 *buf, u8 len, u8 *ack, u16 waittime)
{

    u8 res=0; 
    USART3_RX_STA=0;

    LOGD("send hex.");
    PrintHex(buf, len);
    u3_printf_hex(buf, len);//发送数据

    if(ack&&waittime)       //需要等待应答
    {
        while(--waittime)   //等待倒计时
        {
            delay_ms(10);
            if(USART3_RX_STA&0X8000)//接收到期待的应答结果
            {
                if(sim800c_check_cmd(ack))break;//得到有效数据 
                USART3_RX_STA=0;
            } 
        }
        if(waittime==0)res=1; 
    }
    return res;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//GPRS测试部分代码

const u8 *modetbl[2]={"TCP","UDP"};//连接模式
//tcp/udp测试
//带心跳功能,以维持连接
//mode:0:TCP测试;1,UDP测试)
//ipaddr:ip地址
//port:端口 
void sim800c_tcpudp_test(u8 mode,u8* ipaddr,u8* port)
{ 
    u8 *p,*p1,*p2,*p3;
    u16 timex=0;
    u8 connectsta=0;            //0,????;1,????;2,????; 
    u8 hbeaterrcnt=0;           //心跳错误计数器,连续5次心跳信号无应答,则重新连接
    p=mymalloc(SRAMIN,100);     //申请100字节内存
    p1=mymalloc(SRAMIN,100);    //申请100字节内存

    USART3_RX_STA=0;
    sprintf((char*)p,"AT+CIPSTART=\"%s\",\"%s\",\"%s\"",modetbl[mode],ipaddr,port);
    if(sim800c_send_cmd(p,"OK",500))return;     //发起连接
    while(1)
    { 
        if(connectsta==0&&(timex%200)==0)//连接还没建立的时候,每2秒查询一次CIPSTATUS.
        {
            sim800c_send_cmd("AT+CIPSTATUS","OK",500);  //查询连接状态
            if(strstr((const char*)USART3_RX_BUF,"CLOSED"))connectsta=2;
            if(strstr((const char*)USART3_RX_BUF,"CONNECT OK"))connectsta=1;
        }
        if(connectsta==2||hbeaterrcnt>8)//连接中断了,或者连续8次心跳没有正确发送成功,则重新连接
        {
            sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",500);   //关闭连接
            sim800c_send_cmd("AT+CIPSHUT","SHUT OK",500);       //关闭移动场景 
            //sim800c_send_cmd(p,"OK",500);                     //尝试重新连接
            connectsta=0;   
            hbeaterrcnt=0;
            break; //关闭链接，跳出测试
        }
        if(connectsta==1&&timex>=600)//连接正常的时候,每6秒发送一次心跳
        {
            timex=0;
            if(sim800c_send_cmd("AT+CIPSEND",">",200)==0)//发送数据
            {
                sim800c_send_cmd("keepalive",0,0);  //发送数据:0X00  
                delay_ms(20);                       //必须加延时
                sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,结束数据发送,启动一次传输  
            }else sim800c_send_cmd((u8*)0X1B,0,0);  //ESC,取消发送      
            hbeaterrcnt++; 
            printf("hbeaterrcnt:%d\r\n",hbeaterrcnt);//方便调试代码
        } 
        delay_ms(10);
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
        timex++; 
    } 
    myfree(SRAMIN,p);
    myfree(SRAMIN,p1);
}


//SIM800C GPRS测试
//用于测试TCP/UDP连接
//返回值:0,正常
//其他,错误代码
u8 sim800c_gprs_test(void)
{
    const u8 *port="8086";  //端口固定为8086,当你的电脑8086端口被其他程序占用的时候,请修改为其他空闲端口
    u8 mode=0;              //0,TCP连接;1,UDP连接
    u8 ipbuf[16]="124.90.130.2";//IP缓存
    printf("sim800c_gprs_test. mode=[%s] ip=[%s:%s]\r\n", (u8*)modetbl[mode], ipbuf, (u8*)port);
    sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",100);   //关闭连接
    sim800c_send_cmd("AT+CIPSHUT","SHUT OK",100);       //关闭移动场景 
    if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",1000))return 1;             //设置GPRS移动台类别为B,支持包交换和数据交换 
    if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",1000))return 2;//设置PDP上下文,互联网接协议,接入点等信息
    if(sim800c_send_cmd("AT+CGATT=1","OK",500))return 3;                    //附着GPRS业务
    if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500))return 4;        //设置为GPRS连接模式
    if(sim800c_send_cmd("AT+CIPHEAD=1","OK",500))return 5;                  //设置接收数据显示IP头(方便判断数据来源)   

    u3_printf("AT+CLDTMF=2,\"%s\"\r\n", ipbuf); 

    sim800c_tcpudp_test(mode, ipbuf, (u8*)port);

    return 0;
} 


//测试界面主UI
void sim800c_mtest_ui()
{
    u8 *p, *p1, *p2;
    p=mymalloc(SRAMIN,50);//申请50个字节的内存
    USART3_RX_STA=0;
    if(sim800c_send_cmd("AT+CGMI","OK",200)==0)//查询制造商名称
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");
        p1[0]=0;//加入结束符
        sprintf((char*)p,"制造商:%s",USART3_RX_BUF+2);
        printf("sim800c_mtest_ui. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    } 
    if(sim800c_send_cmd("AT+CGMM","OK",200)==0)//查询模块名字
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n"); 
        p1[0]=0;//加入结束符
        sprintf((char*)p,"模块型号:%s",USART3_RX_BUF+2);
        printf("sim800c_mtest_ui. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    } 
    if(sim800c_send_cmd("AT+CGSN","OK",200)==0)//查询产品序列号
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");//查找回车
        p1[0]=0;//加入结束符 
        sprintf((char*)p,"序列号:%s",USART3_RX_BUF+2);
        printf("sim800c_mtest_ui. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    }
    if(sim800c_send_cmd("AT+CNUM","+CNUM",200)==0)//查询本机号码
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF),",");
        p2=(u8*)strstr((const char*)(p1+2),"\"");
        p2[0]=0;//加入结束符
        sprintf((char*)p,"本机号码:%s",p1+2);
        printf("sim800c_mtest_ui. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    }
    myfree(SRAMIN,p); 
}

//GSM信息显示(信号质量,电池电量,日期时间)
//返回值:0,正常
//其他,错误代码
u8 sim800c_gsminfo_show()
{
    u8 *p,*p1,*p2;
    u8 res=0;
    p=mymalloc(SRAMIN,50);//申请50个字节的内存
    USART3_RX_STA=0;
    if(sim800c_send_cmd("AT+CPIN?","OK",200))res|=1<<0; //查询SIM卡是否在位 
    USART3_RX_STA=0;  
    if(sim800c_send_cmd("AT+COPS?","OK",200)==0)        //查询运营商名字
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF),"\""); 
        if(p1)//有有效数据
        {
            p2=(u8*)strstr((const char*)(p1+1),"\"");
            p2[0]=0;//加入结束符           
            sprintf((char*)p,"运营商:%s",p1+1);
            printf("sim800c_gsminfo_show. %s\r\n", (char*)p);
        } 
        USART3_RX_STA=0;        
    }else res|=1<<1;
    if(sim800c_send_cmd("AT+CSQ","+CSQ:",200)==0)       //查询信号质量
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF),":");
        p2=(u8*)strstr((const char*)(p1),",");
        p2[0]=0;//加入结束符
        sprintf((char*)p,"信号质量:%s",p1+2);
        printf("sim800c_gsminfo_show. %s\r\n", (char*)p);
        USART3_RX_STA=0;
    }else res|=1<<2;
    if(sim800c_send_cmd("AT+CBC","+CBC:",200)==0)       //查询电池电量
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF),",");
        p2=(u8*)strstr((const char*)(p1+1),",");
        p2[0]=0;p2[5]=0;//加入结束符
        sprintf((char*)p,"电池电量:%s%%  %smV",p1+1,p2+1);
        printf("sim800c_gsminfo_show. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    }else res|=1<<3; 
    if(sim800c_send_cmd("AT+CCLK?","+CCLK:",200)==0)        //查询电池电量
    { 
        p1=(u8*)strstr((const char*)(USART3_RX_BUF),"\"");
        p2=(u8*)strstr((const char*)(p1+1),":");
        p2[3]=0;//加入结束符
        sprintf((char*)p,"日期时间:%s",p1+1);
        printf("sim800c_gsminfo_show. %s\r\n", (char*)p);
        USART3_RX_STA=0;        
    }else res|=1<<4; 
    myfree(SRAMIN,p); 
    return res;
}

//NTP更新时间
void ntp_update(void)
{  
     sim800c_send_cmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK",200);//配置承载场景1
     sim800c_send_cmd("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK",200);
     sim800c_send_cmd("AT+SAPBR=1,1",0,200);//激活一个GPRS上下文
     delay_ms(5);
     sim800c_send_cmd("AT+CNTPCID=1","OK",200);//设置CNTP使用的CID
     sim800c_send_cmd("AT+CNTP=\"202.120.2.101\",32","OK",200);//设置NTP服务器和本地时区(32时区 时间最准确)
     sim800c_send_cmd("AT+CNTP","+CNTP: 1",600);//同步网络时间
}

//SIM800C主测试程序
void sim800c_test(void)
{
    printf ("sim800c_test. start sim800c test. \r\n");
    while(1)
    {
        while(sim800c_send_cmd("AT","OK",100))//检测是否应答AT指令 
        {
            delay_ms(400);  
        }
        
        sim800c_send_cmd("ATE0","OK",200);//不回显
        
        sim800c_mtest_ui();//测试主界面信息
        
        sim800c_gsminfo_show();
        
        ntp_update();//网络同步时间
        
        sim800c_gprs_test();    //GPRS测试
        //sim800c_call_test();  //拨号测试
        //sim800c_sms_test();   //短信测试
        //sim800c_spp_test();   //蓝牙spp测试

    }

}


//连接阿里云服务器
u8 connect_to_server(u8 mode,u8* ipaddr,u8* port)
{
    u8 p[64] = {0};
    u16 timex=6;
    u8 connectsta=0;            //0,????;1,????;2,????; 

    while(sim800c_send_cmd("AT","OK",100)) //检测是否应答AT指令 
    {   
        LOGE("no response. sim800c未回应AT指令.");
        delay_ms(400);
    }

    while(sim800c_send_cmd("AT+CPIN?","OK",200))
    {
        LOGE("no response. sim800c未检测到SIM卡.");
        delay_ms(400);
    }

    sim800c_send_cmd("ATE0","OK",200);

    USART3_RX_STA=0;
    while(sim800c_send_cmd("AT+CGSN","OK",200))
    {
        LOGE("no response. sim800c未返回序列号.");
        delay_ms(400);
    }
    //p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");
    //p1[0]=0;//加入结束符 
    LOGI("检测到序列号: %s", USART3_RX_BUF+2);
    USART3_RX_STA=0;

    if(sim800c_send_cmd("AT+CSQ","+CSQ:",200))//查询信号质量
    {
        LOGE("no response. sim800c未返回信号质量\r\n");
    }
    

#if 0

    if(sim800c_send_cmd("AT+CGSN","OK",200))//查询SIM800C产品序列号
    {
        printf("no response. sim800c未返回序列号.\r\n");
    }
    else
    {
        p1=(u8*)strstr((const char*)(USART3_RX_BUF+2),"\r\n");
        p1[0]=0;//加入结束符 
        printf("序列号: %s\r\n", USART3_RX_BUF+2);
        USART3_RX_STA=0;
    }

    if(sim800c_send_cmd("AT+CPIN?","OK",200))//检查SIM卡是否准备好
    {
        printf("no response. sim800c未返回本机号码.\r\n");
    }

    if(sim800c_send_cmd("AT+CCLK?","+CCLK:",200))
    {
        printf("no response. sim800c未返回日期\r\n");
    }
#endif

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











