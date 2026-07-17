#include <stdio.h>
#include <stdint.h>
#include "wiz_platform.h"
#include "wizchip_conf.h"
#include "wiz_interface.h"
#include "stm32f4xx_hal.h"
#include "dhcp.h"
#include "dns.h"
#include "mqtt_interface.h"

#include "main.h"

static uint8_t UsNumber = 0;
static uint16_t MsNumber = 0;

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef  htim2;


// void debug_usart_init(void)
// {
//     GPIO_InitTypeDef GPIO_InitStructure;
//     USART_InitTypeDef USART_InitStructure;
//     /* config USART1 clock */
//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

//     /* USART1 GPIO config */
//     /* Configure USART1 Tx (PA.09) as alternate function push-pull */
//     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(GPIOA, &GPIO_InitStructure);
//     /* Configure USART1 Rx (PA.10) as input floating */
//     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//     GPIO_Init(GPIOA, &GPIO_InitStructure);
	
//     /* USART1 mode config */
//     USART_InitStructure.USART_BaudRate = 115200;
//     USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//     USART_InitStructure.USART_StopBits = USART_StopBits_1;
//     USART_InitStructure.USART_Parity = USART_Parity_No;
//     USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//     USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//     USART_Init(USART1, &USART_InitStructure);
		
//     USART_Cmd(USART1, ENABLE);
// }


int fputc(int ch, FILE *f)
{
    /* Redirect hal library function printf to USART1 */
    //USART1->SR;
    //使用hal库发送单个字符，超时时间位最大值
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    // /* Wait for delivery to complete */
    // while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    return (ch);
}

/**
  * @brief  重定向 c 库函数 scanf/getchar 到 USART1
  */
/* Redirecting the hal library function scanf to USART1 */
int fgetc(FILE *f)
{
    /* Waiting for USART1 to enter data */
    // while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    // return (int)USART_ReceiveData(USART1);
    uint8_t ch = 0;
    // 使用 HAL 库函数接收单个字符，阻塞等待直到收到数据
    HAL_UART_Receive(&huart1, &ch, 1, HAL_MAX_DELAY);
    return ch;
}

// /*
//  * TIM_Period / Auto Reload Register(ARR) = 1000   TIM_Prescaler--71 
//  * interrupt period = 1/(72MHZ /72) * 1000 = 1ms
//  *
//  * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> TIMxCNT reset to 0 to restart counting
//  */
// void wiz_timer_init(void)
// {
//     TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//     NVIC_InitTypeDef NVIC_InitStructure;

//     RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
//     TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
//     TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
//     TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
//     TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

//     TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
//     TIM_ClearFlag(TIM2, TIM_FLAG_Update);
//     TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
//     NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
//     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStructure);
//     TIM_Cmd(TIM2, ENABLE);
// }



// void wiz_spi_init(void)
// {
//     RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD, ENABLE);
//     GPIO_InitTypeDef GPIO_InitStructure;
//     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(GPIOB, &GPIO_InitStructure);
	
//     SPI_InitTypeDef SPI_InitStructure;
//     SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
//     SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
//     SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
//     SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
//     SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
//     SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
//     SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
//     SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
//     SPI_Init(SPI2, &SPI_InitStructure);
//     SPI_Cmd(SPI2, ENABLE);
	
//     /* PD_7 -> CS */
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//     GPIO_InitStructure.GPIO_Pin = WIZ_SCS_PIN;
//     GPIO_Init(WIZ_SCS_PORT, &GPIO_InitStructure);
//     GPIO_SetBits(WIZ_SCS_PORT, WIZ_SCS_PIN);
// }

// void wiz_rst_int_init(void)
// {
//     GPIO_InitTypeDef GPIO_InitStructure;

//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
//     /* PD_08 -> RST */
//     GPIO_InitStructure.GPIO_Pin = WIZ_RST_PIN;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(WIZ_RST_PORT, &GPIO_InitStructure);
//     GPIO_SetBits(WIZ_RST_PORT, WIZ_RST_PIN);

//     /* PD_09 -> INT */
//     GPIO_InitStructure.GPIO_Pin = WIZ_INT_PIN;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//     GPIO_Init(WIZ_INT_PORT, &GPIO_InitStructure);
// }

// /**
//  * @brief   delay init
//  * @param   none
//  * @return  none
//  */
// void delay_init(void)
// {

// 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
// 	UsNumber = SystemCoreClock / 8000000;
// 	MsNumber = (u16)UsNumber * 1000;
// }

/**
 * @brief   delay us
 * @param   none
 * @return  none
 */
void delay_us(uint32_t nus)
{
	uint32_t temp;
	SysTick->LOAD = nus * UsNumber;
	SysTick->VAL = 0x00;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	do
	{
		temp = SysTick->CTRL;
	} while ((temp & 0x01) && !(temp & (1 << 16)));
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL = 0X00;
}

// /**
//  * @brief   delay ms
//  * @param   none
//  * @return  none
//  */
// void delay_ms(uint32_t nms)
// {
// 	uint32_t temp;
// 	SysTick->LOAD = (uint32_t)nms * MsNumber;
// 	SysTick->VAL = 0x00;
// 	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
// 	do
// 	{
// 		temp = SysTick->CTRL;
// 	} while ((temp & 0x01) && !(temp & (1 << 16)));
// 	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
// 	SysTick->VAL = 0X00;
// }

