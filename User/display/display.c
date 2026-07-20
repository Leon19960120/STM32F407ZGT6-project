#include "lcd.h"

void display_init(void){

     // 显示字符串
    POINT_COLOR = BLUE;      
    LCD_ShowString(30, 20, 210, 24, 24, (u8*)"STM32F407 HAL");    
    LCD_ShowString(30, 50, 200, 16, 16, (u8*)"TFTLCD TEST");
    LCD_ShowString(30, 70, 200, 16, 16, (u8*)"Makefile + VSCode");
    // 显示固定的标题（只画一次，避免闪烁）
    LCD_ShowString(30, 150, 200, 24, 24, (u8*)"SHT3x Monitor");
    POINT_COLOR = RED;
    LCD_ShowString(30, 180, 100, 16, 16, (u8*)"Temp:       C");
    LCD_ShowString(30, 200, 100, 16, 16, (u8*)"Hum :       %RH");
    // 显示光照强度
    LCD_ShowString(30, 220, 100, 16, 16, (u8*)"Light:     Lux");
    
    //LCD_ShowString(30, 90, 200, 16, 16, lcd_id);       // 显示 LCD ID                           
    //LCD_ShowString(30, 130, 200, 12, 12, (u8*)"2026/07/06");                      // 显示日期 
    //LCD_ShowString(30, 110, 200, 16, 16, (u8*)date_str);//RTC日期
    //LCD_ShowString(30, 130, 200, 16, 16, (u8*)time_str); // 固定显示开机时间       
}

static float s_temp = 0, s_hum = 0;
static uint16_t s_light = 0;


void display_set_data(float temp, float hum, uint16_t light) {
    s_temp = temp;
    s_hum = hum;
    s_light = light;
}


void display_refresh(u8 *str_temp, u8 *str_hum, u8 *str_light)
{
    POINT_COLOR = RED;
    BACK_COLOR = WHITE;

    // 温度 y=180，宽度70，高度16
    LCD_ShowString(90, 180, 70, 16, 16, str_temp);
    // 湿度 y=200，宽度70，高度16
    LCD_ShowString(90, 200, 70, 16, 16, str_hum);
    // 光照 y=220，宽度70，高度16
    LCD_ShowString(70, 220, 70, 16, 16, str_light);
}