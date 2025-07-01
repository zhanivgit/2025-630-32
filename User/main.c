//按键0是关闭蜂鸣器，风扇，LED控制
//按键1是打开低速蜂鸣器，风扇，LED控制，同时发送复杂数据
//按键2是打开中速蜂鸣器，风扇，LED控制，同时发送复杂数据
//按键3是打开高速蜂鸣器，风扇，LED控制 ，同时发送复杂数据
//按键1是打开低速蜂鸣器，风扇，LED控制
//按键2是打开中速蜂鸣器，风扇，LED控制
//按键3是打开高速蜂鸣器，风扇，LED控制
//按键4是每隔一秒自动发送简单数据
//按键5是停止所有发送数据
//按键6是每秒发送复杂数据，每隔5s发送一次简单数据(有问题)
//按键7是每隔5s发送一次简单数据
//按键8是发送一次复杂数据后,又开始每隔5S发一次简单数据(有问题)
//B0是选择调节  月日  还是  时分
//B1是调节    日和分
//B10是调节   月和时
#include "stm32f10x.h"                  //头文件   
#include "Delay.h"
#include "OLED.h"
#include "led.h"
#include "Key.h"
#include "Beep.h"
#include "MYRTC.h"
#include "MQ2.h"
#include "Fan.h"
#include "Dth11.h"
#include "Serial.h"
// 全局变量定义
u8 temp = 25;   // 温度初始值设为25度（室温）
u8 humi = 50;   // 湿度初始值设为50%（适中湿度）
uint16_t MyRTC_Time[] = {2025, 7, 2, 8, 35, 0}; // 年、月、日、时、分、秒
uint8_t KeyNum;                         // 按键键码值
uint8_t Hour, Min, Sec;                 // 用于调整时间的变量
uint8_t Flag_Count;                     // 计时标志
uint8_t Flag_Change = 0;                // 0=日期模式, 1=时间模式
uint8_t RxData;                         // 串口接收数据
uint8_t BluetoothControl = 0;           // 蓝牙控制标志，0=未控制，1=已控制
uint16_t SendDataTimer = 0;             // 发送数据计时器
uint8_t SendDataFlag = 0;               // 发送数据标志，0=不发送，1=发送

// 函数声明

void Bluetooth_Control(void);  //蓝牙控制
void Temperature_Humidity_Alert(void);    //传感器检测（控制led,风扇，蜂鸣器）
void Send_TempHumi_Data(void);//发送简单数据
void Send_Full_Status(void);//发送复杂数据
void Key_Control(void);//按键调整时间的切换

void Bluetooth_Control(void)//蓝牙控制
{
    if (RxData == 1) {
        // 风扇低速转动
        BluetoothControl = 1;
        Fan_SetPWM(20);
        LED1_ON();
        BEEP_On();
        OLED_ShowString(4, 1, "Fan: Low Speed ");
    } else if (RxData == 2) {
        // 风扇中速转动
        BluetoothControl = 1;
        Fan_SetPWM(50);
        LED1_ON();
        BEEP_On();
        OLED_ShowString(4, 1, "Fan: Mid Speed ");
    } else if (RxData == 3) {
        // 风扇高速转动
        BluetoothControl = 1;
        Fan_SetPWM(80);
        LED1_ON();
        BEEP_On();
        OLED_ShowString(4, 1, "Fan: High Speed");
    } else if (RxData == 0) {
        // 关闭所有设备
        BluetoothControl = 0;
        Fan_SetPWM(0);
        LED1_OFF();
        BEEP_Off();
    } else if (RxData == 4) {
        // 开启温湿度数据发送功能
        SendDataFlag = 1;  // 立即发送一次数据
        SendDataTimer = 0; // 重置计时器
        OLED_ShowString(4, 12, "Auto"); // 显示自动发送状态
    } else if (RxData == 5) {
        // 关闭温湿度数据发送功能
        SendDataFlag = 0;
        SendDataTimer = 0;
        OLED_ShowString(4, 12, "    "); // 清除发送状态显示
    } 
}