// /**
//  * @brief   delay s
//  * @param   none
//  * @return  none
//  */
// void delay_s(uint32_t ns)
// {
// 		while(ns--)
// 		{
// 				delay_ms(1000);
// 		}
// }

/**
 * @brief   SPI select wizchip
 * @param   none
 * @return  none
 */
void wizchip_select(void)
{
    //GPIO_ResetBits(WIZ_SCS_PORT, WIZ_SCS_PIN);
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);    
}

/**
 * @brief   SPI deselect wizchip
 * @param   none
 * @return  none
 */
void wizchip_deselect(void)
{
    //GPIO_SetBits(WIZ_SCS_PORT, WIZ_SCS_PIN);
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET); 

}

/**
 * @brief   SPI write 1 byte to wizchip
 * @param   dat:1 byte data
 * @return  none
 */
void wizchip_write_byte(uint8_t dat)
{
    // while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
    // {
    // }
    // SPI_I2S_SendData(SPI2, dat);
    // while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
    // {
    // }
    // SPI_I2S_ReceiveData(SPI2);
     uint8_t dummy_rx = 0;
    // SPI 是全双工的，发送 1 字节的同时必然会接收 1 字节。
    // 必须使用 TransmitReceive 将接收到的无效数据读走，以清空 RXNE 标志位。
    // 否则连续发送会导致 SPI 接收缓冲区溢出 (OVR 错误)，后续通信全部卡死。
    HAL_SPI_TransmitReceive(&hspi1, &dat, &dummy_rx, 1, HAL_MAX_DELAY);
}

/**
 * @brief   SPI read 1 byte from wizchip
 * @param   none
 * @return  1 byte data
 */
uint8_t wizchip_read_byte(void)
{
    // while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
    // {
    // }
    // SPI_I2S_SendData(SPI2, 0xffff);
    // while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
    // {
    // }
    // return SPI_I2S_ReceiveData(SPI2);
    uint8_t dat_tx = 0xFF; // Dummy Byte (哑字节)
    uint8_t dat_rx = 0;    // 用于存放接收到的数据
    
    // 发送 0xFF 的同时，接收 W5500 返回的真实数据
    // 发送 0xFF 是为了让 STM32 产生 8 个时钟脉冲，W5500 才能在这 8 个时钟内把数据移出来
    HAL_SPI_TransmitReceive(&hspi1, &dat_tx, &dat_rx, 1, HAL_MAX_DELAY);
    
    return dat_rx;
}

/**
 * @brief   SPI write buff from wizchip
 * @param   buff:write buff
 * @param   len:write len
 * @return  none
 */
void wizchip_write_buff(uint8_t *buf, uint16_t len)
{
    uint16_t idx = 0;
    for (idx = 0; idx < len; idx++)
    {
        wizchip_write_byte(buf[idx]);
    }
}

/**
 * @brief   SPI read buff from wizchip
 * @param   buff:read buff
 * @param   len:read len
 * @return  none
 */
void wizchip_read_buff(uint8_t *buf, uint16_t len)
{
    uint16_t idx = 0;
    for (idx = 0; idx < len; idx++)
    {
        buf[idx] = wizchip_read_byte();
    }
}

/**
 * @brief   hardware reset wizchip
 * @param   none
 * @return  none
 */
void wizchip_reset(void)
{
    // GPIO_SetBits(WIZ_RST_PORT,WIZ_RST_PIN);
    // delay_ms(10);
    // GPIO_ResetBits(WIZ_RST_PORT,WIZ_RST_PIN);
    // delay_ms(10);
    // GPIO_SetBits(WIZ_RST_PORT,WIZ_RST_PIN);
    // delay_ms(10);
    // 1. 硬件复位 W5500 (拉低 -> 延时 -> 拉高)
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET); // 拉低复位
    HAL_Delay(100); // 延时 100ms
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);   // 拉高结束复位
    HAL_Delay(100); // 等待芯片启动

    // 2. 开启你的定时器中断（之前提到的）
    //HAL_TIM_Base_Start_IT(&htim2);
}

/**
 * @brief   wizchip spi callback register
 * @param   none
 * @return  none
 */
void wizchip_spi_cb_reg(void)
{
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
    reg_wizchip_spi_cbfunc(wizchip_read_byte, wizchip_write_byte);
    reg_wizchip_spiburst_cbfunc(wizchip_read_buff, wizchip_write_buff);
}

// /**
//  * @brief   Hardware Platform Timer Interrupt Callback Function
//  */
// void TIM2_IRQHandler(void)
// {   __HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE);

	// static uint32_t wiz_timer_1ms_count = 0;
	// if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	// {
    // wiz_timer_1ms_count++;
    // MilliTimer_Handler();
    // if (wiz_timer_1ms_count >= 1000)
    // {
    //     DHCP_time_handler();
    //     DNS_time_handler();
	// 	wiz_timer_1ms_count = 0;
    // }
	// 	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	// }
//}


// 2. 在这个文件 (wiz_platform.c) 的末尾，添加回调函数：
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static uint32_t wiz_timer_1ms_count = 0;

    if (htim->Instance == TIM2)
    {
        wiz_timer_1ms_count++;
        MilliTimer_Handler(); // W5500 1ms 心跳
        
        if (wiz_timer_1ms_count >= 1000)
        {
            DHCP_time_handler();
            DNS_time_handler();
            wiz_timer_1ms_count = 0;
        }
    }
}