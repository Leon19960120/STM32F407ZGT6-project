/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "SHT3x.h"  //引入温湿度传感器
#include "SY30.h"   //光照传感器
#include "lcd.h"    //TFT LCD屏幕
#include "W5500.h"  //w5500
#include "W5500_USER.h" //
#include "wiz_platform.h"
#include "wizchip_conf.h"
#include "wiz_interface.h"
#include "do_mqtt.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
    u8 lcd_id[12]; // 存放LCD ID字符串
    float hum = 0.0f;       // 用于存储湿度值
    float temp = 0.0f;      // 用于存储温度值
    uint16_t light = 0;     //光照强度

    char str_temp[32];      // 用于格式化温度字符串的缓冲区
    char str_hum[32];       // 用于格式化湿度字符串的缓冲区 
    char str_light[32]; 
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FSMC_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  
  // 1. 初始化 LCD
  LCD_Init();           
    
  // 2. 设置画笔颜色
  POINT_COLOR = RED;      

  // 3. 将 LCD ID 格式化到字符串数组
  sprintf((char *)lcd_id, "LCD ID:%04X", lcddev.id);
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  char date_str[20];
  char time_str[20];

  // 开机只读取一次
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

  sprintf(date_str, "20%02d/%02d/%02d", sDate.Year, sDate.Month, sDate.Date);
  sprintf(time_str, "%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);

  // 显示字符串
  POINT_COLOR = BLUE;      
  LCD_ShowString(30, 20, 210, 24, 24, (u8*)"STM32F407 HAL");    
  LCD_ShowString(30, 50, 200, 16, 16, (u8*)"TFTLCD TEST");
  LCD_ShowString(30, 70, 200, 16, 16, (u8*)"Makefile + VSCode");
  LCD_ShowString(30, 90, 200, 16, 16, lcd_id);       // 显示 LCD ID                           
  //LCD_ShowString(30, 130, 200, 12, 12, (u8*)"2026/07/06");                      // 显示日期 
  LCD_ShowString(30, 110, 200, 16, 16, (u8*)date_str);//RTC日期
  LCD_ShowString(30, 130, 200, 16, 16, (u8*)time_str); // 固定显示开机时间
  // 显示固定的标题（只画一次，避免闪烁）
  POINT_COLOR = BLUE;
  LCD_ShowString(30, 150, 200, 24, 24, (u8*)"SHT3x Monitor");

  POINT_COLOR = RED;
  LCD_ShowString(30, 180, 100, 16, 16, (u8*)"Temp:       C");
  LCD_ShowString(30, 200, 100, 16, 16, (u8*)"Hum :       %RH");
  // 显示光照强度
  
  LCD_ShowString(30, 220, 100, 16, 16, (u8*)"Light:     Lux");       
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    
        // 变量递增
        // x++;
        // if (x == 12) x = 0;
        
        // 延时 50ms
        HAL_Delay(50);  
         // 1. 读取传感器数据
    if (ReadSHT3x(&hum, &temp)) 
    {
        // ================= 核心避坑：浮点数转字符串 =================
        // 因为 GCC 的 nano.specs 默认不支持 %f，我们必须手动拆分整数和小数部分
        
        // 处理温度
        int t_int = (int)temp;                  // 获取整数部分 (例如 25)
        int t_dec = (int)((temp - t_int) * 100); // 获取小数部分 (例如 67)
        if (t_dec < 0) t_dec = -t_dec;          // 防止负数温度导致负号重复 (如 -5.-20)
        sprintf(str_temp, "%d.%02d", t_int, t_dec); // 拼接成 "25.67"

        // 处理湿度
        int h_int = (int)hum;
        int h_dec = (int)((hum - h_int) * 100);
        if (h_dec < 0) h_dec = -h_dec;
        sprintf(str_hum, "%d.%02d", h_int, h_dec);
        // ============================================================

        // 2. 局部刷新显示数据 (使用白色背景覆盖旧数据，防止残影)
        POINT_COLOR = RED;  // 字体颜色
        BACK_COLOR = WHITE;   // 背景颜色
        
        // 显示温度 (x=60, y=60, 宽度60, 高度16, 字体16)
        LCD_ShowString(80, 180, 70, 16, 16, (u8*)str_temp); 
        // 显示湿度 (x=60, y=90, 宽度60, 高度16, 字体16)
        LCD_ShowString(80, 200, 70, 16, 16, (u8*)str_hum);  
    } 
    else 
    {
        // 如果读取失败，显示错误提示
        POINT_COLOR = RED;
        LCD_ShowString(60, 180, 70, 16, 16, (u8*)"Error");
        LCD_ShowString(60, 200, 70, 16, 16, (u8*)"Error");
    }
    
  // ================== 2. 读取并显示光照度 (SY30/BH1750) ==================
    light = GY30_GetData();
    
    // 格式化为 5 位整数字符串（例如 "  150" 或 "00150"）
    sprintf(str_light, "%5d", light);
    POINT_COLOR = RED;
    BACK_COLOR = WHITE;
    // 在 x=80, y=220 处显示光照数值
    LCD_ShowString(50, 220, 70, 16, 16, (u8*)str_light);
    // 翻转 LED 表示程序在运行
    HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9); // 请根据你的实际 LED 引脚修改！
    // 延时 1 秒 (SHT3x 单次测量不需要太频繁)
    HAL_Delay(1000);
       
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
