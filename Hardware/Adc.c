#include "Adc.h"

/**
  * @brief  ADC��ʼ������
  * @param  
  * @retval 
  */
void ADCx_Init(void)
{
	//����ADCʱ��
	RCC_APB2PeriphClockCmd(ADC_CLK, ENABLE);
	
	//ADCƵ�ʽ���6��Ƶ
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	//����ADC�ṹ��
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//����ģʽ
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//�����Ҷ���
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//�������
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//����ת��
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//��ɨ��ģʽ
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//��ͨ����
	ADC_Init(ADCx, &ADC_InitStructure);	//��ʼ��ADC1
	
	//����ADCx
	ADC_Cmd(ADCx, ENABLE);
	
	//����ADCУ׼
	ADC_ResetCalibration(ADCx);
	while(ADC_GetResetCalibrationStatus(ADCx) == SET);
	ADC_StartCalibration(ADCx);
	while(ADC_GetCalibrationStatus(ADCx) == SET);	
}

/**
  * @brief  ��ȡADCת���������
  * @param  ADC_Channel 	ѡ����Ҫ�ɼ���ADCͨ��
  * @param  ADC_SampleTime  ѡ����Ҫ����ʱ��
  * @retval ����ת�����ģ���ź���ֵ
  */
u16 ADC_GetValue(uint8_t ADC_Channel,uint8_t ADC_SampleTime)
{
	//����ADCͨ��
	ADC_RegularChannelConfig(ADCx, ADC_Channel, 1, ADC_SampleTime);
	
	ADC_SoftwareStartConvCmd(ADCx, ENABLE); //�������ADCת��
	while(ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC) == RESET); //��ȡADCת����ɱ�־λ
	return ADC_GetConversionValue(ADCx);
}
