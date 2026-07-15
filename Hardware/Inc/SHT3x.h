#ifndef __SHT3x_H
#define __SHT3x_H
#include "stm32f4xx_hal.h"

/* ==========================================
 *              宏定义配置
 * ========================================== */

/* SHT3x I2C 从机地址 (0x44左移一位得到0x88) */
//#define SHT3x_ADDRESS         (0x44 << 1) 

/* SHT3x 常用测量命令宏 */
#define SHT3x_CMD_MEAS_HIGH   0x2C06  // 高重复性单次测量 (Clock Stretching Disabled)
#define SHT3x_CMD_MEAS_MEDIUM 0x2C0D  // 中重复性单次测量 (Clock Stretching Disabled)
#define SHT3x_CMD_MEAS_LOW    0x2C10  // 低重复性单次测量 (Clock Stretching Disabled)

/* ==========================================
 *              外部函数声明
 * ========================================== */

/**
  * @brief  CRC校验，CRC多项式为：x^8+x^5+x^4+1，即0x31
  * @param  DAT 要校验的数据
  * @retval 校验码
  */
uint8_t SHT3x_CRC_CAL(uint16_t DAT);

/**
  * @brief  向SHT3x发送双字节命令
  * @param  cmd 16位完整命令 (如 0x2C0D)
  * @retval HAL状态
  */
HAL_StatusTypeDef SHT3x_WriteCommand(uint16_t cmd);

/**
  * @brief  读取SHT3x数据并进行CRC校验与物理量转换
  * @param  Hum 接收湿度结果的浮点数指针 (单位: %RH)
  * @param  Temp 接收温度结果的浮点数指针 (单位: ℃)
  * @retval 1 - 读取且校验成功；0 - 读取或校验失败
  */
uint8_t ReadSHT3x(float *Hum, float *Temp);

#endif