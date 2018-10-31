#ifndef _APP_INCLUDED_
#define _APP_INCLUDED_

//////////////////////////////////////////////////////////////////////////////////	 
// 妯″潡鍔熻兘锛氭按鎺ф満涓氬姟閫昏緫澶勭悊							  
////////////////////////////////////////////////////////////////////////////////// 


#include "sys.h"


void main_loop(void);

//检测IC卡
//返回值：TRUE-检测到卡片          FALSE- 未检测到卡片
u8 scan_for_card(void);

//开始消费后，开始定时扣费，定时写入卡内余额
//定时扣费在RTC中断中执行，扣除全局变量
u8 card_runing(void);


//读取IC卡数据
//返回值：TRUE-读取程         FALSE- 读取失败
u8 read_card(void);


//写入IC卡服务器绑定ID 
u8 ic_wrtie_server_id(void);


//连接阿里云服务器
u8 connect_to_server(void); 

//订阅mqtt主题
u8 subscribe_mqtt(void);

//发送保活信息，设备指令1
u8 send_keep_alive_mesaage(void);

//处理保活信息响应，服务器指令1
u8 deal_keep_alive_mesaage_response(void);


//查询服务器发送过来的消息
u8 recv_mqtt_message(s8* trade);



//发送开始消费信令
u8 send_start_consume_mesaage(void);

//发送扣费信息
//ic_flag标志 1-IC卡消费， 2-app消费
//finish_flag标志 1-结束消费， 2-正在消费
u8 send_consume_mesaage(u8 ic_flag, u8 finish_flag);

//处理服务端指令6，app消费请求
// ok_flag 1 成功开始消费； -1 开始消费失败
u8 deal_app_cousume_command(s8 ok_flag);


//处理服务端指令8，app消费结束命令
u8 deal_command_app_finish(void);


//发送离线消费信息
u8 send_offline_mesaage(void);


#endif
