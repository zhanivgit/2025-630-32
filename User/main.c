//按键0是关闭蜂鸣器，风扇，LED控制
//按键1是打开低速蜂鸣器，风扇，LED控制，同时发送复杂数据
//按键2是打开中速蜂鸣器，风扇，LED控制，同时发送复杂数据
//按键3是打开高速蜂鸣器，风扇，LED控制 ，同时发送复杂数据
//按键4是每隔一秒自动发送简单数据
//按键5是停止所有发送数据
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
u16 value;
// 全局变量定义
u8 temp = 25;   // 温度初始值设为25度（室温）
u8 humi = 50;   // 湿度初始值设为50%（适中湿度）
uint16_t MyRTC_Time[] = {2025, 7, 2, 8, 35, 0}; // 年、月、日、时、分、秒
uint8_t KeyNum;                         // 按键键码值
uint8_t Hour, Min, Sec;                 // 用于调整时间的变量
uint8_t Flag_Count;                     // 计时标志
uint8_t Flag_Change = 0;                // 0=日期模式, 1=时间模式
uint8_t RxData = 0xFF;                  // 串口接收数据，初始化为无效值
uint8_t BluetoothControl = 0;           // 蓝牙控制标志，0=未控制，1=已控制
uint16_t SendDataTimer = 0;             // 发送数据计时器
uint8_t SendDataFlag = 0;               // 发送数据标志，0=不发送，1=发送
uint16_t MQ2_Value; // 用于存储MQ2数字值

// 新增变量用于优化蓝牙通信
uint8_t LastRxData = 0xFF;              // 上次接收的数据，用于避免重复处理
uint8_t NeedSendFullStatus = 0;         // 是否需要发送完整状态
uint8_t SendDelayCounter = 0;           // 发送延时计数器
uint8_t BluetoothCommDelay = 0;         // 蓝牙通信延时计数器
uint8_t StopAllSending = 0;             // 停止所有数据发送标志，0=允许发送，1=停止发送

// 函数声明
void Bluetooth_Control(void);           // 蓝牙控制
void Temperature_Humidity_Alert(void);  // 传感器检测（控制led,风扇，蜂鸣器）
void Send_TempHumi_Data(void);          // 发送简单数据
void Send_Full_Status(void);            // 发送复杂数据
void Key_Control(void);                 // 按键调整时间的切换
void Process_Bluetooth_Command(void);   // 处理蓝牙命令
void Handle_Serial_Data(void);          // 处理串口数据接收

void Handle_Serial_Data(void) // 改进的串口数据处理
{
    static uint8_t uart_timeout = 0;
    
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET) {
        uint8_t newData = USART_ReceiveData(USART1);
        
        // 清除接收标志位
        USART_ClearFlag(USART1, USART_FLAG_RXNE);
        
        // 只有当接收到新的有效数据时才更新
        if (newData != LastRxData && newData >= 0 && newData <= 5) {
            RxData = newData;
            LastRxData = newData;
            uart_timeout = 0;
            
            // 显示接收到的数据
            OLED_ShowHexNum(3, 16, RxData, 1);
            
            // 设置蓝牙通信延时，避免立即处理
            BluetoothCommDelay = 5; // 延时50ms后处理
        }
    }
    
    // 超时清除，防止数据卡死
    if (uart_timeout++ > 100) {
        uart_timeout = 0;
        LastRxData = 0xFF;
    }
}

void Process_Bluetooth_Command(void) // 处理蓝牙命令（延时处理）
{
    if (BluetoothCommDelay > 0) {
        BluetoothCommDelay--;
        return; // 延时未到，不处理
    }
    
    if (RxData != 0xFF) {
        Bluetooth_Control();
        RxData = 0xFF; // 处理完后清除数据
    }
}

void Bluetooth_Control(void) // 改进的蓝牙控制
{
    static uint8_t last_command = 0xFF;
    
    // 避免重复处理相同命令
    if (RxData == last_command) {
        return;
    }
    last_command = RxData;
    
    if (RxData == 1) {
        // 风扇低速转动
        BluetoothControl = 1;
        
        // 逐步启动设备，避免瞬间功耗过高
        LED1_ON();
        Delay_ms(10);
        BEEP_On();
        Delay_ms(10);
        Fan_SetSpeed(20);
        
        // 延迟发送复杂数据，避免通信冲突
        NeedSendFullStatus = 1;
        SendDelayCounter = 0;

        
    } else if (RxData == 2) {
        // 风扇中速转动
        BluetoothControl = 1;
        
        LED1_ON();
        Delay_ms(10);
        BEEP_On();
        Delay_ms(10);
        Fan_SetSpeed(30);
        
        NeedSendFullStatus = 1;
        SendDelayCounter = 0;

        
    } else if (RxData == 3) {
        // 风扇高速转动
        BluetoothControl = 1;
        
        LED1_ON();
        Delay_ms(10);
        BEEP_On();
        Delay_ms(10);
        Fan_SetSpeed(40);
        
        NeedSendFullStatus = 1;
        SendDelayCounter = 0;
        
        
    } else if (RxData == 0) {
        // 关闭所有设备
        BluetoothControl = 0;
        
        // 逐步关闭设备
        BEEP_Off();
        Delay_ms(10);
        LED1_OFF();
        Delay_ms(10);
        Fan_SetSpeed(0);
        
        // 清除发送状态
        NeedSendFullStatus = 0;
        SendDelayCounter = 0;
        
    } else if (RxData == 4) {
        // 开启温湿度数据发送功能
        StopAllSending = 0;    // 允许发送数据
        SendDataFlag = 1;      // 立即发送一次数据
        SendDataTimer = 0;     // 重置计时器
        
    } else if (RxData == 5) {
        // 关闭所有数据发送功能
        StopAllSending = 1;         // 停止所有数据发送
        SendDataFlag = 0;           // 停止简单数据发送
        SendDataTimer = 0;
    }
}

