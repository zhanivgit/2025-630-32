#include "mq2.h"
#include "stm32f10x.h"  
void MQ2_Init(void)
{
	#if MODE
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd (MQ2_AO_GPIO_CLK, ENABLE );	// ´ò¿ª ADC IO¶Ë¿ÚÊ±ÖÓ
		GPIO_InitStructure.GPIO_Pin = MQ2_AO_GPIO_PIN;					// ÅäÖÃ ADC IO Òý½ÅÄ£Ê½
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		// ÉèÖÃÎªÄ£ÄâÊäÈë
		
		GPIO_Init(MQ2_AO_GPIO_PORT, &GPIO_InitStructure);				// ³õÊ¼»¯ ADC IO

		ADCx_Init();
	}
	#else
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd (MQ2_DO_GPIO_CLK, ENABLE );	// ´ò¿ªÁ¬½Ó ´«¸ÐÆ÷DO µÄµ¥Æ¬»úÒý½Å¶Ë¿ÚÊ±ÖÓ
		GPIO_InitStructure.GPIO_Pin = MQ2_DO_GPIO_PIN;			// ÅäÖÃÁ¬½Ó ´«¸ÐÆ÷DO µÄµ¥Æ¬»úÒý½ÅÄ£Ê½
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			// ÉèÖÃÎªÉÏÀ­ÊäÈë
		
		GPIO_Init(MQ2_DO_GPIO_PORT, &GPIO_InitStructure);				// ³õÊ¼»¯ 
		
	}
	#endif
	
}

#if MODE
uint16_t MQ2_ADC_Read(void)
{
	//ÉèÖÃÖ¸¶¨ADCµÄ¹æÔò×éÍ¨µÀ£¬²ÉÑùÊ±¼ä
	return ADC_GetValue(ADC_CHANNEL, ADC_SampleTime_55Cycles5);
}
#endif

uint16_t MQ2_GetData(void)
{
	
	#if MODE
	uint32_t  tempData = 0;
	for (uint8_t i = 0; i < MQ2_READ_TIMES; i++)
	{
		tempData += MQ2_ADC_Read();
		Delay_ms(5);
	}

	tempData /= MQ2_READ_TIMES;
	return tempData;
	
	#else
	uint16_t tempData;
	tempData = !GPIO_ReadInputDataBit(MQ2_DO_GPIO_PORT, MQ2_DO_GPIO_PIN);
	return tempData;
	#endif
}


float MQ2_GetData_PPM(void)
{
	#if MODE
	float  tempData = 0;
	

	for (uint8_t i = 0; i < MQ2_READ_TIMES; i++)
	{
		tempData += MQ2_ADC_Read();
		Delay_ms(5);
	}
	tempData /= MQ2_READ_TIMES;
	
	float Vol = (tempData*5/4096);
	float RS = (5-Vol)/(Vol*0.5);
	float R0=6.64;
	
	float ppm = pow(11.5428*R0/RS, 0.6549f);
	
	return ppm;
	#endif
}
