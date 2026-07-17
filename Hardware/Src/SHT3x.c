#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"

#include "SHT3x.h"

/*SHT3x地址*/
#define SHT3x_ADDRESS (0x44<<1)	//从机地址是7位，所以左移一位

/*设置使用哪一个I2C*/
#define I2Cx I2C1

/* 引入STM32CubeMX自动生成的I2C句柄 (如果是I2C2，请改成 hi2c2) */
extern I2C_HandleTypeDef hi2c1;
#define SHT3x_I2C_HANDLE     hi2c1

/*
https://blog.zeruns.tech
*/

/**
  * @brief  CRC校验，CRC多项式为：x^8+x^5+x^4+1，即0x31
  * @param  DAT 要校验的数据
  * @retval 校验码
  */
uint8_t SHT3x_CRC_CAL(uint16_t DAT)
{
	uint8_t i,t,temp;
	uint8_t CRC_BYTE;

	CRC_BYTE = 0xFF;
	temp = (DAT>>8) & 0xFF;

	for(t = 0; t < 2; t++)
	{
		CRC_BYTE ^= temp;
		for(i = 0;i < 8;i ++)
		{
			if(CRC_BYTE & 0x80)
			{
				CRC_BYTE <<= 1;
				CRC_BYTE ^= 0x31;
			}
			else
			{
				CRC_BYTE <<= 1;
			}
		}

		if(t == 0)
		{
			temp = DAT & 0xFF;
		}
	}

	return CRC_BYTE;
}

/**
  * @brief  兼容旧命名的SHT3x双字节发送函数
  * @param  cmd 16位完整命令 (如 0x2C0D)
  * @retval HAL状态
  */
HAL_StatusTypeDef SHT3x_WriteCommand(uint16_t cmd)
{ // <--- 检查这个左大括号有没有漏掉！
    uint8_t buf[2];
    buf[0] = (cmd >> 8) & 0xFF; // 高8位 MSB
    buf[1] = cmd & 0xFF;        // 低8位 LSB

    // 正确的调用方式：它作为一条“语句”留在自建函数的肚子里面
    return HAL_I2C_Master_Transmit(&SHT3x_I2C_HANDLE, SHT3x_ADDRESS, buf, 2, 100);
}

/**
  * @brief  读取SHT3x数据
  * @param  Hum 接收湿度的浮点数指针
  * @param  Temp 接收温度的浮点数指针
  * @retval 1 - 读取成功；0 - 读取失败
  */
  uint8_t ReadSHT3x(float *Hum,float *Temp){
	uint8_t recv_buf[6]={0};
	//1,发送单次测量命令
	if(SHT3x_WriteCommand(0x240B) != HAL_OK)
	{
		return 0;
	}
	// 2. 核心修改：等待传感器在内部把数据测量完毕（高可靠性模式需要时间）
    HAL_Delay(15);
	//读取回来数据
	// 【第二步】命令发完后，立刻把传感器测好的6个字节读回来
    if (HAL_I2C_Master_Receive(&SHT3x_I2C_HANDLE, SHT3x_ADDRESS , recv_buf, 6, 100) != HAL_OK)
    {
        return 0; // 接收失败直接退出
    }
	// 【第三步】将接收到的原始数据拼回16位（前2字节是温度，4、5字节是湿度）
    uint16_t TempData = (recv_buf[0] << 8) | recv_buf[1];
    uint16_t HumData  = (recv_buf[3] << 8) | recv_buf[4];

    
    
   // 5. 进行数据校验（能剔除I2C总线受干扰产生的错码）
    if (SHT3x_CRC_CAL(TempData) == recv_buf[2] && SHT3x_CRC_CAL(HumData) == recv_buf[5])
    {
        // 6. 转换成真实的物理温度和湿度
        *Hum = (float)HumData * 100.0f / 65535.0f;
        *Temp = (float)TempData * 175.0f / 65535.0f - 45.0f;
        return 1; // 返回1表示成功获取温湿度
    }
    return 0;
  }