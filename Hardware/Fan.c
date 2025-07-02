#include "Fan.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
void FAN_gpio(void)
{	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  // 使能GPIOA时钟
  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;  // 选择引脚PA12和PA13
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // 通用推挽输出模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // 引脚速度50MHz
  GPIO_Init(GPIOA, &GPIO_InitStructure);              // 初始化GPIOA			     
}
void Fan_PWM_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM2);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;		//ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler = 36 - 1;		//PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;		//CCR
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);
	
	TIM_Cmd(TIM2, ENABLE);
}

void Fan_SetPWM(uint16_t Compare)
{
    TIM_SetCompare3(TIM2, Compare);
}

void Fan_SetSpeed(uint16_t speed)
{
    // 根据speed的值控制风扇的开关
    if (speed > 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_12);   // 风扇开启，PA12设置为高电平
        GPIO_ResetBits(GPIOA, GPIO_Pin_13); // PA13设置为低电平
    }
    else
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_12); // 风扇关闭，PA12设置为低电平
        GPIO_SetBits(GPIOA, GPIO_Pin_13);   // PA13设置为高电平
    }
	TIM_SetCompare3(TIM2, speed);
}
    
