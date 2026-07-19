#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include "SY30.h"

/**I2C2 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB11     ------> I2C2_SDA
*/

/*设置使用哪一个I2C*/
//#define I2Cx I2C2
// 定义 I2C 句柄
//I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c2;     //     这里使用 hi2c2
#define GY30_I2C_HANDLE  hi2c2      // 1. 改为符合当前芯片的名字

/*
GY30读取光照数据
硬件上把 ADDR 引脚接地，芯片的 7 位 I2C 地址就是 0x23；
在 STM32 HAL 库写代码时，不能直接传 0x23，
要转换成 8 位写地址 0x46 传给 HAL 的 I2C 函数，读写操作都用这个值。
*/

uint16_t GY30_GetData(void){
     uint8_t cmd_power =0x01;
     uint8_t cmd_mode = 0x10;
     uint8_t date[2]={0};

     const uint16_t GY30_ADDR =0x46;
     //发送上电命令
     HAL_I2C_Master_Transmit(&GY30_I2C_HANDLE , GY30_ADDR, &cmd_power, 1, HAL_MAX_DELAY);
     //发送高分辨率测量命令
     HAL_I2C_Master_Transmit(&GY30_I2C_HANDLE , GY30_ADDR, &cmd_mode, 1, HAL_MAX_DELAY);
     //等待传感器测量完成
     HAL_Delay(200);
     //读取两字节的测量数据
     if(HAL_I2C_Master_Receive(&GY30_I2C_HANDLE , GY30_ADDR, date, 2, HAL_MAX_DELAY)==HAL_OK)
     {
     //拼接高八位和低八位
     return (date[0]<<8|date[1]);
     }
     return 0;
}