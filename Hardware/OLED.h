#ifndef __OLED_H
#define __OLED_H

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowFloat(uint8_t Line, uint8_t Column, float FloatNum, uint8_t IntLength, uint8_t DecLength);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowChinese(uint8_t Line, uint8_t Column, uint8_t num);
void OLED_ShowChinese1(u8 x,u8 y,u8 num,u8 size1,u8 mode);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
#endif
