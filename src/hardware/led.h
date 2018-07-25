#ifndef __LED_H
#define __LED_H
#include "sys.h"

#define  LED0  PBout(5)// PB5
#define  LED1  PEout(5)// PE5	


#define  LED0_ON    LED0=0
#define  LED0_OFF   LED0=1

#define  LED1_ON    LED1=0
#define  LED1_OFF   LED1=1

void LED_Init(void);//≥ı ºªØ


#endif
