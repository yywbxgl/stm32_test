#include "delay.h"
#include "timer.h"
#include "usart.h"
#include "app.h"
#include "utils.h"
#include "fm1702.h" 
#include "logging.h"



u8 scan_for_card(void)
{
    u8 status;          //读写卡状态返回
    u8 buf[16]= {0};    //读写卡缓冲buff
    u8 DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  //todo:使用g_pwd转换
    status = Request(RF_CMD_REQUEST_ALL);       //寻卡
    if(status != FM1702_OK){
        LOGD("未检测到卡片");
        return FALSE;
    }

    status = AntiColl();                        //冲突检测
    if(status != FM1702_OK){
        LOGE("卡片冲突");
        return FALSE;
    }

    status=Select_Card();                       //选卡
    if(status != FM1702_OK){
        LOGE("选择卡片失败");
        return FALSE;
    }

    status = Load_keyE2_CPY(DefaultKey);          //加载密码
    if(status != TRUE){
        LOGE("加载密码失败");
        return FALSE;
    }

    status = Authentication(UID, 7, RF_CMD_AUTH_LA);//验证1扇区keyA
    if(status != FM1702_OK){
        LOGE("验证扇区失败");
        return FALSE;
    }

    sprintf(g_card_id, "%02x%02x%02x%02x", UID[0], UID[1], UID[2], UID[3]);
    LOGI("读取到卡片唯一号：%s", g_card_id);

    status=MIF_READ(buf,28);             //读卡，读取7扇区0块数据到buffer[0]-buffer[15]
    if(status != FM1702_OK)
    {
        LOGE("读卡数据失败");
        return FALSE;
    }
    else
    {
        g_ICCard_Value = (buf[0]<<8)|buf[1];  //读取卡内余额
        LOGI("读卡数据成功，余额= %d", g_ICCard_Value);
        g_state = ON_IC;    //改变当前设备状态
    }

    return TRUE;
}



u8 card_runing(void)
{
    u8 status;          //读写卡状态返回
    u8 buf[16]= {0};    //读写卡缓冲buff
    u16 now_card_value = 0 ; //当前卡片读取的余额

    status=MIF_READ(buf,28); //读卡，读取7扇区0块数据到buffer[0]-buffer[15]
    if(status != FM1702_OK)
    {
        LOGE("读卡数据失败");
        g_state = WAIT_IC; //读卡失败，卡片移走，设备状态置位
        return FALSE;
    }
    else
    {
        now_card_value = (buf[0]<<8)|buf[1];  //读取卡内余额
    }


    //当卡片上的余额与实际余额不匹配时候，写入卡片数据
    if(now_card_value != g_ICCard_Value)
    {
        LOGI("读卡数据成功，余额= %d", now_card_value);
        buf[0]=g_ICCard_Value>>8;
        buf[1]=g_ICCard_Value&0x00ff;	
        status=MIF_Write(buf,28);  //写卡，将buffer[0]-buffer[15]写入1扇区0块
        if(status != FM1702_OK)
        {
            LOGE("写数据失败");
            g_state = WAIT_IC;     //写卡失败，卡片移走，设备状态置位
            return FALSE;
        }
    }

    return TRUE;
}


