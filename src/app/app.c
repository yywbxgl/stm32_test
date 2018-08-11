#include "delay.h"
#include "timer.h"
#include "usart.h"
#include "app.h"
#include "utils.h"
#include "fm1702.h" 
#include "logging.h"
#include "mqtt_app.h"
#include "sim800c.h"
#include "usart3.h"
#include <string.h>
#include "utils.h"
#include "message.h"
#include <jansson.h>




u8 mqtt_msg[400]={0}; //mqtt消息包
u8 send_cmd[20]= {0};


u8 scan_for_card(void)
{
    u8 status;          //读写卡状态返回
    u8 buf[16]= {0};    //读写卡缓冲buff
    u8 DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  //todo:使用g_pwd转换
    status = Request(RF_CMD_REQUEST_ALL);       //寻卡
    if(status != FM1702_OK){
        LOGD("未检测到卡片");
        return FALSE;
    }

    status = AntiColl();                        //冲突检测
    if(status != FM1702_OK){
        LOGE("卡片冲突");
        return FALSE;
    }

    status=Select_Card();                       //选卡
    if(status != FM1702_OK){
        LOGE("选择卡片失败");
        return FALSE;
    }

    status = Load_keyE2_CPY(DefaultKey);          //加载密码
    if(status != TRUE){
        LOGE("加载密码失败");
        return FALSE;
    }

    status = Authentication(UID, 7, RF_CMD_AUTH_LA);//验证1扇区keyA
    if(status != FM1702_OK){
        LOGE("验证扇区失败");
        return FALSE;
    }

    sprintf(g_card_id, "%02x%02x%02x%02x", UID[0], UID[1], UID[2], UID[3]);
    LOGI("读取到卡片唯一号：%s", g_card_id);

    status=MIF_READ(buf,28);             //读卡，读取7扇区0块数据到buffer[0]-buffer[15]
    if(status != FM1702_OK)
    {
        LOGE("读卡数据失败");
        return FALSE;
    }
    else
    {
        g_ICCard_Value = (buf[0]<<8)|buf[1];  //读取卡内余额
        LOGI("读卡数据成功，余额= %d", g_ICCard_Value);
    }

    return TRUE;
}


u8 card_runing(void)
{
    u8 status;          //读写卡状态返回
    u8 buf[16]= {0};    //读写卡缓冲buff
    u16 now_card_value = 0 ; //当前卡片读取的余额

    status=MIF_READ(buf,28); //读卡，读取7扇区0块数据到buffer[0]-buffer[15]
    if(status != FM1702_OK)
    {
        LOGE("读卡数据失败");
        g_state = WAIT_IC; //读卡失败，卡片移走，设备状态置位
        return FALSE;
    }
    else
    {
        now_card_value = (buf[0]<<8)|buf[1];  //读取卡内余额
    }


    //当卡片上的余额与实际余额不匹配时候，写入卡片数据
    if(now_card_value != g_ICCard_Value)
    {
        LOGI("读卡数据成功，余额= %d", now_card_value);
        buf[0]=g_ICCard_Value>>8;
        buf[1]=g_ICCard_Value&0x00ff;	
        status=MIF_Write(buf,28);  //写卡，将buffer[0]-buffer[15]写入1扇区0块
        if(status != FM1702_OK)
        {
            LOGE("写数据失败");
            g_state = WAIT_IC;     //写卡失败，卡片移走，设备状态置位
            return FALSE;
        }
    }

    return TRUE;
}


