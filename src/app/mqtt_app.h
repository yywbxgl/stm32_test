#ifndef __MQTT_APP_H__
#define __MQTT_APP_H__

#include "stm32f10x.h"

//����MQTT���Ӱ� 
u16 mqtt_connect_message(u8 *mqtt_message,char *client_id,char *username,char *password);

//����MQTT������Ϣ��
u16 mqtt_publish_message(u8 *mqtt_message, char * topic, char * message, u8 qos);

//����MQTT����ȷ�ϰ�
//��QoS����1�� PUBLISH ��Ϣ�Ļ�Ӧ
//������������ PUBLISH ��Ϣ�������߿ͻ��ˣ��ͻ��˻ظ� PUBACK ��Ϣ
u8 mqtt_puback_message(u8 *mqtt_message);


//����MQTT��������/ȡ�����İ�
//whether=1,����; whether=0,ȡ������
u16 mqtt_subscribe_message(u8 *mqtt_message,char *topic,u8 qos,u8 whether);


//����MQTT PING�����
u8 mqtt_ping_message(u8 *mqtt_message);


//����MQTT�Ͽ����Ӱ�
u8 mqtt_disconnect_message(u8 *mqtt_message);

#endif

