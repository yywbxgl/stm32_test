#include "test.h"

//LED设置
void led0_set(u8 sta)
{
    LED0=sta;
} 


//LED设置
void led1_set(u8 sta)
{
    LED1=sta;
} 


//LED测试接口
void led_test(void)
{
    delay_init();
    LED_Init();
    LED0 = 0;  //打开LED
    LED1 = 0;
    delay_ms(500);
    LED0 = 1; //关闭LED
    LED1 = 1;
}


//蜂鸣器测试接口
void beep_test(void)
{
    delay_init();
    BEEP_Init();

    BEEP_Set();  //打开蜂鸣器
    delay_ms(500);
    BEEP_Reset(); //关闭蜂鸣器
}



//IC卡读写测试
void fm1702_test(void)
{
    unsigned char status, try;
    unsigned char buf[16], DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

    LED0 = 0;
    Init_FM1702();
    LED0 = 1;

    try = 5;
    while(--try)
    {
        
        status = Request(RF_CMD_REQUEST_ALL);       //寻卡
        printf("fm1702_test request find card. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status = AntiColl();                        //冲突检测
        printf("fm1702_test AntiColl. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status=Select_Card();                       //选卡
        printf("fm1702_test Select_Card. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status = Load_keyE2_CPY(DefaultKey);          //加载密码
        printf("fm1702_test Load_keyE2_CPY. ret=%d\r\n", status);
        if(status != TRUE)         continue;

        status = Authentication(UID, 1, RF_CMD_AUTH_LA);//验证1扇区keyA
        printf("fm1702_test Authentication. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status=MIF_READ(buf,4);             //读卡，读取1扇区0块数据到buffer[0]-buffer[15]
        printf("fm1702_test MIF_READ. ret=%d\r\n", status);
        //status=MIF_Write(buffer,4);       //写卡，将buffer[0]-buffer[15]写入1扇区0块
        if(status == FM1702_OK)
        {
            //读写成功，点亮LED
            printf("fm1702_test, read and write success.\r\n");
            delay_ms(10000);
        }
        
        delay_ms(1000);

    }
}


