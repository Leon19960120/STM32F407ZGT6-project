/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.c
  * @brief   This file provides code for the configuration
  *          of the SPI instances.
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
#include "spi.h"

/* USER CODE BEGIN 0 */

#include <string.h>
/* USER CODE END 0 */

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

/* SPI1 init function */
void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}
/* SPI3 init function */
void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi3.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
  else if(spiHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspInit 0 */

  /* USER CODE END SPI3_MspInit 0 */
    /* SPI3 clock enable */
    __HAL_RCC_SPI3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI3 GPIO Configuration
    PB3     ------> SPI3_SCK
    PB4     ------> SPI3_MISO
    PB5     ------> SPI3_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI3_MspInit 1 */

  /* USER CODE END SPI3_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
  else if(spiHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspDeInit 0 */

  /* USER CODE END SPI3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI3_CLK_DISABLE();

    /**SPI3 GPIO Configuration
    PB3     ------> SPI3_SCK
    PB4     ------> SPI3_MISO
    PB5     ------> SPI3_MOSI
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);

  /* USER CODE BEGIN SPI3_MspDeInit 1 */

  /* USER CODE END SPI3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

// /**
//  * @brief      spi bus write read (LibDriver 适配层)
//  * @note       必须保证在 in_buf 发送和 out_buf 接收期间，CS 引脚保持低电平！
//  *             使用一次完整的 SPI 事务完成收发，满足 W25Qxx 时序要求。
//  */
// uint8_t spi_write_read(uint8_t *in_buf, uint32_t in_len, uint8_t *out_buf, uint32_t out_len)
// {
//     HAL_StatusTypeDef res;

//     /* 1. 拉低 CS (片选) */
//     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

//     /* 2. 如果有数据要发送 */
//     if (in_len > 0 && in_buf != NULL)
//     {
//         if (out_len > 0 && out_buf != NULL)
//         {
//             /* 全双工收发：同时发送 in_buf 并接收 out_buf */
//             res = HAL_SPI_TransmitReceive(&hspi3, in_buf, out_buf, in_len < out_len ? in_len : out_len, 1000);
//         }
//         else
//         {
//             /* 只发送，不接收 */
//             res = HAL_SPI_Transmit(&hspi3, in_buf, in_len, 1000);
//         }

//         if (res != HAL_OK)
//         {
//             HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
//             return 1;
//         }
//     }
//     /* 3. 如果只需要接收（W25Qxx 读 ID 场景：发 dummy + 收数据） */
//     else if (out_len > 0 && out_buf != NULL)
//     {
//         /* 发送 dummy 数据产生时钟，同时接收 Flash 返回的数据 */
//         uint8_t dummy[in_len > 0 ? in_len : out_len];
//         memset(dummy, 0xFF, sizeof(dummy));
//         res = HAL_SPI_TransmitReceive(&hspi3, dummy, out_buf, out_len, 1000);
//         if (res != HAL_OK)
//         {
//             HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
//             return 1;
//         }
//     }

//     /* 4. 拉高 CS，结束本次通信 */
//     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);

//     return 0;
// }

/**
 * @brief      spi bus write read (LibDriver 适配层)
 */
uint8_t spi_write_read(uint8_t *in_buf, uint32_t in_len, uint8_t *out_buf, uint32_t out_len)
{
    uint8_t res;
    
    /* set cs low */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    
    /* if in_len > 0 */
    if (in_len > 0)
    {
        /* transmit the input buffer */
        res = HAL_SPI_Transmit(&hspi3, in_buf, in_len, 1000);
        if (res != HAL_OK)
        {
            /* set cs high */
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
           
            return 1;
        }
    }
    
    /* if out_len > 0 */
    if (out_len > 0)
    {
        /* transmit to the output buffer */
        res = HAL_SPI_Receive(&hspi3, out_buf, out_len, 1000);
        if (res != HAL_OK)
        {
            /* set cs high */
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
           
            return 1;
        }
    }
    
    /* set cs high */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    
    return 0;
}


/**
 * @brief     spi bus init (适配 CubeMX 版本)
 * @param[in] 
 * @return    status code
 *            - 0 success
 *            - 1 init failed
 * @note      SCLK is PA5, MOSI is PA7, MISO is PA6, CS is PA14 (软件控制)
 */
uint8_t spi_init(void)
{
    //1. 直接调用 CubeMX 生成的 SPI 初始化函数
    ///该函数内部会自动配置 PA5(SCK), PA6(MISO), PA7(MOSI) 并调用 HAL_SPI_Init
    //MX_SPI3_Init();
    
    // (可选) 如果你的 CubeMX 默认配置的模式与传入的 mode 不一致，可以在这里动态修改
    // 但通常建议在 CubeMX 的图形界面中直接配置好 Mode 0 或 Mode 3，这里直接忽略 mode 参数以避免警告
    //(void)mode; 

    // 2. 手动初始化 CS (片选) 引脚
    // 注意：CubeMX 的 SPI 初始化不会配置 CS 引脚，因为 CS 通常由软件 GPIO 控制
    //GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    //确保 GPIOA 时钟已开启 (如果 MX_SPI1_Init 没开的话)
   //__HAL_RCC_GPIOB_CLK_ENABLE();
    
    // GPIO_InitStruct.Pin = GPIO_PIN_14;                 // CS 引脚
    // GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;       // 推挽输出
    // GPIO_InitStruct.Pull = GPIO_PULLUP;               // 上拉
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;     // 高速
    // HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    //3. 【极其重要】默认拉高 CS，释放总线，防止一上电就干扰 Flash
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    
    return 0; // 成功返回 0
    
}

/**
 * @brief  spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t spi_deinit(void)
{
    /* cs deinit */
    //HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14);
     /* 【优化】将 CS 拉高，释放总线，防止浮空引脚引入干扰 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    /* spi deinit */
    if (HAL_SPI_DeInit(&hspi3) != HAL_OK)
    {
        return 1;
    }
    
    return 0;
}

/* USER CODE END 1 */

