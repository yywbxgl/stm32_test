#ifndef __UTILS_H__
#define __UTILS_H__	 

#include "sys.h"

//������
#define DEBUG_TEST 


//���Ӵ������������
//#define HOST_IP "101.67.134.148"
//#define HOST_PORT "8086"
#define HOST_IP "mqtt-cn-0pp0nqosd05.mqtt.aliyuncs.com"  //mqtt���������
#define HOST_PORT "1883"                            //mqtt����������˿�
#define GROUPID "GID_water_test@@@"
#define USRNAME "LTAIwfxXisNo3pDh"                  //usr
#define PASSWD "kqfxXvRTsAnQJHSbbDkj1B0iXfk="       //������ժҪ
#define TOPIC_PUB "water_device_to_server_test"     //������Ϣ����
#define TOPIC_SUB "water_server_to_device_test"     //������Ϣ����

extern char g_clent_id[48]; 
extern char g_device_code[24]; //�豸���к�


//���������ֶ�
#define M_TRADE      "trade"      //����ָ��--0,1,2,3.....
#define M_DATA       "data"       //��������json
#define M_VERSION    "version"    //�ӿڰ汾�ţ�Ĭ��10
#define M_CODE       "code"       //�Ƿ�ɹ���-1 ʧ�� 1�ɹ���
#define M_DEVICECODE "deviceCode" //ˮ�ػ�����д��mqtt�� ����˷���д��mqServer��
#define M_ERROR      "errorMsg"   //������Ϣ
//���������ֶ�ֵ
#define M_VERSION_VALUE "10"


//ָ��1  �ֶΣ�����ָ��
#define M_DISSABLE   "disable"    //��������(0.�����ã�1.��ά�룬2.ic����3.ȫ��)
#define M_ABNORMAL   "abnormal"   //�쳣����(0.���쳣��1.��ŷ��쳣��2.LED�쳣��3.��ͷ�쳣��4...)
#define M_HEART      "heart"      //��������(��)
#define M_OFFLINE    "offLine"    //�Ƿ�������������(1.����0.������)
#define M_PWD        "pwd"        //��������ic��������
#define M_CHARGRATE  "chargRate"  //��λ�۷�Ƶ�ʣ���λ��
#define M_MONEYRATE  "moneyRate"  //��λ�۷ѽ��
#define M_LOGRATE    "logRate"    //�ύ�۷Ѽ�¼Ƶ��(ָ��3)����λ��
//ָ��1  �ֶ�ֵ
extern u8 g_disable;
extern u8 g_abnormal ;
extern u8 g_heart ;
extern u8 g_offLine ;
extern char g_pwd[24] ;
extern u8 g_chargRate ;
extern u8 g_moneyRate;
extern u8 g_logRate ;


//ָ��2�ֶΣ���ʼ����ָ��
#define M_CARDMONEY "cardMoney"
#define M_CARDNO    "cardNo"
//ָ��2�ֶ�ֵ
extern char g_card_id[24];   //��ǰ��ƬID, ���15λ
extern u32 g_ICCard_Value;   //��Ƭ���


#define M_switched "switched"
#define M_bingding "bingding"
#define M_serverCardNo "serverCardNo"
#define M_orderNo  "orderNo"

extern char g_orderNo[12];
extern u32 g_maxMoney;
extern u8 g_serverCardNo[10];



//ָ��3,4�ֶΣ��۷���Ϣ
#define M_TIME     "time"
#define M_MONEY    "money"



//ָ��6
#define M_MAX_MONEY "maxMoney"
#define M_EXPIRE "expire"

//�豸״̬ȫ�ֱ���
extern u16 g_consume_time;
extern u8  g_has_offline_order;
extern char  g_offline_msg[200];


//�豸��ǰҵ��״̬
extern u8   g_state;             //�豸��ǰ״̬
enum{
    INIT = 0,  //ϵͳ��ʼ��״̬ ����> ���ӷ�����
    TCP_OK,    //TCP���ӽ����ɹ�    ����> ����mqtt����
    MQTT_OK,   //mqtt���ĳɹ�  -> �ȴ�IC��
    WAIT_IC,   //�ȴ�IC��
    ON_IC,     //��⵽IC��
    WAIT_CONSUME_RESPONSE, //�ȴ���ʼ����������Ӧ
    IC_CONSUME,  //IC����ʼ����
    APP_CONSUME, // app��ʼ����
    NOT_AUTH,
};


//��ӡBUF�Ķ��������ݣ����ڵ���
void PrintHex(u8 *buf,u16 len);

#endif
