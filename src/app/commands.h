#ifndef _COMMANDS_INCLUDED_
#define _COMMANDS_INCLUDED_
#include "sys.h"

void create_keep_alive_message(u8 *outbuf, u16 len);



extern u8 g_disable;
extern u8 g_abnormal ;
extern u8 g_heart ;
extern u8 g_offLine ;
extern char g_pwd[64] ;
extern u8 g_chargRate ;
extern u8 g_moneyRate;
extern u8 g_logRate ;

extern char g_card_id[12];
extern u16 g_ICCard_Value;


#endif
