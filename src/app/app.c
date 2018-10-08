#include "delay.h"
#include "timer.h"
#include "usart.h"
#include "usmart.h"
#include "app.h"
#include "utils.h"
#include "rtc.h"
#include "fm1702.h" 
#include "logging.h"
#include "mqtt_app.h"
#include "sim800c.h"
#include "usart3.h"
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "message.h"
#include "MQTTPacket.h"
#include "MQTTPublish.h"
#include "digitron.h"
#include "test.h"
#include "dcf.h"
#include "digitron.h"
#include <jansson.h>
#include <string.h>


u8 mqtt_msg[400]={0}; //mqtt消息包
u8 send_cmd[20]= {0};



void main_loop(void)
{
    u16 time_t=0;
    while(1)
    {
        if(g_state == INIT){
            memset(g_Digitron, INIT, sizeof(g_Digitron)); //打开数码数码管显示当前设备状态
            setOnFlag();
            if (connect_to_server() == TRUE){
                char device_auth[24] = "868926036902861";
                char  device_auth_2[24] = "869627039319199";
                if (strncmp(g_device_code, device_auth, strlen(device_auth)) == 0)
                    g_state = TCP_OK;
                else if (strncmp(g_device_code, device_auth_2, strlen(device_auth_2)) == 0)
                    g_state = TCP_OK;
                else{
                    LOGE("设备未授权. 当前设备序列号为：%s", g_device_code);
                    g_state = NOT_AUTH;
                }
            }
        }
        else if (g_state == NOT_AUTH){
            memset(g_Digitron, NOT_AUTH, sizeof(g_Digitron)); //打开数码数码管显示当前设备状态
            setOnFlag();
        }
        else if(g_state == TCP_OK){
            memset(g_Digitron, TCP_OK, sizeof(g_Digitron)); //打开数码数码管显示当前设备状态
            setOnFlag();
            if (subscribe_mqtt() == TRUE){
                g_state = MQTT_OK;
            }else if (subscribe_mqtt() == FALSE){
                g_state = INIT; 
            }
        }
        else if(g_state == MQTT_OK){
            memset(g_Digitron, MQTT_OK, sizeof(g_Digitron)); //打开数码数码管显示当前设备状态
            setOnFlag();
            USART3_RX_STA=0;
            if (send_keep_alive_mesaage() == TRUE){
                g_state = WAIT_IC;
            }
            else{
                g_state = TCP_OK;
            }
        }
        else if(g_state == WAIT_IC){
            setOffFlag(); //关闭数码数码管，等待IC卡、
            
            //每隔5秒检测一次网络状况
            if (time_t % (5*100) == 0)
            {
                //网络断开重新连接
                if(sim800c_tcp_check() == FALSE){
                    g_state = INIT;
                    continue;
                }
            }

            //处理服务器消息
            s8 trade =  0;
            if (recv_mqtt_message(&trade) == TRUE)
            { 
                if (trade == 1)
                {
                    deal_keep_alive_mesaage_response();//处理保活信令
                }
                else if (trade == 6)
                {
                    //处理扫描消费信令
                    if(deal_app_cousume_command(1) == TRUE)
                    {
                        g_ICCard_Value = g_maxMoney;
                        display(g_ICCard_Value);
                        g_state = APP_CONSUME;
                        g_consume_time = 0;  //开始计费
                        DCF_Set();           //打开电磁阀
                        continue; 
                    }
                }
                else if (trade == 0)
                {
                    send_keep_alive_mesaage();
                }
            }

            //等待IC卡
            if (time_t % 20 == 0){
                if (scan_for_card() == TRUE){
                    g_state = ON_IC;
                }
            }

            //每隔g_heart发送一个心跳
            if (time_t % (g_heart*100) == 0)
            {
                if (send_keep_alive_mesaage() == FALSE){
                    g_state = TCP_OK;
                }
            }
        }
        else if(g_state == ON_IC){
            memset(g_Digitron, ON_IC, sizeof(g_Digitron));
            setOnFlag(); //打开数码数码管显示当前设备状态
            if(send_start_consume_mesaage() == TRUE){
                //发送开始消费请求，等待响应
                LOGI("开始刷卡消费！");
                g_state = IC_CONSUME;
                g_consume_time = 0;  //开始计费
                DCF_Set();           //打开电磁阀
            }else{
                //等待卡片移走置位状态
                while(read_card() == TRUE)
                {
                    delay_ms(100);
                }
                g_state = WAIT_IC;
            }
        }
        else if(g_state == IC_CONSUME){
            setOnFlag();  //打开数码管，显示卡内余额

            //每隔5秒检测一次网络状况
            if (time_t % (5*100) == 0)
            {
                //网络断开??
                if(sim800c_tcp_check() == FALSE){
                    ;
                    //g_state = INIT;
                    //continue;
                }
            }
            
            if (card_runing() == FALSE)
            {
               //结束前发送一个结束信令
               send_consume_mesaage(1, 1);
               g_state = WAIT_IC;
               DCF_Reset();         //关闭电磁阀
               g_consume_time = 0;  //结束计费
            }

            if (g_ICCard_Value == 0)
            {
                LOGI("当前余额不足");
                //结束前发送一个结束信令
                send_consume_mesaage(1, 1);
                g_state = WAIT_IC;
                DCF_Reset();         //关闭电磁阀
                g_consume_time = 0;  //结束计费
                //等待卡片移走置位状态
                display(g_ICCard_Value);
                while(read_card() == TRUE)
                {
                    delay_ms(100);
                }
                g_state = WAIT_IC;
            }

            //每隔g_logRate发送一消费信息
            if(time_t % (g_logRate*100) == 0)
            {
               send_consume_mesaage(1, 2);
            }

            s8 trade =  -1;
            if (recv_mqtt_message(&trade) == TRUE)
            {
                //不允许app消费
                if(trade == 6)
                {
                    deal_app_cousume_command(-1);
                }
            }

        }
        else if(g_state == APP_CONSUME){
            setOnFlag();  //打开数码管，显示卡内余额

            //每隔5秒检测一次网络状况
            if (time_t % (5*100) == 0)
            {
                //网络断开??
                if(sim800c_tcp_check() == FALSE){
                    ;
                    //g_state = INIT;
                    //continue;
                }
            }

            //每隔g_logRate发送一消费信息
            if(time_t % (g_logRate*100) == 0)
            {
                send_consume_mesaage(2, 2);
            }


            if (g_ICCard_Value == 0)
            {
                LOGI("当前余额为0");
                send_consume_mesaage(2, 1);//关闭前发送结束消费信令
                g_state = WAIT_IC;
                DCF_Reset();         //关闭电磁阀
                g_consume_time = 0;  //结束计费
                display(g_ICCard_Value);
            }

            //处理结束服务端指令8，结束消费信令
            s8 trade = -1;
            if (recv_mqtt_message(&trade) == TRUE)
            {
                if(trade == 8 && deal_command_app_finish() == TRUE)
                {
                    send_consume_mesaage(2, 1);
                    g_state = WAIT_IC;
                    DCF_Reset();         //关闭电磁阀
                    g_consume_time = 0;  //结束计费
                }

                //不允许重复消费
                if(trade == 6)
                {
                    deal_app_cousume_command(-1);
                }
            }
            
        }
        

        delay_ms(10);
        time_t++;
    }

}


