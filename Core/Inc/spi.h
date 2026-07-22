/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.h
  * @brief   This file contains all the function prototypes for
  *          the spi.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SPI_HandleTypeDef hspi1;

extern SPI_HandleTypeDef hspi3;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_SPI1_Init(void);
void MX_SPI3_Init(void);

/* USER CODE BEGIN Prototypes */
/**
 * @brief spi mode enumeration definition (LibDriver 接口要求)
 */
typedef enum
{
    SPI_MODE_0 = 0x00,        /**< mode 0 (CPOL=0, CPHA=0) */
    SPI_MODE_1 = 0x01,        /**< mode 1 (CPOL=0, CPHA=1) */
    SPI_MODE_2 = 0x02,        /**< mode 2 (CPOL=1, CPHA=0) */
    SPI_MODE_3 = 0x03,        /**< mode 3 (CPOL=1, CPHA=1) */
} spi_mode_t;



// 声明 SPI 初始化函数
uint8_t spi_init(void);
// 声明 SPI 反初始化函数
uint8_t spi_deinit(void);
// 添加 LibDriver 需要的 SPI 读写封装函数声明
uint8_t spi_write_read(uint8_t *in_buf, uint32_t in_len, uint8_t *out_buf, uint32_t out_len);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

