#include "stm32f10x.h"                  // Device header
#include  "Dth11.h"
#include  "Delay.h"
//数据#include "dht11.h"
#include "delay.h"
      
void DHT11_Rst(void) //复位	   
{                 
	DHT11_Mode(OUT); 	
	DHT11_Low; 	     
	Delay_ms(20);    	
	DHT11_High; 			//DQ=1 
	Delay_us(13);     	
}

u8 DHT11_Check(void) 	     //检验
{   
	u8 retry=0;
	DHT11_Mode(IN);//SET INPUT	 
    while (GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)      //等待DHT11拉低40us
	{
		retry++;
		Delay_us(1);
	};	 
	if(retry>=100)return 1;
	else retry=0;
    while (!GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)       //等待DHT11拉高80us
	{
		retry++;
		Delay_us(1);
	};
	if(retry>=100)return 1;	    
	return 0;// 如果两个等待都成功，说明DHT11响应正常，返回0表示成功
}

//读取一个位
u8 DHT11_Read_Bit(void) 	//读取高低电平，先拉低后拉高，如果高电平23-27就表示0，高电平68-74就表示1
{
 	u8 retry=0;
	while(GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)
	{
		retry++;
		Delay_us(1);
	}
	retry=0;
	while(!GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)
	{
		retry++;
		Delay_us(1);
	}
	Delay_us(40);
	if(GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN))return 1;
	else return 0;		   
}


u8 DHT11_Read_Byte(void)    //读取1字节
{        
	u8 i,dat;
	dat=0;
	for (i=0;i<8;i++) 
	{
		dat<<=1; 
		dat|=DHT11_Read_Bit(); //调用8次DHT11_Read_Bit()函数，每次读取一个位
	}						    
	return dat;
}


u8 DHT11_Read_Data(u8 *temp,u8 *humi)   //读取一次温湿度 
{        
 	u8 buf[5];
	u8 i;
	DHT11_Rst();
	if(DHT11_Check()==0)    //校验成功
	{
		for(i=0;i<5;i++)     
		{
			buf[i]=DHT11_Read_Byte();   //调用5次DHT11_Read_Byte()函数，每次读取一个字节
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
		{
			*humi=buf[0];
			*temp=buf[2];
		}
	}
	else return 1;
	return 0;	    
}

	 
u8 DHT11_Init(void)   //初始化
{	 
 	GPIO_InitTypeDef  GPIO_InitStructure;	
 	RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE);	 
 	GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN;				
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);				 
 	GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN);						
			    
	DHT11_Rst();  
	return DHT11_Check();  //等待DHT11的回应
} 

void DHT11_Mode(u8 mode)   //设置DTH11为输入或输出模式
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	if(mode)
	{
		GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	}
	else
	{
		GPIO_InitStructure.GPIO_Pin =  DHT11_GPIO_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	}
	GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}