void Temperature_Humidity_Alert(void) // 传感器检测（控制led,风扇，蜂鸣器）
{
    // 温湿度报警控制 - 仅在未被蓝牙控制时生效
    // 温湿度和MQ2报警控制 - 仅在未被蓝牙控制时生效
    if (temp > 40 || humi > 90) { // 高级报警：温度过高或湿度过高
        if (BluetoothControl == 0) { // 仅在未被蓝牙控制时生效
            BEEP_On();
            LED1_ON();
            Fan_SetSpeed(40); // 风扇高速转动
            OLED_ShowString(3,14,"3"); 
        }
        NeedSendFullStatus = 1;
        SendDelayCounter = 0;
        
    } else if (temp > 35 || humi > 80) { // 中级报警：温度偏高或湿度偏高
        if (BluetoothControl == 0) { // 仅在未被蓝牙控制时生效
            BEEP_On(); // 蜂鸣器持续鸣叫
            LED1_ON(); // LED持续亮
            Fan_SetSpeed(30); // 风扇中速转动
            OLED_ShowString(3,14,"2"); 
        }
        NeedSendFullStatus = 1;
        SendDelayCounter = 0;
        
    } else if (temp > 30 || humi > 70 || MQ2_Value == 1) { // 低级报警：温度略高或湿度略高或MQ2异常
        if (BluetoothControl == 0) { // 仅在未被蓝牙控制时生效
            BEEP_On(); // 蜂鸣器持续鸣叫
            LED1_ON(); // LED持续亮
            Fan_SetSpeed(20); // 风扇低速转动
            OLED_ShowString(3,14,"1");
        }
        NeedSendFullStatus = 1;
        SendDelayCounter = 0;
        
    } else { // 正常范围
        if (BluetoothControl == 0) { // 仅在未被蓝牙控制时生效
            BEEP_Off();
            LED1_OFF();
            Fan_SetSpeed(0); // 风扇停止转动
            
            }
        }
    }

