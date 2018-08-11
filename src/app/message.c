#include "message.h"
#include "utils.h"
#include <jansson.h>
#include <string.h>
#include "logging.h"



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
   //json_object_set_new(param, M_CARDNO, json_string("g_card_id")); 
   json_object_set_new(param, M_CARDNO, json_string("3245234523test"));     // --sun-- test
   
   json_object_set_new(message, M_TRADE, json_string("2"));
   json_object_set_new(message, M_DATA, param);
   json_object_set_new(message, M_VERSION, json_string(M_VERSION_VALUE));
   json_object_set_new(message, M_CODE, json_integer(2));
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
    u8 ret = TRUE;
    json_t* message = NULL;
    json_t* param = NULL;
    json_t* ordNo = NULL;
    json_t* binging = NULL;
    json_t* switched = NULL;
    json_t* serverCardNo = NULL;


    message = json_loads((char*)outbuf, JSON_ENCODE_ANY, NULL);
    if(!json_is_object(message)){
        LOGE("Json 数据格式错误");
        ret = FALSE;
    }
    
    param = json_object_get(message, "data");
    if(!json_is_object(param)){
        LOGE("Json 数据格式错误");
        ret = FALSE;
    }

    ordNo = json_object_get(param, M_orderNo);
    if(!json_is_object(param)){
        LOGD("Json 数据格式错误");
        ret = FALSE;
    }else{
        LOGI("当前消费订单:%s", json_string_value(ordNo));
        strncpy(g_orderNo, json_string_value(ordNo), sizeof(g_orderNo));
    }

    switched = json_object_get(param, M_switched);
    if (json_integer_value(switched) == 0 ){
        LOGE("服务器不允许放水");
        ret = FALSE;
    }

    binging = json_object_get(param, M_bingding);
    if (json_integer_value(binging) == 0 ){
        LOGE("卡片未绑定");
        ret = FALSE;
    }

   if (ret == FALSE){
       json_decref(message);
       json_decref(param);
       json_decref(ordNo);
       json_decref(binging);
       json_decref(switched);
       json_decref(serverCardNo);
   }
   
   return ret;
}



void create_consume_message(u8 *outbuf, u16 len, u8 finish_flag)
{
    char* out;
    json_t* message = json_object();
    json_t* param = json_object();
    memset(outbuf, 0 , len);

    json_object_set_new(param, M_orderNo, json_string(g_orderNo));
    json_object_set_new(param, M_TIME, json_integer(g_consume_time*g_chargRate)); 
    json_object_set_new(param, M_CARDMONEY, json_integer(g_ICCard_Value));
    json_object_set_new(param, M_MONEY, json_integer(g_consume_time*g_moneyRate)); 

    //是否未结束消费信令
    if (finish_flag)
        json_object_set_new(message, M_TRADE, json_string("4"));
    else
        json_object_set_new(message, M_TRADE, json_string("3"));
    json_object_set_new(message, M_DATA, param);
    json_object_set_new(message, M_VERSION, json_string(M_VERSION_VALUE));
    json_object_set_new(message, M_CODE, json_integer(2));
    json_object_set_new(message, M_DEVICECODE, json_string(g_device_code));
    json_object_set_new(message, M_ERROR, json_string(""));

    out = json_dumps(message, 0);
    strncpy((char*)outbuf, out, len);

    free(out);
    json_decref(param);
    json_decref(message);

}





