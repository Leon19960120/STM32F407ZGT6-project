#include "delay.h"
#include "stm32f4xx.h" 

// 外部变量，由 HAL 库在 SystemClock_Config() 中自动更新
extern uint32_t SystemCoreClock; 

/**
  * @brief  初始化 DWT 计数器
  */
void Delay_Init(void)
{
    // 1. 启用调试和追踪模块 (Trace Enable)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    
    // 2. 清空 DWT 计数器
    DWT->CYCCNT = 0;
    
    // 3. 启用 DWT 循环计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * @brief  微秒级延时
  */
void Delay_us(uint32_t us)
{
    // 计算 us 对应的 CPU 周期数: (SystemCoreClock / 1000000) 即为 1us 的周期数
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    uint32_t start = DWT->CYCCNT;
    
    // 等待直到经过的周期数达到目标值
    // 注意：无符号整数减法天然支持计数器溢出回绕，非常安全
    while ((DWT->CYCCNT - start) < ticks);
}

/**
  * @brief  毫秒级延时
  */
void Delay_ms(uint32_t ms)
{
    uint32_t ticks = ms * (SystemCoreClock / 1000);
    uint32_t start = DWT->CYCCNT;
    
    while ((DWT->CYCCNT - start) < ticks);
}