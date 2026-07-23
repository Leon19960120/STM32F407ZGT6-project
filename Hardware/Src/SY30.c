#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include "SY30.h"

/**I2C2 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB11     ------> I2C2_SDA
*/

// 定义 I2C 句柄
extern I2C_HandleTypeDef hi2c2;     //     这里使用 hi2c2
#define GY30_I2C_HANDLE  hi2c2      //     改为符合当前模块的名字
#define GY30_ADDR  (0x23U << 1)

/*初始化在main函数中调用一次*/
uint8_t GY30_Init(void){
    uint8_t cmd = 0x10;  /*连续高分辨率模式*/
    uint8_t cmd_power =0x01;

    //发送上电命令
     HAL_I2C_Master_Transmit(&GY30_I2C_HANDLE , GY30_ADDR, &cmd_power, 1, HAL_MAX_DELAY);
    if(HAL_I2C_Master_Transmit(&hi2c2,GY30_ADDR,&cmd,1,100)){
        return 0 ;
    }
    HAL_Delay(200);
    return 1;
}

/*
GY30读取光照数据
硬件上把 ADDR 引脚接地，芯片的 7 位 I2C 地址就是 0x23；
在 STM32 HAL 库写代码时，不能直接传 0x23，
要转换成 8 位写地址 0x46 传给 HAL 的 I2C 函数，读写操作都用这个值。
*/

uint16_t GY30_GetData(void)
{
    uint8_t data[2];

    if (HAL_I2C_Master_Receive(&GY30_I2C_HANDLE,
                               (0x23U << 1),
                               data,
                               2,
                               100) != HAL_OK)
    {
        return 0;
    }

    uint16_t raw = ((uint16_t)data[0] << 8) | data[1];

    return (uint16_t)(((uint32_t)raw * 5U) / 6U);
}

HAL_StatusTypeDef GY30_ReadLux(uint16_t *lux)
{
    uint8_t data[2];
    uint16_t raw;

    if (lux == NULL)
    {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status =
        HAL_I2C_Master_Receive(&GY30_I2C_HANDLE,
                               (0x23U << 1),
                               data,
                               2,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }

    raw = ((uint16_t)data[0] << 8) | data[1];

    /* raw / 1.2，四舍五入 */
    *lux = (uint16_t)(((uint32_t)raw * 5U + 3U) / 6U);

    return HAL_OK;
}