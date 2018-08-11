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




u8 mqtt_msg[400]={0}; //mqtt��Ϣ��
u8 send_cmd[20]= {0};


u8 scan_for_card(void)
{
    u8 status;          //��д��״̬����
    u8 buf[16]= {0};    //��д������buff
    u8 DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  //todo:ʹ��g_pwdת��
    status = Request(RF_CMD_REQUEST_ALL);       //Ѱ��
    if(status != FM1702_OK){
        LOGD("δ��⵽��Ƭ");
        return FALSE;
    }

    status = AntiColl();                        //��ͻ���
    if(status != FM1702_OK){
        LOGE("��Ƭ��ͻ");
        return FALSE;
    }

    status=Select_Card();                       //ѡ��
    if(status != FM1702_OK){
        LOGE("ѡ��Ƭʧ��");
        return FALSE;
    }

    status = Load_keyE2_CPY(DefaultKey);          //��������
    if(status != TRUE){
        LOGE("��������ʧ��");
        return FALSE;
    }

    status = Authentication(UID, 7, RF_CMD_AUTH_LA);//��֤1����keyA
    if(status != FM1702_OK){
        LOGE("��֤����ʧ��");
        return FALSE;
    }

    sprintf(g_card_id, "%02x%02x%02x%02x", UID[0], UID[1], UID[2], UID[3]);
    LOGI("��ȡ����ƬΨһ�ţ�%s", g_card_id);

    status=MIF_READ(buf,28);             //��������ȡ7����0�����ݵ�buffer[0]-buffer[15]
    if(status != FM1702_OK)
    {
        LOGE("��������ʧ��");
        return FALSE;
    }
    else
    {
        g_ICCard_Value = (buf[0]<<8)|buf[1];  //��ȡ�������
        LOGI("�������ݳɹ������= %d", g_ICCard_Value);
    }

    return TRUE;
}


u8 card_runing(void)
{
    u8 status;          //��д��״̬����
    u8 buf[16]= {0};    //��д������buff
    u16 now_card_value = 0 ; //��ǰ��Ƭ��ȡ�����

    status=MIF_READ(buf,28); //��������ȡ7����0�����ݵ�buffer[0]-buffer[15]
    if(status != FM1702_OK)
    {
        LOGE("��������ʧ��");
        g_state = WAIT_IC; //����ʧ�ܣ���Ƭ���ߣ��豸״̬��λ
        return FALSE;
    }
    else
    {
        now_card_value = (buf[0]<<8)|buf[1];  //��ȡ�������
    }


    //����Ƭ�ϵ������ʵ����ƥ��ʱ��д�뿨Ƭ����
    if(now_card_value != g_ICCard_Value)
    {
        LOGI("�������ݳɹ������= %d", now_card_value);
        buf[0]=g_ICCard_Value>>8;
        buf[1]=g_ICCard_Value&0x00ff;	
        status=MIF_Write(buf,28);  //д������buffer[0]-buffer[15]д��1����0��
        if(status != FM1702_OK)
        {
            LOGE("д����ʧ��");
            g_state = WAIT_IC;     //д��ʧ�ܣ���Ƭ���ߣ��豸״̬��λ
            return FALSE;
        }
    }

    return TRUE;
}


u8 connect_to_server(void)
{
    u8 p[64] = {0};
    if(sim800c_send_cmd("AT","OK",100)) //����Ƿ�Ӧ��ATָ�� 
    {   
        LOGE("sim800cδ��ӦATָ��.");
        return FALSE;
    }

    if(sim800c_send_cmd("AT+CPIN?","OK",200))
    {
        LOGE("sim800cδ��⵽SIM��.");
        return FALSE;
    }

    sim800c_send_cmd("ATE0","OK",200);

    //������к�
    USART3_RX_STA=0;
    if(sim800c_send_cmd("AT+CGSN","OK",200))
    {
        LOGE("sim800cδ�������к�.");
        return FALSE;
    }
    char* temp = strstr( (char*)USART3_RX_BUF+2, "\r\n");
    temp[0] = 0;
    strncpy(g_device_code, (char*)USART3_RX_BUF+2, sizeof(g_device_code));
    LOGI("��⵽���к�: %s", g_device_code); //����sim�����к���Ϊ�豸���к�
    USART3_RX_STA=0;

    //����ź�ǿ��
    if(sim800c_send_cmd("AT+CSQ","+CSQ:",200))//��ѯ�ź�����
    {
        LOGE("sim800cδ�����ź�����");
        return FALSE;
    }
    else
    {
        char *end;
        end=strstr((const char*)(USART3_RX_BUF+2),"\r\n");
        *end= 0;//��������� 
        LOGI("sim800c�����ź�����:%s", USART3_RX_BUF+2);
        USART3_RX_STA=0;
    }


#if 0
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
    if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",1000)) 
        return FALSE;             //����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
    if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",1000))
        return FALSE;//����PDP������,��������Э��,��������Ϣ
    sim800c_send_cmd("AT+CGATT=1","OK",1000); //����GPRSҵ��
        //return FALSE;                    
    if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500))
        return FALSE;        //����ΪGPRS����ģʽ
        
    USART3_RX_STA=0;
    sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",HOST_IP,HOST_PORT);
    if(sim800c_send_cmd(p,"OK",500))
        return FALSE;     //��������

    u16 t=6;
    while(t--)
    {
        USART3_RX_STA=0;
        sim800c_send_cmd("AT+CIPSTATUS","OK",500);  //��ѯ����״̬
        if(strstr((const char*)USART3_RX_BUF,"CONNECT OK"))
        {
            LOGI("TCP���ӽ����ɹ�.");
            return TRUE;
        }
        else if(strstr((const char*)USART3_RX_BUF,"CLOSED"))
        {
            LOGI("TCP���ӽ���ʧ��.");
            return FALSE;
        }
        delay_ms(1000);
    }

    return FALSE;
}



