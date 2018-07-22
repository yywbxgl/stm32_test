#include "test.h"

//LED����
void led0_set(u8 sta)
{
    LED0=sta;
} 


//LED����
void led1_set(u8 sta)
{
    LED1=sta;
} 


//LED���Խӿ�
void led_test(void)
{
    delay_init();
    LED_Init();
    LED0 = 0;  //��LED
    LED1 = 0;
    delay_ms(500);
    LED0 = 1; //�ر�LED
    LED1 = 1;
}


//���������Խӿ�
void beep_test(void)
{
    delay_init();
    BEEP_Init();

    BEEP_Set();  //�򿪷�����
    delay_ms(500);
    BEEP_Reset(); //�رշ�����
}



//IC����д����
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
        
        status = Request(RF_CMD_REQUEST_ALL);       //Ѱ��
        printf("fm1702_test request find card. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status = AntiColl();                        //��ͻ���
        printf("fm1702_test AntiColl. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status=Select_Card();                       //ѡ��
        printf("fm1702_test Select_Card. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status = Load_keyE2_CPY(DefaultKey);          //��������
        printf("fm1702_test Load_keyE2_CPY. ret=%d\r\n", status);
        if(status != TRUE)         continue;

        status = Authentication(UID, 1, RF_CMD_AUTH_LA);//��֤1����keyA
        printf("fm1702_test Authentication. ret=%d\r\n", status);
        if(status != FM1702_OK)    continue;

        status=MIF_READ(buf,4);             //��������ȡ1����0�����ݵ�buffer[0]-buffer[15]
        printf("fm1702_test MIF_READ. ret=%d\r\n", status);
        //status=MIF_Write(buffer,4);       //д������buffer[0]-buffer[15]д��1����0��
        if(status == FM1702_OK)
        {
            //��д�ɹ�������LED
            printf("fm1702_test, read and write success.\r\n");
            delay_ms(10000);
        }
        
        delay_ms(1000);

    }
}


