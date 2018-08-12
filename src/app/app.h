#ifndef _APP_INCLUDED_
#define _APP_INCLUDED_

//////////////////////////////////////////////////////////////////////////////////	 
// 模块功能：水控机业务逻辑处理							  
////////////////////////////////////////////////////////////////////////////////// 


#include "sys.h"


//���IC��
//����ֵ��TRUE-��⵽��Ƭ          FALSE- δ��⵽��Ƭ
u8 scan_for_card(void);

//��ʼ���Ѻ󣬿�ʼ��ʱ�۷ѣ���ʱд�뿨�����
//��ʱ�۷���RTC�ж���ִ�У��۳�ȫ�ֱ���
u8 card_runing(void);


//д��IC����������ID 
u8 ic_wrtie_server_id(void);




//���Ӱ����Ʒ�����
u8 connect_to_server(void); 

//����mqtt����
u8 subscribe_mqtt(void);

//���ͱ�����Ϣ
u8 send_keep_alive_mesaage(void);

//��ѯ���յ�����Ϣ
u8 recv_mqtt_message(void);

//���Ϳ�ʼ��������
u8 send_start_consume_mesaage(void);

//���Ϳ۷���Ϣ
u8 send_consume_mesaage(void);

//���ͽ����۷���Ϣ
u8 send_finish_consume_mesaage(void);


#endif