void Temperature_Humidity_Alert(void)//传感器检测（控制led,风扇，蜂鸣器）
{
    // 温湿度报警控制 - 仅在未被蓝牙控制时生效
    if (BluetoothControl == 0) {
        if (temp > 50 || humi > 90) { // 高级报警：温度过高或湿度过高
            BEEP_On();
            LED1_ON();
            Fan_SetPWM(80); // 风扇高速转动
            Send_Full_Status();
            OLED_ShowString(4, 1, "Temp Alert: High");
        } else if (temp > 40 || humi > 75) { // 中级报警：温度偏高或湿度偏高
            BEEP_On(); // 蜂鸣器持续鸣叫
            LED1_ON(); // LED持续亮
            Fan_SetPWM(50); // 风扇中速转动
            Send_Full_Status();
            OLED_ShowString(4, 1, "Temp Alert: Mid ");
        } else if (temp > 30 || humi > 70) { // 低级报警：温度略高或湿度略高
            BEEP_On(); // 蜂鸣器持续鸣叫
            LED1_ON(); // LED持续亮
            Send_Full_Status();
            Fan_SetPWM(20); // 风扇低速转动
            OLED_ShowString(4, 1, "Temp Alert: Low ");
        } else { // 正常范围
            BEEP_Off();
            LED1_OFF();
            Fan_SetPWM(0); // 风扇停止转动
            if (Flag_Change == 0) {
                OLED_ShowString(4, 1, "Change Date     ");
            } else {
                OLED_ShowString(4, 1, "Change Time     ");
            }
        }
    }
}

