#include "commands.h"
#include <jansson.h>
#include <string.h>

#define M_SENDER_VALUE "mqtt"
#define M_VERSION_VALUE "10"

//��������
#define M_TRADE      "trade"      //����ָ��--0,1,2,3.....
#define M_DATA       "data"       //��������json
#define M_VERSION    "version"    //�ӿڰ汾�ţ�Ĭ��10
#define M_CODE       "code"       //�Ƿ�ɹ���-1 ʧ�� 1�ɹ���
#define M_SENDER     "sender"     //ˮ�ػ�����д��mqtt�� ����˷���д��mqServer��
#define M_ERROR      "errorMsg"   //������Ϣ


//ָ��1  ��������
#define M_DISSABLE   "disable"    //��������(0.�����ã�1.��ά�룬2.ic����3.ȫ��)
#define M_ABNORMAL   "abnormal"   //�쳣����(0.���쳣��1.��ŷ��쳣��2.LED�쳣��3.��ͷ�쳣��4...)
#define M_HEART      "heart"      //��������(��)
#define M_OFFLINE    "offLine"    //�Ƿ�������������(1.����0.������)
#define M_PWD        "pwd"        //��������ic��������
#define M_CHARGRATE  "chargRate"  //��λ�۷�Ƶ�ʣ���λ��
#define M_MONEYRATE  "moneyRate"  //��λ�۷ѽ��
#define M_LOGRATE    "logRate"    //�ύ�۷Ѽ�¼Ƶ��(ָ��3)����λ��


u8 g_disable = 0;
u8 g_abnormal = 0;
u8 g_heart = 5;
u8 g_offLine = 1;
char g_pwd[64] = "test111111111";
u8 g_chargRate = 1;
u8 g_moneyRate = 7;
u8 g_logRate = 1;


char g_card_id[12] = "1234567890";
u16 g_ICCard_Value = 0;


void create_keep_alive_message(u8 *outbuf, u16 len)
{
    char* out;
    json_t* message = json_object();
    json_t* param = json_object();
    memset(outbuf, 0 , len);
        
    json_object_set_new(param, M_DISSABLE, json_integer(g_disable));
    json_object_set_new(param, M_ABNORMAL, json_integer(g_abnormal));
    json_object_set_new(param, M_HEART, json_integer(g_heart));
    json_object_set_new(param, M_OFFLINE, json_integer(g_offLine));
    json_object_set_new(param, M_PWD, json_string(g_pwd));
    json_object_set_new(param, M_CHARGRATE, json_integer(g_chargRate));
    json_object_set_new(param, M_MONEYRATE, json_integer(g_moneyRate));
    json_object_set_new(param, M_LOGRATE, json_integer(g_logRate));


    json_object_set_new(message, M_TRADE, json_string("0"));
    json_object_set_new(message, M_DATA, param);
    json_object_set_new(message, M_VERSION, json_string(M_VERSION_VALUE));
    json_object_set_new(message, M_CODE, json_integer(1));
    json_object_set_new(message, M_SENDER, json_string(M_SENDER_VALUE));
    json_object_set_new(message, M_ERROR, json_string(""));

    out = json_dumps(message, 0);

    strncpy((char*)outbuf, out, len);

    free(out);
    json_decref(param);
    json_decref(message);

}