u8 subscribe_mqtt(void)
{
    //��mqtt��������������
    u16 len;
    //����mqtt_connect����
    len=mqtt_connect_message(mqtt_msg, CLIENTID , USRNAME, PASSWD);//id,�û���������
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//���յ����ֽ���
    USART3_RX_STA = 0;
    if(sim800c_send_cmd(send_cmd,">",200)==0)//�������ݳɹ�
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                      //�������ʱ
        //sim800c_send_cmd((u8*)0X1A,0,0);   //CTRL+Z,�������ݷ���,����һ�δ���
        delay_ms(1000);                      //�������ʱ
        LOGI("MQTT���ֳɹ�.");
    }else if(sim800c_send_cmd(send_cmd,"ERROR",200)==0){
        LOGE("MQTT����ʧ��.");
        return FALSE;
    }


    len=mqtt_subscribe_message(mqtt_msg,TOPIC_SUB,1,1);//����test����
    //printf("send len = %d\r\n", len);
    LOGI("mqtt_subscribe... \r\n");
    PrintHex(mqtt_msg,len);
    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//���յ����ֽ���
    if(sim800c_send_cmd(send_cmd,">",200)==0)//��������
    {
        u3_printf_hex(mqtt_msg, len);
        delay_ms(1000);                 //�������ʱ
        //sim800c_send_cmd((u8*)0X1A,0,0);    //CTRL+Z,�������ݷ���,����һ�δ���
        delay_ms(1000);               //�������ʱ
        LOGI("MQTT��������ɹ�.");
    }
    else if(sim800c_send_cmd(send_cmd,"ERROR",200)==0){
        LOGE("MQTT��������ʧ��.");
        return FALSE;
    }

    return TRUE;
}



u8 send_keep_alive_mesaage(void)
{
    u16 len;
    u8 msg[300]={0}; //��������
    create_keep_alive_message(msg, sizeof(msg));
    PrintHex(msg, strlen(msg));
    LOGD("message:%s", msg);
    len = mqtt_publish_message(mqtt_msg, TOPIC_PUB, (char*)msg, 2);
    PrintHex(mqtt_msg,len);
    LOGD("send len=%d", len);


    //�������254���ֽڣ���Ҫ�ֶ�η���
    //ÿ�η���200���ֽ�
    u8 t = 0 ;  //��¼�Ѿ����͵Ĵ���

    while(len > 254){
        sprintf((char*)send_cmd, "AT+CIPSEND=%d", 200);//���յ����ֽ���
        if(sim800c_send_cmd(send_cmd,">",200)==0)//��������
        {
            LOGD("����MQTT��������.");
            u3_printf_hex(mqtt_msg + t*200, 200);
            delay_ms(200);                //�������ʱ
            USART3_RX_STA = 0;
        }
        t++;
        len = len - 200;
    }


    sprintf((char*)send_cmd, "AT+CIPSEND=%d", len);//���յ����ֽ���
    if(sim800c_send_cmd(send_cmd,">",200)==0)//��������
    {
        LOGD("����MQTT��������.");
        u3_printf_hex(mqtt_msg + t*200, len);
        delay_ms(1000);                 //�������ʱ
        delay_ms(1000);                //�������ʱ
        USART3_RX_STA = 0;
    }else if (sim800c_send_cmd(send_cmd,"ERROR",200)==0){
        //����ʧ�ܣ����ӿ��ܶϿ�
        LOGE("���ͱ�������ʧ��.");
        return FALSE;
    }

    return TRUE;
}