void Send_TempHumi_Data(void) // 发送简单数据
{
    // 检查是否被停止发送
    if (StopAllSending == 1) {
        SendDataFlag = 0;
        SendDataTimer = 0;
        return;
    }
    
    // 每隔一定时间发送一次温湿度数据
    if (SendDataFlag == 1) {
        // 发送数据格式："T:温度值,H:湿度值,Time:时:分:秒\r\n"
        Serial_Printf("T:%d,H:%d,Time:%02d:%02d:%02d\r\n", 
                      temp, humi, 
                      MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
        

        // 重置发送标志
        SendDataFlag = 0;
    }
    
    // 计时器增加
    SendDataTimer++;
    
    // 每5秒发送一次数据
    if (SendDataTimer >= 50) { // 主循环每次约100ms，50次=5秒
        SendDataFlag = 1;
        SendDataTimer = 0;
    }
}

void Send_Full_Status(void) // 发送复杂数据（延迟发送）
{
    // 只有在需要发送且延时到达后才发送
    if (NeedSendFullStatus == 0) {
        return;
    }
    

    
    // 发送格式："Status:T=温度,H=湿度,Fan=风扇状态,Alert=报警状态,Time=时:分:秒\r\n"
    uint8_t fanStatus = 0;
    uint8_t alertStatus = 0;
    
    // 获取风扇状态
    if (BluetoothControl == 1) {
        if (LastRxData == 1) fanStatus = 1;      // 低速
        else if (LastRxData == 2) fanStatus = 2; // 中速
        else if (LastRxData == 3) fanStatus = 3; // 高速
    } else {
        // 根据温湿度报警状态设置风扇状态
        if (temp > 40 || humi > 70) fanStatus = 3;      // 高速
        else if (temp > 35 || humi > 60) fanStatus = 2; // 中速
        else if (temp > 30 || humi > 50) fanStatus = 1; // 低速
    }
    
    // 获取报警状态
    if (temp > 40 || humi > 70) alertStatus = 3;      // 高级报警
    else if (temp > 35 || humi > 60) alertStatus = 2; // 中级报警
    else if (temp > 30 || humi > 50) alertStatus = 1; // 低级报警
    
    // 发送完整状态信息
    Serial_Printf("Status:T=%d,H=%d,Fan=%d,Alert=%d,Time=%02d:%02d:%02d\r\n",
                  temp, humi, fanStatus, alertStatus,
                  MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
    
    // 重置发送标志
    NeedSendFullStatus = 0;
    SendDelayCounter = 0;
}

void Key_Control(void) // 按键调整时间的切换
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
        
        if (KeyNum != 0) { // 有按键按下时才更新RTC
            MyRTC_SetTime(); // 更新RTC时间
            MyRTC_ReadTime(); // 确保在设置时间后立即读取，以更新MyRTC_Time数组
            MyRTC_ReadTime(); // 确保在设置时间后立即读取，以更新MyRTC_Time数组
        }
        
    } else if (Flag_Change == 1) { // 时间模式
        if (KeyNum == 2) { // 调整时钟
            MyRTC_Time[3]++; // 时钟增加
            if (MyRTC_Time[3] > 23) MyRTC_Time[3] = 0; // 时钟范围限制（24小时制）
        } else if (KeyNum == 3) { // 调整分钟
            MyRTC_Time[4]++; // 分钟增加
            if (MyRTC_Time[4] > 59) MyRTC_Time[4] = 0; // 分钟范围限制
        } else if (KeyNum == 4) { // 切换回日期模式
            Flag_Change = 0;
        }
        
        if (KeyNum != 0) { // 有按键按下时才更新RTC
            MyRTC_SetTime(); // 更新RTC时间
            MyRTC_ReadTime(); // 确保在设置时间后立即读取，以更新MyRTC_Time数组
            MyRTC_ReadTime(); // 确保在设置时间后立即读取，以更新MyRTC_Time数组
        }
    }
}

int main(void)
{   
    // 外设初始化
    Serial_Init();   // 蓝牙串口
    OLED_Init();     // OLED显示
    LED_Init();      // LED
    BEEP_Init();     // 蜂鸣器
    DHT11_Init();    // 温湿度传感器
    MyRTC_Init();    // RTC时钟
    Key_Init();      // 按键
    Fan_PWM_Init();  // 风扇PWM
    MQ2_Init();
    FAN_gpio();

    // 初始化延时，确保所有设备稳定
    Delay_ms(1000);
    
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
    OLED_ShowString(4, 1, "MQ2"); // 显示"MQ2"
    OLED_ShowChar(4, 5, ':');
    
    // 主循环计数器
    uint16_t loop_counter = 0;
    
    // 主循环
    while (1)
    {
        // 处理串口数据接收（每次循环都检查）
        Handle_Serial_Data();
        MQ2_Value = MQ2_GetData(); // 获取MQ2数字值
        // 处理蓝牙命令（延时处理）
        Process_Bluetooth_Command();
        
        // 每10个循环（约1秒）执行一次的任务
        if (loop_counter % 10 == 0) {
            // 读取温湿度
            DHT11_Read_Data(&temp, &humi);
            
            // 显示温湿度
            OLED_ShowNum(2, 6, temp, 2);
            OLED_ShowNum(3, 6, humi, 2);
            if (MQ2_Value == 0)
            {
                OLED_ShowString(4, 6, "Normal  "); // 正常，后面加空格清除旧内容
            }
            else
            {
                OLED_ShowString(4, 6, "Abnormal"); // 异常
            }
            
            // 读取并更新时间
            MyRTC_ReadTime();
            
            // 显示日期和时间
            OLED_ShowNum(1, 1, MyRTC_Time[0], 4);  // 年
            OLED_ShowNum(1, 6, MyRTC_Time[1], 2);  // 月
            OLED_ShowNum(1, 9, MyRTC_Time[2], 2);  // 日
            OLED_ShowChar(1, 11, '-');
            OLED_ShowNum(1, 12, MyRTC_Time[3], 2); // 时
            OLED_ShowChar(1, 14, ':');
            OLED_ShowNum(1, 15, MyRTC_Time[4], 2); // 分
            OLED_ShowNum(2, 15, MyRTC_Time[5], 2); // 秒
        }
        
        // 处理按键（每次循环都检查）
        Key_Control();
        
        // 处理温湿度报警
        Temperature_Humidity_Alert();
        
        // 处理温湿度数据发送
        Send_TempHumi_Data();
        
        // 处理复杂状态数据发送（延迟发送）
        Send_Full_Status();
        
        // 循环计数器增加
        loop_counter++;
        if (loop_counter >= 1000) {
            loop_counter = 0;
        }
        
        // 主循环延时约100ms
        Delay_ms(100);
    }
}
