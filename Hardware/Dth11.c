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

u8 DHT11_Check(void) 	   //发送低高
{   
	u8 retry=0;
	DHT11_Mode(IN);//SET INPUT	 
    while (GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)
	{
		retry++;
		Delay_us(1);
	};	 
	if(retry>=100)return 1;
	else retry=0;
    while (!GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)
	{
		retry++;
		Delay_us(1);
	};
	if(retry>=100)return 1;	    
	return 0;
}


u8 DHT11_Read_Bit(void) 			 //读取数据
{
 	u8 retry=0;
	while(GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)//µÈ´ý±äÎªµÍµçÆ½
	{
		retry++;
		Delay_us(1);
	}
	retry=0;
	while(!GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)//µÈ´ý±ä¸ßµçÆ½
	{
		retry++;
		Delay_us(1);
	}
	Delay_us(40);//µÈ´ý40us
	if(GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN))return 1;
	else return 0;		   
}


u8 DHT11_Read_Byte(void)    
{        
	u8 i,dat;
	dat=0;
	for (i=0;i<8;i++) 
	{
		dat<<=1; 
		dat|=DHT11_Read_Bit();
	}						    
	return dat;
}


u8 DHT11_Read_Data(u8 *temp,u8 *humi)    
{        
 	u8 buf[5];
	u8 i;
	DHT11_Rst();
	if(DHT11_Check()==0)
	{
		for(i=0;i<5;i++)//¶ÁÈ¡40Î»Êý¾Ý
		{
			buf[i]=DHT11_Read_Byte();
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

	 
u8 DHT11_Init(void)
{	 
 	GPIO_InitTypeDef  GPIO_InitStructure;	
 	RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE);	 //Ê¹ÄÜPA¶Ë¿ÚÊ±ÖÓ
 	GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN;				 //PG11¶Ë¿ÚÅäÖÃ
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //ÍÆÍìÊä³ö
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);				 //³õÊ¼»¯IO¿Ú
 	GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN);						 //PG11 Êä³ö¸ß
			    
	DHT11_Rst();  //¸´Î»DHT11
	return DHT11_Check();//µÈ´ýDHT11µÄ»ØÓ¦
} 

void DHT11_Mode(u8 mode)
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

