#ifndef _MESSAGE_INCLUDED_
#define _MESSAGE_INCLUDED_
#include "sys.h"


//生成指令1，定时保活信息
void create_keep_alive_message(u8 *outbuf, u16 len);

//生成指令2，开始刷卡消费信令
void create_start_consume_message(u8 *outbuf, u16 len);

//生成指令3和4，定时上传扣费信息，finish_flag为1时，生成结束扣费信息
void create_consume_message(u8 *outbuf, u16 len, u8 finish_flag);

//解析收到的服务器消息，协议公共部分，
//返回0表示失败，返回其他值表示成功，其他值为trade命令号
u8 parse_service_message_common(u8 *outbuf, u16 len);

//处理指令2的响应，服务器返回指令2响应
u8 deal_start_consume_response(u8 *outbuf, u16 len);

#endif
