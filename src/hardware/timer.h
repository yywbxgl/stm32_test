#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"



void TIM3_Int_Init(u16 arr,u16 psc);

void TIM7_Int_Init(u16 arr,u16 psc);
void TIM7_SetARR(u16 period);




#endif
