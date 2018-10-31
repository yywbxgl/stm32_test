#ifndef _APP_INCLUDED_
#define _APP_INCLUDED_

//////////////////////////////////////////////////////////////////////////////////	 
// 模块功能：水控机业务逻辑处理							  
////////////////////////////////////////////////////////////////////////////////// 


#include "sys.h"


void main_loop(void);

//���IC��
//����ֵ��TRUE-��⵽��Ƭ          FALSE- δ��⵽��Ƭ
u8 scan_for_card(void);

//��ʼ���Ѻ󣬿�ʼ��ʱ�۷ѣ���ʱд�뿨�����
//��ʱ�۷���RTC�ж���ִ�У��۳�ȫ�ֱ���
u8 card_runing(void);


//��ȡIC������
//����ֵ��TRUE-��ȡ��         FALSE- ��ȡʧ��
u8 read_card(void);


//д��IC����������ID 
u8 ic_wrtie_server_id(void);


//���Ӱ����Ʒ�����
u8 connect_to_server(void); 

//����mqtt����
u8 subscribe_mqtt(void);

//���ͱ�����Ϣ���豸ָ��1
u8 send_keep_alive_mesaage(void);

//��������Ϣ��Ӧ��������ָ��1
u8 deal_keep_alive_mesaage_response(void);


//��ѯ���������͹�������Ϣ
u8 recv_mqtt_message(s8* trade);



//���Ϳ�ʼ��������
u8 send_start_consume_mesaage(void);

//���Ϳ۷���Ϣ
//ic_flag��־ 1-IC�����ѣ� 2-app����
//finish_flag��־ 1-�������ѣ� 2-��������
u8 send_consume_mesaage(u8 ic_flag, u8 finish_flag);

//��������ָ��6��app��������
// ok_flag 1 �ɹ���ʼ���ѣ� -1 ��ʼ����ʧ��
u8 deal_app_cousume_command(s8 ok_flag);


//��������ָ��8��app���ѽ�������
u8 deal_command_app_finish(void);


//��������������Ϣ
u8 send_offline_mesaage(void);


#endif
