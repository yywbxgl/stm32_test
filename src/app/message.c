#include "message.h"
#include "utils.h"
#include <jansson.h>
#include <string.h>
#include "logging.h"
#include "app.h"
#include "digitron.h"



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


    json_object_set_new(message, M_TRADE, json_string("1"));
    json_object_set_new(message, M_DATA, param);
    json_object_set_new(message, M_VERSION, json_string(M_VERSION_VALUE));
    json_object_set_new(message, M_CODE, json_integer(1));
    json_object_set_new(message, M_DEVICECODE, json_string(g_device_code));
    json_object_set_new(message, M_ERROR, json_string(""));

    out = json_dumps(message, 0);
    strncpy((char*)outbuf, out, len);

    free(out);
    json_decref(param);
    json_decref(message);

}



void create_start_consume_message(u8 *outbuf, u16 len)
{

    char* out;
    json_t* message = json_object();
    json_t* param = json_object();
    memset(outbuf, 0 , len);

    json_object_set_new(param, M_CARDMONEY, json_integer(g_ICCard_Value));
    json_object_set_new(param, M_CARDNO, json_string(g_card_id));
    //LOGD("创建消费请求：")
    //PrintHex(g_serverCardNo, sizeof(g_serverCardNo));
    if (g_serverCardNo[0] == 0){
        json_object_set_new(param, M_serverCardNo, json_string("00000000"));
    }else{
        json_object_set_new(param, M_serverCardNo, json_string((char*)g_serverCardNo));
    }
    //sjson_object_set_new(param, M_serverCardNo, json_string(g_serverCardNo));
    //json_object_set_new(param, M_CARDNO, json_string("3245234523test"));     // --sun-- test

    json_object_set_new(message, M_TRADE, json_string("2"));
    json_object_set_new(message, M_DATA, param);
    json_object_set_new(message, M_VERSION, json_string(M_VERSION_VALUE));
    json_object_set_new(message, M_CODE, json_integer(1));
    json_object_set_new(message, M_DEVICECODE, json_string(g_device_code));
    json_object_set_new(message, M_ERROR, json_string(""));

    out = json_dumps(message, 0);
    strncpy((char*)outbuf, out, len);

    free(out);
    json_decref(param);
    json_decref(message);

}


//生成指令6，app消费请求响应
void create_app_consume_response(u8 *outbuf, u16 len, u8 ok_flag)
{
    char* out;
   json_t* message = json_object();
   json_t* param = json_object();
   memset(outbuf, 0 , len);


   json_object_set_new(param, M_orderNo, json_string(g_orderNo));
   json_object_set_new(param, M_CODE, json_integer(ok_flag));
   

   json_object_set_new(message, M_TRADE, json_string("6"));
   json_object_set_new(message, M_DATA, param);
   json_object_set_new(message, M_VERSION, json_string(M_VERSION_VALUE));
   json_object_set_new(message, M_CODE, json_integer(1));
   json_object_set_new(message, M_DEVICECODE, json_string(g_device_code));
   json_object_set_new(message, M_ERROR, json_string(""));

   out = json_dumps(message, 0);
   strncpy((char*)outbuf, out, len);

   free(out);
   json_decref(param);
   json_decref(message);
}