void Send_TempHumi_Data(void)//发送简单数据
{
    // 每隔一定时间发送一次温湿度数据
    if (SendDataFlag == 1) {
        // 发送数据格式："T:温度值,H:湿度值,Time:时:分:秒\r\n"
        Serial_Printf("T:%d,H:%d,Time:%02d:%02d:%02d\r\n", 
                      temp, humi, 
                      MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
        
        // 在OLED上显示发送状态
        OLED_ShowString(4, 12, "Send");
        
        // 重置发送标志
        SendDataFlag = 0;
    }
    
    // 计时器增加
    SendDataTimer++;
    
    // 每5秒发送一次数据（假设主循环中每次延时约1秒）
    if (SendDataTimer >= 5) {
        SendDataFlag = 1;
        SendDataTimer = 0;
    }
}

void Send_Full_Status(void)//发送复杂数据
{
    // 发送格式："Status:T=温度,H=湿度,Fan=风扇状态,Alert=报警状态,Time=时:分:秒\r\n"
    uint8_t fanStatus = 0;
    uint8_t alertStatus = 0;
    
    // 获取风扇状态
    if (BluetoothControl == 1) {
        if (RxData == 1) fanStatus = 1;      // 低速
        else if (RxData == 2) fanStatus = 2; // 中速
        else if (RxData == 3) fanStatus = 3; // 高速
    } else {
        // 根据温湿度报警状态设置风扇状态
        if (temp > 50 || humi > 90) fanStatus = 3;      // 高速
        else if (temp > 40 || humi > 75) fanStatus = 2; // 中速
        else if (temp > 30 || humi > 70) fanStatus = 1; // 低速
    }
    
    // 获取报警状态
    if (temp > 50 || humi > 90) alertStatus = 3;      // 高级报警
    else if (temp > 40 || humi > 75) alertStatus = 2; // 中级报警
    else if (temp > 30 || humi > 70) alertStatus = 1; // 低级报警
    
    // 发送完整状态信息
    Serial_Printf("Status:T=%d,H=%d,Fan=%d,Alert=%d,Time=%02d:%02d:%02d\r\n",
                  temp, humi, fanStatus, alertStatus,
                  MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
}

void Key_Control(void)//按键调整时间的切换
{
    KeyNum = Key_GetNum();  // 读取按键键码
    
    if (Flag_Change == 0) {    // 日期模式
        if (KeyNum == 1) {     // 调整年
            MyRTC_Time[0]++;  // 年增加
            if (MyRTC_Time[0] > 2100) MyRTC_Time[0] = 2000; // 年份范围限制
        } else if (KeyNum == 2) { // 调整月
            MyRTC_Time[1]++; // 月增加
            if (MyRTC_Time[1] > 12) MyRTC_Time[1] = 1; // 月份范围限制
        } else if (KeyNum == 3) { // 调整日
            MyRTC_Time[2]++; // 日增加
            if (MyRTC_Time[2] > 31) MyRTC_Time[2] = 1; // 日期范围限制
        } else if (KeyNum == 4) { // 切换到时间模式
            Flag_Change = 1;
        }
        
        MyRTC_SetTime(); // 更新RTC时间
        MyRTC_ReadTime(); // 确保在设置时间后立即读取，以更新MyRTC_Time数组
        MyRTC_ReadTime(); // 确保在设置时间后立即读取，以更新MyRTC_Time数组
    } else if (Flag_Change == 1) { // 时间模式
         if (KeyNum == 2) { // 调整时钟
            MyRTC_Time[3]++; // 时钟增加
            if (MyRTC_Time[3] > 59) MyRTC_Time[3] = 0; // 时钟范围限制
        } else if (KeyNum == 3) { // 调整分钟
            MyRTC_Time[4]++; // 分钟增加
            if (MyRTC_Time[4] > 59) MyRTC_Time[4] = 0; // 分钟范围限制
        } else if (KeyNum == 4) { // 切换回日期模式
            Flag_Change = 0;
        }
        
        MyRTC_SetTime(); // 更新RTC时间
        MyRTC_ReadTime(); // 确保在设置时间后立即读取，以更新MyRTC_Time数组
        MyRTC_ReadTime(); // 确保在设置时间后立即读取，以更新MyRTC_Time数组
    }
}

int main(void)
{   // 外设初始化
    Serial_Init();//蓝牙
    OLED_Init();
    LED_Init();      
    BEEP_Init();//蜂鸣器
    DHT11_Init();//温湿度
    MyRTC_Init();//RTC时钟
    Key_Init();//按键
    Fan_PWM_Init();//风扇
    // 显示初始化界面
    OLED_ShowString(1, 1, "XXXX-XX-XX");
    OLED_ShowChinese(2, 1, 0); // 温
    OLED_ShowChinese(2, 2, 1); // 度
    OLED_ShowChar(2, 5, ':');
    OLED_ShowChar(2, 8, 'C');	
    OLED_ShowChinese(3, 1, 2); // 湿
    OLED_ShowChinese(3, 2, 1); // 度
    OLED_ShowChar(3, 5, ':');	
    OLED_ShowChar(3, 8, '%');
    
    // 主循环
    while (1)
    {
        // 读取温湿度
        DHT11_Read_Data(&temp, &humi);
        Delay_ms(1000);   //必须要1000，不然有问题
        
        // 显示温湿度
        OLED_ShowNum(2, 6, temp, 2);
        OLED_ShowNum(3, 6, humi, 2);
        
        // 读取并更新时间
        MyRTC_ReadTime();
        
        // 处理按键
        Key_Control();
        
        // 显示日期和时间
        OLED_ShowNum(1, 1, MyRTC_Time[0], 4);  // 年
        OLED_ShowNum(1, 6, MyRTC_Time[1], 2);  // 月
        OLED_ShowNum(1, 9, MyRTC_Time[2], 2);  // 日
        OLED_ShowChar(1, 11, '-');
        OLED_ShowNum(1, 12, MyRTC_Time[3], 2); // 时
        OLED_ShowChar(1, 14, ':');
        OLED_ShowNum(1, 15, MyRTC_Time[4], 2); // 分
        OLED_ShowNum(2, 15, MyRTC_Time[5], 2); // 秒
        
        // 检查是否有串口数据接收
        if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET) {
            RxData = USART_ReceiveData(USART1);
            OLED_ShowHexNum(3, 13, RxData, 1);
        }
             
        // 处理蓝牙控制
        Bluetooth_Control();
        
        // 处理温湿度报警
        Temperature_Humidity_Alert();
        
        // 处理温湿度数据发送
        Send_TempHumi_Data();
    }
}
