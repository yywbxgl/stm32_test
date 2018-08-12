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
    json_t* card_money = NULL;


    message = json_loads((char*)outbuf, JSON_ENCODE_ANY, NULL);
    if(!json_is_object(message)){
        LOGE("Json 数据格式错误[message]");
        ret = FALSE;
    }
    
    param = json_object_get(message, "data");
    if(!json_is_object(param)){
        LOGE("Json 数据格式错误[data]");
        ret = FALSE;
    }

    ordNo = json_object_get(param, M_orderNo);
    if(!json_is_object(param)){
        LOGD("Json 数据格式错误[M_orderNo]");
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

    serverCardNo = json_object_get(param, M_serverCardNo);
    if(!json_string_value(serverCardNo)){
        LOGE("Json 数据格式错误. [M_serverCardNo]");
        ret = FALSE;
    }else{
        strncpy((char*)g_serverCardNo, json_string_value(serverCardNo), sizeof(g_serverCardNo));
        LOGD("返回卡片服务ID=%s", g_serverCardNo);
    }


    binging = json_object_get(param, M_bingding);
    if (json_integer_value(binging) == 0 ){
        LOGE("卡片未绑定");
        //数码管显示服务ID
        printf("\r\n------\r\n");
        u8 i =0;
        for(;i<9;++i){
            g_Digitron[i] = g_serverCardNo[i] - '0'; //数码管显示数字
            printf("%c", g_serverCardNo[i]);
        }
        printf("\r\n------\r\n");
        ret = FALSE;
    }else if( json_integer_value(binging) == 1){
        //strncpy((char*)g_serverCardNo, json_string_value(serverCardNo), sizeof(g_serverCardNo));
        ic_wrtie_server_id();
    }

    card_money = json_object_get(param, M_CARDMONEY);
    if (json_integer_value(card_money) != g_ICCard_Value ){
        g_ICCard_Value = json_integer_value(card_money);
        LOGI("服务端返回卡内余额：%d", g_ICCard_Value);
    }


    if (ret == FALSE){
       json_decref(message);
       json_decref(param);
       json_decref(ordNo);
       json_decref(binging);
       json_decref(switched);
       json_decref(serverCardNo);
       json_decref(card_money);
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





