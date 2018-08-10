#include "delay.h"
#include "timer.h"
#include "usart.h"
#include "app.h"
#include "utils.h"
#include "fm1702.h" 
#include "logging.h"



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
        g_state = ON_IC;    //�ı䵱ǰ�豸״̬
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


