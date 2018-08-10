#ifndef __UTILS_H__
#define __UTILS_H__	 

#include "sys.h"


//���Ӵ������������
//#define HOST_IP "124.90.130.2"
//#define HOST_PORT "8086"
#define HOST_IP "114.215.73.18"                     //mqtt���������
#define HOST_PORT "1883"                            //mqtt����������˿�
#define CLIENTID "GID_water_test@@@TEST00003"
#define USRNAME "LTAIwfxXisNo3pDh"                  //usr
#define PASSWD "kqfxXvRTsAnQJHSbbDkj1B0iXfk="       //������ժҪ
#define TOPIC "water_device_to_server_test"         //��������


//���������ֶ�
#define M_TRADE      "trade"      //����ָ��--0,1,2,3.....
#define M_DATA       "data"       //��������json
#define M_VERSION    "version"    //�ӿڰ汾�ţ�Ĭ��10
#define M_CODE       "code"       //�Ƿ�ɹ���-1 ʧ�� 1�ɹ���
#define M_DEVICECODE "deviceCode" //ˮ�ػ�����д��mqtt�� ����˷���д��mqServer��
#define M_ERROR      "errorMsg"   //������Ϣ

//���������ֶ�ֵ
#define M_VERSION_VALUE "10"
extern char g_device_code[24];


//ָ��1  �ֶ�
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


//�豸��ǰҵ��״̬
extern u8   g_state;             //�豸��ǰ״̬
extern char g_card_id[24];       //��ǰ��ƬID
extern u16 g_ICCard_Value;       //��Ƭ���


enum{
    WAIT_IC,   //�ȴ�IC��
    ON_IC,     //��⵽IC��
};


//��ӡBUF�Ķ��������ݣ����ڵ���
void PrintHex(u8 *buf,u16 len);

#endif