u8 deal_start_consume_response(u8 *outbuf, u16 len)
{
    json_t* message = NULL;
    json_t* param = NULL;
    json_t* ordNo = NULL;
    json_t* binging = NULL;
    json_t* switched = NULL;
    json_t* serverCardNo = NULL;
    json_t* card_money = NULL;


    message = json_loads((char*)outbuf, JSON_ENCODE_ANY, NULL);
    if(!message || !json_is_object(message)){
        LOGE("Json 数据格式错误[message]");
        json_decref(message);
        return FALSE;
    }
    
    param = json_object_get(message, "data");
    if(!param || !json_is_object(param)){
        LOGE("Json 数据格式错误[data]");
        json_decref(message);
        return FALSE;
    }

    ordNo = json_object_get(param, M_orderNo);
    if(!ordNo || !json_is_string(ordNo)){
        LOGE("Json 数据格式错误[M_orderNo]");
        json_decref(message);
        return FALSE;
    }else{
        LOGI("当前消费订单:%s", json_string_value(ordNo));
        strncpy(g_orderNo, json_string_value(ordNo), sizeof(g_orderNo));
    }

    serverCardNo = json_object_get(param, M_serverCardNo);
    if(!serverCardNo || !json_is_string(serverCardNo)){
        LOGE("Json 数据格式错误[serverCardNo]");
        json_decref(message);
        return FALSE;
    }else{
        strncpy((char*)g_serverCardNo, json_string_value(serverCardNo), sizeof(g_serverCardNo));
        LOGD("返回卡片服务ID=%s", g_serverCardNo);
    }


    binging = json_object_get(param, M_bingding);
    if(!binging || !json_is_integer(binging)){
        LOGE("Json 数据格式错误[binging]");
        json_decref(message);
        return FALSE;
    }else if (json_integer_value(binging) == 0 ){
        LOGE("卡片未绑定");
        //数码管显示服务ID
        //printf("\r\n------\r\n");
        u8 i =0;
        for(;i<9;++i){
            g_Digitron[i] = g_serverCardNo[i] - '0'; //数码管显示数字
        //    printf("%c", g_serverCardNo[i]);
        }
        //printf("\r\n------\r\n");
        json_decref(message);
        return FALSE;
    }else if( json_integer_value(binging) == 1){
        //strncpy((char*)g_serverCardNo, json_string_value(serverCardNo), sizeof(g_serverCardNo));
        //烧写服务ID到卡片
        ic_wrtie_server_id();
    }

    card_money = json_object_get(param, M_CARDMONEY);
    if(!card_money || !json_is_integer(card_money)){
        LOGD("Json 数据格式错误[card_money]");
        json_decref(message);
        return FALSE;
    }else if (json_integer_value(card_money) != g_ICCard_Value ){
        g_ICCard_Value = json_integer_value(card_money);
        LOGI("服务端返回卡内余额：%d", g_ICCard_Value);
    }

    switched = json_object_get(param, M_switched);
    if(!switched || !json_is_integer(switched)){
        LOGD("Json 数据格式错误[switched]");
        json_decref(message);
        return FALSE;
    }else if (json_integer_value(switched) == 0){
        LOGE("服务器不允许放水");
        json_decref(message);
        return FALSE;
    }


    json_t* chargRate = json_object_get(param, M_CHARGRATE);
    if(!chargRate || !json_is_number(chargRate)){
       LOGE("Json 数据格式错误[chargRate]");
       json_decref(message);
       return FALSE;
    }else{
       g_chargRate = json_integer_value(chargRate);
       LOGI("扣费频率:%d", g_chargRate);
    }

    json_t* moneyRate = json_object_get(param, M_MONEYRATE);
    if(!moneyRate || !json_is_number(moneyRate)){
       LOGE("Json 数据格式错误[moneyRate]");
       json_decref(message);
       return FALSE;
    }else{
       g_moneyRate = json_integer_value(moneyRate);
       LOGI("单位扣费金额:%d", g_moneyRate);
    }

    json_t* logRate = json_object_get(param, M_LOGRATE);
    if(!logRate || !json_is_number(logRate)){
       LOGE("Json 数据格式错误[logRate]");
       json_decref(message);
       return FALSE;
    }else{
       g_logRate = json_integer_value(logRate);
       LOGI("上报频率:%d", g_logRate);
    }

    json_decref(message);
    return TRUE;
}

void create_consume_message(u8 *outbuf, u16 len, u8 ic_flag, u8 finish_flag)
{
    char* out;
    json_t* message = json_object();
    json_t* param = json_object();
    memset(outbuf, 0 , len);

    json_object_set_new(param, M_orderNo, json_string(g_orderNo));
    json_object_set_new(param, M_TIME, json_integer(g_consume_time*g_chargRate)); 
    json_object_set_new(param, M_MONEY, json_integer(g_consume_time*g_moneyRate)); 

    if (ic_flag == 1){
        json_object_set_new(param, M_CARDMONEY, json_integer(g_ICCard_Value));
    }

    if (ic_flag == 1){
        if(finish_flag ==2){
            json_object_set_new(message, M_TRADE, json_string("3"));  //IC卡定时扣费
        }else if (finish_flag == 1){
            json_object_set_new(message, M_TRADE, json_string("4")); //IC卡结束扣费
        }
    }else if (ic_flag == 2){
        if(finish_flag ==2){
            json_object_set_new(message, M_TRADE, json_string("7"));  //app消费定时扣费
        }else if (finish_flag == 1){
            json_object_set_new(message, M_TRADE, json_string("8")); //app消费结束扣费
        }
    }

    json_object_set_new(message, M_DATA, param);
    json_object_set_new(message, M_VERSION, json_string(M_VERSION_VALUE));
    json_object_set_new(message, M_CODE, json_integer(1));
    json_object_set_new(message, M_DEVICECODE, json_string(g_device_code));
    json_object_set_new(message, M_ERROR, json_string(""));

    out = json_dumps(message, 0);
    strncpy((char*)outbuf, out, len);

    free(out);
    json_decref(param);
    json_decref(message);

}


