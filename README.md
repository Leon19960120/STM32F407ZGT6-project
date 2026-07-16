# SHT35 环境数据采集与网络传输系统

基于 **STM32F407** 的多传感器数据采集与以太网传输系统，集成温湿度（SHT3x）、光照（SY30）传感器，通过 **W5500** 以太网模块上传数据，同时在 **TFT LCD** 上本地显示。

## 硬件平台

| 组件 | 型号/协议 | 说明 |
|------|----------|------|
| MCU | STM32F407xx（Cortex-M4，168MHz） | STMicroelectronics |
| 温湿度传感器 | SHT3x（I2C） | 高精度数字温湿度 |
| 光照传感器 | SY30 | 光照强度检测 |
| 以太网 | W5500（SPI） | 内置 TCP/IP 协议栈 |
| LCD | TFT LCD（FSMC + SPI） | 彩色显示屏 |
| 实时时钟 | RTC（I2C） | 时间记录 |

## 功能特性

- **温湿度采集**：通过 I2C 读取 SHT3x 传感器数据
- **光照强度检测**：SY30 光照传感器数据采集
- **以太网通信**：W5500 实现 TCP/UDP 网络传输
- **本地显示**：TFT LCD 实时显示传感器数据
- **RTC 时间戳**：通过 RTC 提供精确的时间信息
- **串口调试**：USART 用于调试信息和日志输出

## 工程目录结构

```
sht35/
├── Core/                         # 核心代码（CubeMX 生成）
│   ├── Inc/                      # 头文件
│   │   ├── main.h
│   │   ├── gpio.h
│   │   ├── i2c.h
│   │   ├── spi.h
│   │   ├── usart.h
│   │   ├── rtc.h
│   │   └── fsmc.h
│   └── Src/                      # 源文件
│       ├── main.c                # 主函数入口
│       ├── gpio.c
│       ├── i2c.c
│       ├── spi.c
│       ├── usart.c
│       ├── rtc.c
│       ├── fsmc.c
│       └── stm32f4xx_it.c
├── Hardware/                     # 用户硬件驱动
│   ├── Inc/
│   │   ├── SHT3x.h               # 温湿度传感器驱动
│   │   ├── SY30.h                # 光照传感器驱动
│   │   ├── W5500.h               # W5500 以太网驱动
│   │   ├── W5500_USER.h          # W5500 应用层封装
│   │   └── lcd.h                 # TFT LCD 驱动
│   └── Src/
│       ├── SHT3x.c
│       ├── SY30.c
│       ├── W5500.c
│       ├── W5500_USER.c
│       └── lcd.c
├── Drivers/                      # STM32 HAL 库 & CMSIS
├── build/                        # 编译输出（.hex、.bin、.elf）
├── Makefile                      # 构建脚本
├── compile_commands.json         # Clangd 索引
├── sht35.ioc                     # STM32CubeMX 工程文件
├── STM32F407XX_FLASH.ld          # 链接脚本
└── startup_stm32f407xx.s         # 启动文件
```

## 快速开始

### 使用 Makefile 编译

```bash
make
```

编译产物在 `build/` 目录下，包含 `.hex`、`.bin`、`.elf` 等格式。

### 使用 STM32CubeIDE / Keil

1. 用 **STM32CubeMX** 打开 `sht35.ioc` 工程文件
2. 重新生成代码
3. 使用 STM32CubeIDE 或 Keil MDK 打开工程并编译

### 烧录

使用 ST-Link 或 J-Link 将生成的固件烧录到 STM32F407 开发板：

```bash
# 使用 openocd 烧录
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program build/sht35.elf verify reset exit"
```

## 编译环境

- **IDE**: STM32CubeIDE / Keil MDK-ARM / VSCode
- **编译器**: GCC ARM / ARM Compiler
- **标准库**: STM32 HAL Library (STM32F4xx)
- **构建工具**: Make
- **CMSIS**: ARM Cortex-M4 Device CMSIS

## 注意事项

- 确保 SHT3x 的 I2C 总线连接正确（注意上拉电阻）
- W5500 的 SPI 引脚需正确配置
- FSMC 用于 LCD 驱动，需确认 FSMC 初始化正常
- 网络参数（IP、端口等）需在 W5500_USER 中配置

## 参考资源

- [Sensirion SHT3x 数据手册](https://www.sensirion.com/products/catalog/SHT3x/)
- [WizNet W5500 数据手册](https://www.wiznet.io/product-item/w5500/)
- STM32F407 Reference Manual
- ST7735 / TFT LCD 驱动数据手册
