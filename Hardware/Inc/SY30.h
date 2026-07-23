#ifndef __SY30_H_
#define __SY30_H_
#include "stm32f4xx_hal.h"

/* ==========================================
 *              外部函数声明
 * ========================================== */
uint8_t  GY30_Init(void);
uint16_t GY30_GetData(void);
#endif