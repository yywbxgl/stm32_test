#ifndef _MESSAGE_INCLUDED_
#define _MESSAGE_INCLUDED_
#include "sys.h"

void create_keep_alive_message(u8 *outbuf, u16 len);


void create_start_consume_message(u8 *outbuf, u16 len);


void create_consume_message(u8 *outbuf, u16 len, u8 finish_flag);


u8 deal_start_consume_response(u8 *outbuf, u16 len);

#endif
