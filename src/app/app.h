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

//���Ӱ����Ʒ�����
u8 connect_to_server(void); 

//����mqtt����
u8 subscribe_mqtt(void);

//���ͱ�����Ϣ
u8 send_keep_alive_mesaage(void);


#endif
