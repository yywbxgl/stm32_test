#ifndef __UTILS_H__
#define __UTILS_H__	 

#include "sys.h"


//连接代理服务器配置
//#define HOST_IP "124.90.130.2"
//#define HOST_PORT "8086"
#define HOST_IP "114.215.73.18"                     //mqtt代理服务器
#define HOST_PORT "1883"                            //mqtt代理服务器端口
#define CLIENTID "GID_water_test@@@TEST00003"
#define USRNAME "LTAIwfxXisNo3pDh"                  //usr
#define PASSWD "kqfxXvRTsAnQJHSbbDkj1B0iXfk="       //计算后的摘要
#define TOPIC "water_device_to_server_test"         //订阅主题


//公共部分字段
#define M_TRADE      "trade"      //交易指令--0,1,2,3.....
#define M_DATA       "data"       //请求数据json
#define M_VERSION    "version"    //接口版本号：默认10
#define M_CODE       "code"       //是否成功【-1 失败 1成功】
#define M_DEVICECODE "deviceCode" //水控机发起写“mqtt” 服务端发起写“mqServer”
#define M_ERROR      "errorMsg"   //错误信息

//公共部分字段值
#define M_VERSION_VALUE "10"
extern char g_device_code[24];


//指令1  字段
#define M_DISSABLE   "disable"    //禁用类型(0.不禁用，1.二维码，2.ic卡，3.全部)
#define M_ABNORMAL   "abnormal"   //异常类型(0.无异常，1.电磁阀异常，2.LED异常，3.龙头异常，4...)
#define M_HEART      "heart"      //心跳评率(秒)
#define M_OFFLINE    "offLine"    //是否允许离线消费(1.允许，0.不允许)
#define M_PWD        "pwd"        //用来解析ic卡的密码
#define M_CHARGRATE  "chargRate"  //单位扣费频率，单位秒
#define M_MONEYRATE  "moneyRate"  //单位扣费金额
#define M_LOGRATE    "logRate"    //提交扣费记录频率(指令3)，单位秒

//指令1  字段值
extern u8 g_disable;
extern u8 g_abnormal ;
extern u8 g_heart ;
extern u8 g_offLine ;
extern char g_pwd[24] ;
extern u8 g_chargRate ;
extern u8 g_moneyRate;
extern u8 g_logRate ;


//设备当前业务状态
extern u8   g_state;             //设备当前状态
extern char g_card_id[24];       //当前卡片ID
extern u16 g_ICCard_Value;       //卡片余额


enum{
    WAIT_IC,   //等待IC卡
    ON_IC,     //检测到IC卡
};


//打印BUF的二进制数据，用于调试
void PrintHex(u8 *buf,u16 len);

#endif
