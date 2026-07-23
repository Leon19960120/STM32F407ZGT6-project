#include "lcd.h"
#include "display.h"
#include <stdio.h>

void display_init(void){
    char lcd_id[12];
    /* LCD 的初始化交给显示模块 */
    LCD_Init();
    /* lcddev.id 是 LCD_Init() 读取出来的 */
    (void)snprintf(
        lcd_id,
        sizeof(lcd_id),
        "LCD ID:%04X",
        (unsigned int)lcddev.id
    );
     // 显示字符串
    // 显示固定的标题（只画一次，避免闪烁）
    POINT_COLOR = BLUE;      
    LCD_ShowString(30, 20, 210, 24, 24, (u8*)"STM32F407ZGT6 HAL");    
    LCD_ShowString(30, 50, 200, 16, 16, (u8*)"TFTLCD SHOW");
    LCD_ShowString(30, 70, 200, 16, 16, (u8*)"Makefile + VSCode");
    /* 放在 y=90，避免与 y=20 的标题重叠 */
    POINT_COLOR = RED;
    LCD_ShowString(30, 90, 200, 16, 16, (u8 *)lcd_id);
    LCD_ShowString(30, 150, 200, 24, 24, (u8*)"SHT3x Monitor");
    POINT_COLOR = RED;
    LCD_ShowString(30, 180, 100, 16, 16, (u8*)"Temp:       C");
    LCD_ShowString(30, 200, 100, 16, 16, (u8*)"Hum :       %RH");
    // 显示光照强度
    LCD_ShowString(30, 220, 100, 16, 16, (u8*)"Light:      Lux");
    
}



void display_refresh(const char *str_temp, const char  *str_hum, const char *str_light)
{
    POINT_COLOR = RED;
    BACK_COLOR = WHITE;  
    // ============================================================
    // 2. 局部刷新显示数据 (使用白色背景覆盖旧数据，防止残影)    
    // 温度 y=180，宽度70，高度16
    LCD_ShowString(85, 180, 70, 16, 16, (uint8_t*)str_temp);
    // 湿度 y=200，宽度70，高度16
    LCD_ShowString(85, 200, 70, 16, 16, (uint8_t*)str_hum);
    // 光照 y=220，宽度70，高度16
    LCD_ShowString(85, 220, 70, 16, 16, (uint8_t*)str_light);
}

void display_time(char* date_str, char* time_str){
    POINT_COLOR = RED;
    BACK_COLOR = WHITE;
    LCD_ShowString(30, 130, 200, 16, 16, (u8*)time_str); 
    //LCD_ShowString(30, 130, 200, 12, 12, (u8*)"2026/07/06");                      // 显示日期 
    LCD_ShowString(30, 110, 200, 16, 16, (u8*)date_str);//RTC日期
}   