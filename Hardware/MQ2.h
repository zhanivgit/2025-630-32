#ifndef __MQ2_H
#define	__MQ2_H
#include "stm32f10x.h"
#include "Adc.h"
#include "delay.h"
#include "math.h"



#define MQ2_READ_TIMES	10  //MQ-2´«¸ÐÆ÷ADCÑ­»·¶ÁÈ¡´ÎÊý

//Ä£Ê½Ñ¡Ôñ	
//Ä£ÄâAO:	1
//Êý×ÖDO:	0
#define	MODE 	1

/***************¸ù¾Ý×Ô¼ºÐèÇó¸ü¸Ä****************/
// MQ-2 GPIOºê¶¨Òå
#if MODE
#define		MQ2_AO_GPIO_CLK								RCC_APB2Periph_GPIOA
#define 	MQ2_AO_GPIO_PORT							GPIOA
#define 	MQ2_AO_GPIO_PIN								GPIO_Pin_0
#define   ADC_CHANNEL               		ADC_Channel_0	// ADC Í¨µÀºê¶¨Òå

#else
#define		MQ2_DO_GPIO_CLK								RCC_APB2Periph_GPIOA
#define 	MQ2_DO_GPIO_PORT							GPIOA
#define 	MQ2_DO_GPIO_PIN								GPIO_Pin_1			

#endif
/*********************END**********************/


void MQ2_Init(void);
uint16_t MQ2_GetData(void);
float MQ2_GetData_PPM(void);

#endif /* __ADC_H */

