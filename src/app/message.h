#ifndef _MESSAGE_INCLUDED_
#define _MESSAGE_INCLUDED_
#include "sys.h"


//����ָ��1����ʱ������Ϣ
void create_keep_alive_message(u8 *outbuf, u16 len);

//����ָ��2����ʼˢ����������
void create_start_consume_message(u8 *outbuf, u16 len);

//����ָ��3��4����ʱ�ϴ��۷���Ϣ, 
//ic_flag��־ 1-IC�����ѣ� 2-app����
//finish_flag��־ 1-�������ѣ� 2-��������
void create_consume_message(u8 *outbuf, u16 len, u8 ic_flag, u8 finish_flag);

//����ָ��7����ʱ�ϴ�app���ѿ۷���Ϣ
void create_app_consume_message(u8 *outbuf, u16 len);


//����ָ��6��app����������Ӧ,ok_flagΪ1ʱ����code�ɹ�
void create_app_consume_response(u8 *outbuf, u16 len, u8 ok_flag);

//�����յ��ķ�������Ϣ��Э�鹫�����֣�
//����0��ʾʧ�ܣ���������ֵ��ʾ�ɹ�������ֵΪtrade�����
u8 parse_service_message_common(u8 *outbuf, u16 len);


//���������ָ��2����ʼˢ�������������Ӧ
u8 deal_start_consume_response(u8 *outbuf, u16 len);


//���������ָ��6����ʼapp��������
u8 deal_start_app_consume(u8 *outbuf, u16 len);


#endif
