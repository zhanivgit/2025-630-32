#ifndef __FAN_H
#define __FAN_H

#include "stm32f10x.h"

void Fan_PWM_Init(void);
void Fan_SetPWM(uint16_t Compare);

#endif