u8 connect_to_server(void)
{
    u8 p[64] = {0};
    if(sim800c_send_cmd("AT","OK",100)) //检测是否应答AT指令 
    {   
        LOGE("sim800c未回应AT指令.");
        return FALSE;
    }

    if(sim800c_send_cmd("AT+CPIN?","OK",200))
    {
        LOGE("sim800c未检测到SIM卡.");
        return FALSE;
    }

    sim800c_send_cmd("ATE0","OK",200);

    //检测序列号
    USART3_RX_STA=0;
    if(sim800c_send_cmd("AT+CGSN","OK",200))
    {
        LOGE("sim800c未返回序列号.");
        return FALSE;
    }
    char* temp = strstr( (char*)USART3_RX_BUF+2, "\r\n");
    temp[0] = 0;
    strncpy(g_device_code, (char*)USART3_RX_BUF+2, sizeof(g_device_code));
    LOGI("检测到序列号: %s", g_device_code); //保存sim卡序列号作为设备序列号
    USART3_RX_STA=0;

    //检测信号强度
    if(sim800c_send_cmd("AT+CSQ","+CSQ:",200))//查询信号质量
    {
        LOGE("sim800c未返回信号质量");
        return FALSE;
    }
    else
    {
        char *end;
        end=strstr((const char*)(USART3_RX_BUF+2),"\r\n");
        *end= 0;//加入结束符 
        LOGI("sim800c返回信号质量:%s", USART3_RX_BUF+2);
        USART3_RX_STA=0;
    }


#if 0
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
    if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",1000)) 
        return FALSE;             //设置GPRS移动台类别为B,支持包交换和数据交换 
    if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",1000))
        return FALSE;//设置PDP上下文,互联网接协议,接入点等信息
    sim800c_send_cmd("AT+CGATT=1","OK",1000); //附着GPRS业务
        //return FALSE;                    
    if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500))
        return FALSE;        //设置为GPRS连接模式
        
    USART3_RX_STA=0;
    sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",HOST_IP,HOST_PORT);
    if(sim800c_send_cmd(p,"OK",500))
        return FALSE;     //发起连接

    u16 t=6;
    while(t--)
    {
        USART3_RX_STA=0;
        sim800c_send_cmd("AT+CIPSTATUS","OK",500);  //查询连接状态
        if(strstr((const char*)USART3_RX_BUF,"CONNECT OK"))
        {
            LOGI("TCP连接建立成功.");
            return TRUE;
        }
        else if(strstr((const char*)USART3_RX_BUF,"CLOSED"))
        {
            LOGI("TCP连接建立失败.");
            return FALSE;
        }
        delay_ms(1000);
    }

    return FALSE;
}



u8 subscribe_mqtt(void)
{
    //与mqtt服务器建立握手
    u16 len;
    //发起mqtt_connect请求
    len=mqtt_connect_message(mqtt_msg, CLIENTID , USRNAME, PASSWD);//id,用户名和密码
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
    USART3_RX_STA = 0;
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据成功
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                      //必须加延时
        //sim800c_send_cmd((u8*)0X1A,0,0);   //CTRL+Z,结束数据发送,启动一次传输
        delay_ms(1000);                      //必须加延时
        LOGI("MQTT握手成功.");
    }else if(sim800c_send_cmd(send_cmd,"ERROR",200)==0){
        LOGE("MQTT握手失败.");
        return FALSE;
    }


    len=mqtt_subscribe_message(mqtt_msg,TOPIC_SUB,1,1);//订阅test主题
    //printf("send len = %d\r\n", len);
    LOGI("mqtt_subscribe... \r\n");
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                 //必须加延时
        //sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,结束数据发送,启动一次传输
        delay_ms(1000);               //必须加延时
        LOGI("MQTT订阅主题成功.");
    }
    else if(sim800c_send_cmd(send_cmd,"ERROR",200)==0){
        LOGE("MQTT订阅主题失败.");
        return FALSE;
    }

    return TRUE;
}



u8 send_keep_alive_mesaage(void)
{
    u16 len;
    u8 msg[300]={0}; //信令内容
    create_keep_alive_message(msg, sizeof(msg));
    PrintHex(msg, strlen(msg));
    LOGD("message:%s", msg);
    len = mqtt_publish_message(mqtt_msg, TOPIC_PUB, (char*)msg, 2);
    PrintHex(mqtt_msg,len);
    LOGD("send len=%d", len);


    //如果大于254个字节，需要分多次发送
    //每次发送200个字节
    u8 t = 0 ;  //记录已经发送的次数

    while(len > 254){
        sprintf((char*)send_cmd, "AT+CIPSEND=%d", 200);//接收到的字节数
        if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
        {
            LOGD("发送MQTT保活信令.");
            u3_printf_hex(mqtt_msg + t*200, 200);
            delay_ms(200);                //必须加延时
            USART3_RX_STA = 0;
        }
        t++;
        len = len - 200;
    }


    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
    {
        LOGD("发送MQTT保活信令.");
        u3_printf_hex(mqtt_msg + t*200, len);
        delay_ms(1000);                 //必须加延时
        delay_ms(1000);                //必须加延时
        USART3_RX_STA = 0;
    }else if (sim800c_send_cmd(send_cmd,"ERROR",200)==0){
        //发送失败，连接可能断开
        LOGE("发送保活信令失败.");
        return FALSE;
    }

    return TRUE;
}

