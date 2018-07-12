#ifndef __SIM800C_H__
#define __SIM800C_H__	 
#include "sys.h"

#define swap16(x) (x&0XFF)<<8|(x&0XFF00)>>8	//高低字节交换宏定义


void sim_send_sms(u8*phonenumber,u8*msg);
void sim_at_response(u8 mode);	
u8* sim800c_check_cmd(u8 *str);
u8 sim800c_send_cmd(u8 *cmd,u8 *ack,u16 waittime);


void sim800c_mtest_ui(void);	         //sim800c主测试UI
u8 sim800c_gsminfo_show(void);	         //显示GSM模块信息
void ntp_update(void);               //网络同步时间
void sim800c_test(void);			 //sim800c主测试函数
#endif





