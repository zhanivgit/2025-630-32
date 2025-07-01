#include "Beep.h"
#include "stm32f10x.h"                  // Device header
#include <stdint.h>  // 包含stdint.h头文件以定义uint8_t
void BEEP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;                
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       
    GPIO_Init(GPIOA, &GPIO_InitStructure);                  
	
    GPIO_SetBits(GPIOA, GPIO_Pin_6); // 初始化为高电平，蜂鸣器关闭（假设低电平触发）
}

void BEEP_On(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_6); // 低电平触发蜂鸣器
}

void BEEP_Off(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_6); // 高电平关闭蜂鸣器
}
