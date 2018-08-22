#ifndef _MESSAGE_INCLUDED_
#define _MESSAGE_INCLUDED_
#include "sys.h"


//生成指令1，定时保活信息
void create_keep_alive_message(u8 *outbuf, u16 len);

//生成指令2，开始刷卡消费信令
void create_start_consume_message(u8 *outbuf, u16 len);

//生成指令3和4，定时上传扣费信息, 
//ic_flag标志 1-IC卡消费， 2-app消费
//finish_flag标志 1-结束消费， 2-正在消费
void create_consume_message(u8 *outbuf, u16 len, u8 ic_flag, u8 finish_flag);

//生成指令7，定时上传app消费扣费信息
void create_app_consume_message(u8 *outbuf, u16 len);


//生成指令6，app消费请求响应,ok_flag为1时返回code成功
void create_app_consume_response(u8 *outbuf, u16 len, u8 ok_flag);

//解析收到的服务器消息，协议公共部分，
//返回0表示失败，返回其他值表示成功，其他值为trade命令号
u8 parse_service_message_common(u8 *outbuf, u16 len);


//处理服务器指令2，开始刷卡消费请求的响应
u8 deal_start_consume_response(u8 *outbuf, u16 len);


//处理服务器指令6，开始app消费请求
u8 deal_start_app_consume(u8 *outbuf, u16 len);


#endif