void create_app_consume_message(u8 *outbuf, u16 len)
{
    char* out;
    json_t* message = json_object();
    json_t* param = json_object();
    memset(outbuf, 0 , len);

    json_object_set_new(param, M_orderNo, json_string(g_orderNo));
    json_object_set_new(param, M_TIME, json_integer(g_consume_time*g_chargRate)); 
    json_object_set_new(param, M_MONEY, json_integer(g_consume_time*g_moneyRate)); 

    json_object_set_new(message, M_TRADE, json_string("7"));
    json_object_set_new(message, M_DATA, param);
    json_object_set_new(message, M_VERSION, json_string(M_VERSION_VALUE));
    json_object_set_new(message, M_CODE, json_integer(1));
    json_object_set_new(message, M_DEVICECODE, json_string(g_device_code));
    json_object_set_new(message, M_ERROR, json_string(""));

    out = json_dumps(message, 0);
    strncpy((char*)outbuf, out, len);

    free(out);
    json_decref(param);
    json_decref(message);

}

u8 parse_service_message_common(u8 *outbuf, u16 len)
{
    u8 ret = 0;
    json_t* message = NULL;
    //json_t* data = NULL;
    json_t* trade = NULL;
    //json_t* version = NULL;
    //json_t* code = NULL;
    //json_t* deviceCode = NULL;
    //json_t* errorMsg = NULL;

    json_error_t error;
    message = json_loads((char*)outbuf, 0, &error);
    if(!message){
        LOGE("Json 数据格式错误[message]");
        json_decref(message);
        return FALSE;
    }

    trade = json_object_get(message, M_TRADE);
    if (!trade || !json_is_string(trade)){
        LOGE("Json 数据格式错误[trade]");
        json_decref(message);
        return FALSE;
    }
    else
    {
        const char* trade_value = json_string_value(trade);
        LOGI("收到服务器指令[%s]", trade_value);
        ret = atoi(trade_value);
    }

    json_decref(message);
    return ret;
}



u8 deal_start_app_consume(u8 *outbuf, u16 len)
{
    json_t* message = NULL;
    json_t* data = NULL;
    json_error_t error;
    message = json_loads((char*)outbuf, 0, &error);
    if(!message){
        LOGE("Json 数据格式错误[message]");
        json_decref(message);
        return FALSE;
    }

    data = json_object_get(message, M_DATA);
    if (!data || !json_is_object(data)){
        LOGE("Json 数据格式错误[data]");
        json_decref(message);
        return FALSE;
    }

    json_t* ordNo = json_object_get(data, M_orderNo);
    if(!ordNo || !json_is_string(ordNo)){
        LOGE("Json 数据格式错误[M_orderNo]");
        json_decref(message);
        return FALSE;
    }else{
        LOGI("当前app消费订单:%s", json_string_value(ordNo));
        strncpy(g_orderNo, json_string_value(ordNo), sizeof(g_orderNo));
    }

    json_t* maxMoney = json_object_get(data, M_MAX_MONEY);
    if(!maxMoney || !json_is_number(maxMoney)){
        LOGE("Json 数据格式错误[maxMoney]");
        json_decref(message);
        return FALSE;
    }else{
        g_maxMoney = json_integer_value(maxMoney);
        LOGI("最大消费金额:%d", maxMoney);
    }

    json_t* chargRate = json_object_get(data, M_CHARGRATE);
    if(!chargRate || !json_is_number(chargRate)){
        LOGE("Json 数据格式错误[chargRate]");
        json_decref(message);
        return FALSE;
    }else{
        g_chargRate = json_integer_value(chargRate);
        LOGI("扣费频率:%d", g_chargRate);
    }

    json_t* moneyRate = json_object_get(data, M_MONEYRATE);
    if(!moneyRate || !json_is_number(moneyRate)){
        LOGE("Json 数据格式错误[moneyRate]");
        json_decref(message);
        return FALSE;
    }else{
        g_moneyRate = json_integer_value(moneyRate);
        LOGI("单位扣费金额:%d", g_moneyRate);
    }

    json_t* logRate = json_object_get(data, M_LOGRATE);
    if(!logRate || !json_is_number(logRate)){
        LOGE("Json 数据格式错误[logRate]");
        json_decref(message);
        return FALSE;
    }else{
        g_logRate = json_integer_value(logRate);
        LOGI("上报频率:%d", g_logRate);
    }


    json_t* expire = json_object_get(data, M_EXPIRE);
    if(!expire || !json_is_string(expire)){
        LOGE("Json 数据格式错误[expire]");
        json_decref(message);
        return FALSE;
    }else{
        LOGI("当前请求过期时间:%s", json_string_value(expire));
        //暂时不处理请求是否过期
    }

    return TRUE;

}



