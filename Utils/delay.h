#ifndef __DELAY_H
#define __DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
  * @brief  初始化 DWT 计数器 (必须在 HAL_Init() 之后调用)
  * @param  None
  * @retval None
  */
void Delay_Init(void);

/**
  * @brief  微秒级延时 (基于 DWT，不占用 SysTick，精度极高)
  * @param  us: 延时微秒数 (最大支持约 4000ms 级别的换算，建议单次 < 100000us)
  * @retval None
  */
void Delay_us(uint32_t us);

/**
  * @brief  毫秒级延时 (基于 DWT，作为 HAL_Delay 的高精度替代)
  * @param  ms: 延时毫秒数
  * @retval None
  */
void Delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* __DELAY_H */