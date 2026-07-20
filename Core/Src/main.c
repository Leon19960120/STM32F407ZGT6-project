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
  #include <unistd.h>  // _read 和 _write 必须包含此头文件
  #include <errno.h>
  #include "delay.h" //延迟函数
  #include <stdio.h>
  #include "SHT3x.h"  //引入温湿度传感器
  #include "SY30.h"   //光照传感器
  #include "lcd.h"    //TFT LCD屏幕
  #include "W5500.h"  //w5500
  #include "wiz_platform.h"
  #include "wizchip_conf.h"
  #include "wiz_interface.h"
  #include "do_mqtt.h"
  #include "display.h"
  

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
   extern UART_HandleTypeDef huart1; // 确保声明了 huart1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

  #define SOCKET_ID 0
  #define ETHERNET_BUF_MAX_SIZE (1024 * 2)

   float hum = 0.0f;       // 用于存储湿度值
  float temp = 0.0f;      // 用于存储温度值
      uint16_t light = 0;     //光照强度

  /* network information */
  wiz_NetInfo default_net_info = {
      .mac = {0x00, 0x08, 0xdc, 0x12, 0x22, 0x12},
      .ip  = {192, 168, 1, 30},
      .gw  = {192, 168, 1, 1},
      .sn  = {255, 255, 255, 0},
      .dns = {8, 8, 8, 8},
      .dhcp = NETINFO_DHCP
  };

  uint8_t ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0};
  static uint8_t mqtt_send_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0};
  static uint8_t mqtt_recv_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

  /**
    * @brief  重定向 _read (用于 scanf / getchar，适配 newlib-nano)
    */
  int _read(int fd, char *ptr, int len)
  {
      // fd == 0 代表 stdin (标准输入)
      if (fd == 0) 
      {
          // 阻塞接收 1 个字符 (与原来的 fgetc 逻辑一致)
          HAL_UART_Receive(&huart1, (uint8_t *)ptr, 1, HAL_MAX_DELAY);
          return 1; // 返回成功读取的字节数
      }
      
      errno = EBADF;
      return -1;
  }

/**
  * @brief  重定向 _write (用于 printf，适配 newlib-nano)
  */
