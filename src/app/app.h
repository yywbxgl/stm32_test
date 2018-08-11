#ifndef _APP_INCLUDED_
#define _APP_INCLUDED_

//////////////////////////////////////////////////////////////////////////////////	 
// 妯″潡鍔熻兘锛氭按鎺ф満涓氬姟閫昏緫澶勭悊							  
////////////////////////////////////////////////////////////////////////////////// 


#include "sys.h"


//检测IC卡
//返回值：TRUE-检测到卡片          FALSE- 未检测到卡片
u8 scan_for_card(void);

//开始消费后，开始定时扣费，定时写入卡内余额
//定时扣费在RTC中断中执行，扣除全局变量
u8 card_runing(void);

//连接阿里云服务器
u8 connect_to_server(void); 

//订阅mqtt主题
u8 subscribe_mqtt(void);

//发送保活信息
u8 send_keep_alive_mesaage(void);


#endif