u8 scan_for_card(void)
{
    u8 status;          //读写卡状态返回
    u8 buf[16]= {0};    //读写卡缓冲buff
    u8 DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  //todo:使用g_pwd转换
    status = Request(RF_CMD_REQUEST_ALL);       //寻卡
    if(status != FM1702_OK){
        //LOGD("未检测到卡片");
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
    //LOGI("读取到卡片唯一号：%s", g_card_id);

    status=MIF_READ(buf,28);             //读卡，读取7扇区0块数据到buffer[0]-buffer[15]
    if(status != FM1702_OK)
    {
        LOGE("读卡数据失败");
        return FALSE;
    }
    else
    {
        int i=0;
        g_ICCard_Value = 0;
        for(; i<4; ++i){
            g_ICCard_Value = g_ICCard_Value << 8;
            g_ICCard_Value = g_ICCard_Value | buf[i];
        }

        LOGI("扇区读取：");
        PrintHex(buf, sizeof(buf));
        i = 0;
        for(; i< 8; ++i){
            g_serverCardNo[i]= buf[i+4];
        }
        //memcpy(g_serverCardNo, buf[4] , sizeof(g_serverCardNo));//该接口有问题
        LOGI("读卡数据成功，余额= %ld", g_ICCard_Value);
        LOGI("读卡数据成功，服务ID= %s", g_serverCardNo);
    }

    return TRUE;
}


u8 card_runing(void)
{
    u8 status;          //读写卡状态返回
    u8 buf[16]= {0};    //读写卡缓冲buff
    u32 now_card_value = 0 ; //当前卡片读取的余额

    status=MIF_READ(buf,28); //读卡，读取7扇区0块数据到buffer[0]-buffer[15]
    if(status != FM1702_OK)
    {
        LOGE("读卡数据失败");
        g_state = WAIT_IC; //读卡失败，卡片移走，设备状态置位
        return FALSE;
    }
    else
    {
        now_card_value = 0; //读取卡内余额
        int  i = 0;
        for(; i<4; ++i){
            now_card_value = now_card_value << 8;
            now_card_value = now_card_value | buf[i];
        }
    }


    //当卡片上的余额与实际余额不匹配时候，写入卡片数据
    if(now_card_value != g_ICCard_Value)
    {
        //LOGI("读卡数据成功，余额= %ld", now_card_value);
        LOGD("卡上余额=%d， 当前实际余额=%d, 写入卡片.", now_card_value, g_ICCard_Value);

        int i =3;
        u32 temp = g_ICCard_Value ;
        for (; i>=0; --i){
            buf[i] = temp & 0xff;
            temp = temp >> 8;
        }

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


u8 read_card(void)
{
    u8 status;          //读写卡状态返回
    u8 buf[16]= {0};    //读写卡缓冲buff

    //LOGD("卡片检测---");
    status=MIF_READ(buf,28); //读卡，读取7扇区0块数据到buffer[0]-buffer[15]
    if(status != FM1702_OK)
    {
        LOGI("卡片被移走");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


u8 ic_wrtie_server_id(void)
{
    u8 status;          //读写卡状态返回
    u8 buf[16]= {0};    //读写卡缓冲buff

    status=MIF_READ(buf,28); //读卡，读取7扇区0块数据到buffer[0]-buffer[15]
    if(status != FM1702_OK)
    {
        LOGE("读卡数据失败");
        g_state = WAIT_IC; //读卡失败，卡片移走，设备状态置位
        return FALSE;
    }

    //写入卡片服务ID数据
    u8 i =0;
    for(; i<8; ++i){
        buf[i+4]= g_serverCardNo[i];
    }
    status=MIF_Write(buf,28);  //写卡，将buffer[0]-buffer[15]写入1扇区0块
    if(status != FM1702_OK)
    {
        LOGE("写服务器绑定ID失败");
        g_state = WAIT_IC;     //写卡失败，卡片移走，设备状态置位
        return FALSE;
    }
    else{
        LOGI("写服务器绑定ID成功，%s", g_serverCardNo);
    }

    status=MIF_READ(buf,28); //读卡，读取7扇区0块数据到buffer[0]-buffer[15]
    if(status != FM1702_OK)
    {
        LOGE("读卡数据失败");
        g_state = WAIT_IC; //读卡失败，卡片移走，设备状态置位
        return FALSE;
    }
    else
    {
         LOGD("当前服务器ID=%s", (char*)buf[4]);
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
    //组装ClientID
    strncpy(g_clent_id + strlen(g_clent_id), (char*)g_device_code, sizeof(g_device_code));
    LOGD("客户端ID:%s", g_clent_id);

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
    len=mqtt_connect_message(mqtt_msg, g_clent_id , USRNAME, PASSWD);//id,用户名和密码
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
    USART3_RX_STA = 0;
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据成功
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                      //必须加延时
        //sim800c_send_cmd((u8*)0X1A,0,0);   //CTRL+Z,结束数据发送,启动一次传输
        LOGI("MQTT握手成功.");
    }else if(sim800c_send_cmd(send_cmd,"ERROR",200)==0){
        LOGE("MQTT握手失败.");
        return FALSE;
    }


    len=mqtt_subscribe_message(mqtt_msg,TOPIC_SUB,1,1);//订阅test主题
    //printf("send len = %d\r\n", len);
    //LOGI("mqtt_subscribe... \r\n");
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(500);                 //必须加延时
        //sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,结束数据发送,启动一次传输
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
    LOGD("发送MQTT保活信令.");
    create_keep_alive_message(msg, sizeof(msg));
    //PrintHex(msg, strlen(msg));
    //LOGD("message:%s", msg);

    MQTTString top = MQTTString_initializer;
    top.cstring = TOPIC_PUB;
    len = MQTTSerialize_publish((unsigned char*)mqtt_msg, sizeof(mqtt_msg), 0 ,0, 0, 0, top, msg, strlen(msg));
    //PrintHex(mqtt_msg,len);

    //如果大于254个字节，需要分多次发送,每次发送200个字节
    u8 t = 0 ;  //记录已经发送的次数
    while(len > 254){
        sprintf((char*)send_cmd, "AT+CIPSEND=%d", 200);//接收到的字节数
        if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
        {
            u3_printf_hex(mqtt_msg + t*200, 200);
            delay_ms(200);                //必须加延时
            //USART3_RX_STA = 0;
        }
        t++;
        len = len - 200;
    }

    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//接收到的字节数
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
    {
        u3_printf_hex(mqtt_msg + t*200, len);
        delay_ms(200);                  //必须加延时
        //USART3_RX_STA=0;
    }else if (sim800c_send_cmd(send_cmd,"ERROR",200)==0){
        //发送失败，连接可能断开
        LOGE("发送保活信令失败.");
        return FALSE;
    }

    return TRUE;
}


u8 deal_keep_alive_mesaage_response(void)
{
    return parse_keep_alive_response(mqtt_msg, sizeof(mqtt_msg));
}


u8 recv_mqtt_message(s8* trade)
{
    *trade = -1;
    //接收到一次数据了
    if(USART3_RX_STA&0X8000)
    {
       //LOGD("收到了数据[%x]=%s", USART3_RX_STA, USART3_RX_BUF);
       LOGD("收到了数据[%x]", USART3_RX_STA);
       //PrintHex(USART3_RX_BUF, 100);
       //收到订阅消息
       if (USART3_RX_BUF[0] == 0x32){
           USART3_RX_BUF[USART3_RX_STA&0x7fff] = 0;
           unsigned char dup;
           int qos;
           unsigned char retained;
           unsigned short msgid;
           int payloadlen_in;
           unsigned char* payload_in;
           MQTTString receivedTopic;
           
           MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
                               &payload_in, &payloadlen_in, USART3_RX_BUF, sizeof(USART3_RX_BUF));
           strncpy(mqtt_msg, payload_in, payloadlen_in);
           mqtt_msg[payloadlen_in] = 0;
           //LOGI("收到服务器消息1[msgId:%d]=%s\n",  msgid, payload_in);
           LOGI("收到服务器消息=%s",   mqtt_msg);
           //发送ACK包
           u8 ack[4] ={0x40, 0x02, (0xff00&msgid)>>8, 0xff&msgid};
           if(sim800c_send_cmd("AT+CIPSEND=4",">",200)==0){
               //LOGD("返回ACK包");
               u3_printf_hex(ack, sizeof(ack));
               delay_ms(200);
           }

           *trade = parse_service_message_common(mqtt_msg, sizeof(mqtt_msg));
           return TRUE;
       }
       else{
            //不是服务器消息，清除接受缓冲
            USART3_RX_STA = 0;
       }

       #if 0
       p2=strstr((char*)USART3_RX_BUF,"+IPD");
       if(p2)//接收到TCP/UDP数据
       {
           int recv_len = 0;
           p2 = strstr((char*)USART3_RX_BUF,",");
           p3 = strstr((char*)USART3_RX_BUF,":");
           *p3 = 0;//加入结束符
           recv_len = atoi(p2+1);
           *(p3 + 1 + recv_len) = 0; //只显示接收到的数据
           LOGD("收到%d个字节，内容如下:%s", recv_len, p3+1);
           PrintHex((u8*)p3+1, recv_len);
       }
       #endif

       USART3_RX_STA=0;
       return FALSE;
    }

    return FALSE;
}





u8 send_start_consume_mesaage(void)
{

    u16 len;
    u8 msg[200]={0}; //信令内容

    create_start_consume_message(msg, sizeof(msg));
    LOGD("发送开始消费信令:%s.", msg);

    MQTTString top = MQTTString_initializer;
    top.cstring = TOPIC_PUB;
    len = MQTTSerialize_publish((unsigned char*)mqtt_msg, sizeof(mqtt_msg), 0 ,0, 0, 0, top, msg, strlen(msg));
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//要发送的数据长度
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
    {
       u3_printf_hex(mqtt_msg, len);
       delay_ms(500);                  //必须加延时
       //USART3_RX_STA=0;
    }else if (sim800c_send_cmd(send_cmd,"ERROR",200)==0){
       //发送失败，连接可能断开
       LOGE("发送开始消费信令失败.");
       return FALSE;
    }


    //等待消息回复，超时10秒
    u16 t=500;
    while(t--)
    {
        s8 trade = 0;
        if (recv_mqtt_message(&trade) == TRUE)
        {
            if(2 == trade)
            {
                if(parse_start_consume_response(mqtt_msg, sizeof(mqtt_msg)) == TRUE)
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
        }
        delay_ms(10);
    }

    return FALSE;
}


//发送扣费信息
//ic_flag标志 1-IC卡消费， 2-app消费
//finish_flag标志 1-结束消费， 2-正在消费
u8 send_consume_mesaage(u8 ic_flag, u8 finish_flag)
{

    u16 len;
    u8 msg[200]={0}; //信令内容
    
    create_consume_message(msg, sizeof(msg), ic_flag, finish_flag);
    LOGD("发送扣费信息:%s", msg);

    MQTTString top = MQTTString_initializer;
    top.cstring = TOPIC_PUB;
    len = MQTTSerialize_publish((unsigned char*)mqtt_msg, sizeof(mqtt_msg), 0 ,0, 0, 0, top, msg, strlen(msg));
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//要发送的数据长度
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(500);                  //必须加延时
        //USART3_RX_STA=0;
        return TRUE;
    }else if (sim800c_send_cmd(send_cmd,"ERROR",200)==0){
        //发送失败，连接可能断开
        LOGE("发送扣费信息失败.");
        return FALSE;
    }

    return FALSE;
}



u8 deal_app_cousume_command(s8 ok_flag)
{
    char temp_orderNo[12] = {0};
    if (1 == ok_flag)
    {
        if (parse_start_app_consume_message(mqtt_msg, sizeof(mqtt_msg)) == FALSE)
            return FALSE;
    }
    else if (-1 == ok_flag)
    {
        json_t* message = NULL;
        json_t* data = NULL;
        json_error_t error;
        message = json_loads((char*)mqtt_msg, 0, &error);
        if(!message){
            LOGE("Json 数据格式错误[message]");
            json_decref(message);
            return FALSE;
        }

        data = json_object_get(message, M_DATA);
        if (!data || !json_is_object(data)){
            LOGE("Json 数据格式错误[data]");
            json_decref(message);
            return FALSE;
        }

        json_t* ordNo = json_object_get(data, M_orderNo);
        if(!ordNo || !json_is_string(ordNo)){
            LOGE("Json 数据格式错误[M_orderNo]");
            json_decref(message);
            return FALSE;
        }else{
            LOGI("当前app消费订单:%s", json_string_value(ordNo));
            strncpy(temp_orderNo, json_string_value(ordNo), sizeof(temp_orderNo));
        }

        json_decref(message);
        LOGE("当前设备正在消费，不允许再次消费.");
    }

    u16 len;
    u8 msg[200]={0}; //信令内容
    create_start_app_consume_response(msg, sizeof(msg), ok_flag, temp_orderNo);//返回成功
    LOGD("发送app消费请求响应:%s", msg);

    MQTTString top = MQTTString_initializer;
    top.cstring = TOPIC_PUB;
    len = MQTTSerialize_publish((unsigned char*)mqtt_msg, sizeof(mqtt_msg), 0 ,0, 0, 0, top, msg, strlen(msg));
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//要发送的数据长度
    if(sim800c_send_cmd(send_cmd,">",200)==0)//发送数据
    {
       u3_printf_hex(mqtt_msg, len);
       delay_ms(500);//必须加延时

       return TRUE;
    }else {
       //发送失败，连接可能断开
       LOGE("发送app消费请求响应失败.");
       return FALSE;
    }

}



//处理服务端指令8，app消费结束命令
u8 deal_command_app_finish(void)
{
    return parse_finish_app_consume_message(mqtt_msg, sizeof(mqtt_msg));
}