int _write(int fd, char *ptr, int len)
{
    // fd == 1 代表 stdout, fd == 2 代表 stderr
    if (fd == 1 || fd == 2) 
    {
        // 批量发送，效率极高
        HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
        return len;
    }
    
    errno = EBADF;
    return -1;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
      u8 lcd_id[12]; // 存放LCD ID字符串
     
      char str_temp[32];      // 用于格式化温度字符串的缓冲区
      char str_hum[32];       // 用于格式化湿度字符串的缓冲区 
      char str_light[32]; 
      uint32_t last_sensor_time = 0;
      uint32_t last_rtc_time = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  // 4. 【关键修正】DWT 延时初始化，必须放在 SystemClock_Config() 之后！
  // 此时 SystemCoreClock 已经是准确的 168,000,000
  Delay_Init();
    
    
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
  // 可选：打印验证一下，确保时钟配置正确
  printf("System Core Clock: %u Hz\r\n", SystemCoreClock); 

  uint8_t test[] = "Hello USART1\r\n";
  HAL_UART_Transmit(&huart1, test, sizeof(test), 100);

  
  HAL_Delay(10);
  printf("%s MQTT OneNET example\r\n",_WIZCHIP_ID_);
  
  /* wizchip init */
  wizchip_initialize();
  uint8_t version = getVERSIONR(); // 读取 W5500 版本寄存器 (固定为 0x04)
  printf("[W5500] VERSIONR = 0x%02X\r\n", version);
  
  network_init(ethernet_buf, &default_net_info);

  mqtt_init(SOCKET_ID, mqtt_send_ethernet_buf, mqtt_recv_ethernet_buf);

  // 1. 检查 SPI 硬件是否连通
    //uint8_t version = getVERSIONR(); 
    // printf("[1] W5500 Version Register: 0x%02X\r\n", version);

    // // 2. 打开 Socket 0
    // printf("[2] Calling socket(0, TCP, 50000)...\r\n");
    // uint8_t sock_ret = socket(0, Sn_MR_TCP, 50000, 0x00);
    // printf("[2] socket() returned: %d\r\n", sock_ret);

    // // 3. 检查初始状态
    // uint8_t status = getSn_SR(0);
    // printf("[3] Socket 0 Status: 0x%02X (SOCK_INIT)\r\n", status);

    // ================= 【绝对不可省略】发起 TCP 连接并等待 =================
    // ⚠️ 强制使用 OneNET 明文 IP，放弃 DNS 解析的 mqtts 域名！
    // uint8_t server_ip[4] = {183, 230, 40, 96}; // OneNET 官方明文 MQTT IP
    // uint16_t server_port = 1883;               // 必须是 1883
    
    // printf("[4] Connecting to OneNET MQTT: %d.%d.%d.%d:%d ...\r\n", 
    //        server_ip[0], server_ip[1], server_ip[2], server_ip[3], server_port);
    
    // // 【关键动作】发起 TCP 三次握手 (拨号)
    // connect(0, server_ip, server_port); 
    
    // // 【关键动作】阻塞等待，直到状态变为 0x17 (SOCK_ESTABLISHED)
    // uint16_t timeout = 0;
    // while (getSn_SR(0) != SOCK_ESTABLISHED) 
    // {
    //     if (getSn_SR(0) == SOCK_CLOSED) {
    //         printf("[ERROR] TCP Connection Refused by Server!\r\n");
    //         break;
    //     }
    //     HAL_Delay(10); // 等待 10ms
    //     timeout++;
    //     if (timeout > 500) { // 5秒超时
    //         printf("[ERROR] TCP Connection Timeout!\r\n");
    //         disconnect(0);
    //         break;
    //     }
    // }
    
    // // 检查结果
    // if (getSn_SR(0) == SOCK_ESTABLISHED) {
    //     printf("[SUCCESS] TCP Connected! Status: 0x17. Ready for MQTT!\r\n\r\n");
        
    //     // ⚠️ 只有在这里，才可以安全地调用你的 MQTT 连接逻辑！
    //     // 例如： do_mqtt(); 或者 MQTTConnect(&c, &data);
    // } else {
    //     printf("[ERROR] TCP Failed. Cannot proceed to MQTT.\r\n\r\n");
    // }
    //================================================================

    // 1. 初始化 LCD
    LCD_Init();                
    // 2. 设置画笔颜色
    POINT_COLOR = RED;     

    // 3. 将 LCD ID 格式化到字符串数组
    sprintf((char *)lcd_id, "LCD ID:%04X", lcddev.id);
    // 显示固定的标题（只画一次，避免闪烁）
      POINT_COLOR = RED;
      LCD_ShowString(30, 90, 200, 16, 16, lcd_id);       // 显示 LCD ID   
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    char date_str[20];
    char time_str[20];

    display_init();
    
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    { 
     // ---------------------------------------------------------
     // 任务1：每 1000ms 执行一次传感器读取和 MQTT
     // --------------------------------------------------------- 
     if (HAL_GetTick() - last_sensor_time >= 1000) 
        {
            last_sensor_time = HAL_GetTick(); // 更新时间戳  
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
         
      } 
      else 
      {
          // 如果读取失败，显示错误提示
          POINT_COLOR = RED;
          //LCD_ShowString(60, 180, 70, 16, 16, (u8*)"Error");
          //(60, 200, 70, 16, 16, (u8*)"Error");
      }
      

      // ================== 2. 读取并显示光照度 (SY30/BH1750) ==================
      light = GY30_GetData();      
      // 格式化为 5 位整数字符串（例如 "  150" 或 "00150"）
      sprintf(str_light, "%5d", light);     
      // 在 x=80, y=220 处显示光照数值
      display_refresh(str_temp, str_hum, str_light);
      //LCD_ShowString(50, 220, 70, 16, 16, (u8*)str_light);
      // 1. 核心：循环处理 MQTT 状态机 (连接、订阅、发布、保活)
      do_mqtt(); 
      // 翻转 LED 表示程序在运行
      HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9); // 请根据你的实际 LED 引脚修改！
    }
        // ---------------------------------------------------------
        // 任务2：每 200ms 刷新一次 RTC 时间，确保秒数平滑不跳帧
        // ---------------------------------------------------------
        if (HAL_GetTick() - last_rtc_time >= 200)
        {
            last_rtc_time = HAL_GetTick(); // 更新时间戳
     if(HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN)==HAL_OK){
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
        sprintf(date_str, "20%02d/%02d/%02d", sDate.Year, sDate.Month, sDate.Date);
        sprintf(time_str, "%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
        display_time( date_str, time_str);
     }
        }
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
  RCC_OscInitStruct.PLL.PLLM = 8;
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
