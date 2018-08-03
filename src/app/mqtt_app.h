#ifndef __MQTT_APP_H__
#define __MQTT_APP_H__

#include "stm32f10x.h"

//构建MQTT连接包 
u16 mqtt_connect_message(u8 *mqtt_message,char *client_id,char *username,char *password);

//构建MQTT发布消息包
u16 mqtt_publish_message(u8 *mqtt_message, char * topic, char * message, u8 qos);

//构建MQTT发布确认包
//对QoS级别1的 PUBLISH 消息的回应
//当服务器发送 PUBLISH 消息给订阅者客户端，客户端回复 PUBACK 消息
u8 mqtt_puback_message(u8 *mqtt_message);


//构建MQTT订阅请求/取消订阅包
//whether=1,订阅; whether=0,取消订阅
u16 mqtt_subscribe_message(u8 *mqtt_message,char *topic,u8 qos,u8 whether);


//构建MQTT PING请求包
u8 mqtt_ping_message(u8 *mqtt_message);


//构建MQTT断开连接包
u8 mqtt_disconnect_message(u8 *mqtt_message);

#endif

