#include "commands.h"
#include <jansson.h>
#include <string.h>

#define M_SENDER_VALUE "mqtt"
#define M_VERSION_VALUE "10"

//公共部分
#define M_TRADE      "trade"      //交易指令--0,1,2,3.....
#define M_DATA       "data"       //请求数据json
#define M_VERSION    "version"    //接口版本号：默认10
#define M_CODE       "code"       //是否成功【-1 失败 1成功】
#define M_SENDER     "sender"     //水控机发起写“mqtt” 服务端发起写“mqServer”
#define M_ERROR      "errorMsg"   //错误信息


//指令1  保活信令
#define M_DISSABLE   "disable"    //禁用类型(0.不禁用，1.二维码，2.ic卡，3.全部)
#define M_ABNORMAL   "abnormal"   //异常类型(0.无异常，1.电磁阀异常，2.LED异常，3.龙头异常，4...)
#define M_HEART      "heart"      //心跳评率(秒)
#define M_OFFLINE    "offLine"    //是否允许离线消费(1.允许，0.不允许)
#define M_PWD        "pwd"        //用来解析ic卡的密码
#define M_CHARGRATE  "chargRate"  //单位扣费频率，单位秒
#define M_MONEYRATE  "moneyRate"  //单位扣费金额
#define M_LOGRATE    "logRate"    //提交扣费记录频率(指令3)，单位秒


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

