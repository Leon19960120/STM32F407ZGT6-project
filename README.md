# SHT35 环境数据采集与 OneNET 云平台传输系统

基于 **STM32F407** 的多传感器数据采集、本地显示与以太网上传系统。通过 **W5500** 以太网模块使用 **MQTT 协议**将温湿度、光照数据上报至 **OneNET 云平台**，同时在 **TFT LCD** 上本地实时显示。

> **项目状态**：已完成从 STM32F103 StdPeriph + Keil MDK 到 STM32F407 HAL + Makefile 的迁移。

## 硬件平台

| 组件 | 型号/协议 | 说明 |
|------|----------|------|
| MCU | STM32F407ZGT6（Cortex-M4F，168MHz） | 1MB Flash / 192KB RAM |
| 温湿度传感器 | SHT3x（I2C1） | 高精度数字温湿度 |
| 光照传感器 | SY30/BH1750（I2C2） | 光照强度检测 |
| 以太网 | W5500（SPI1） | 内置 TCP/IP 协议栈 |
| LCD | TFT LCD（FSMC + SPI） | 彩色显示屏 |
| 实时时钟 | RTC（I2C） | 时间记录 |
| 调试串口 | USART1（PA9/PA10） | 115200 baud，printf 重定向 |

## 功能特性

- **多传感器采集**：SHT3x 温湿度 + SY30 光照强度 + RTC 时间戳
- **以太网通信**：W5500 实现 TCP/UDP 网络传输
- **MQTT 云平台**：通过 MQTT 协议连接 OneNET 云平台（mqtts.heclouds.com）
- **JSON 数据上报**：cJSON 序列化传感器数据为 JSON 格式
- **云端指令响应**：订阅云端 Topic，解析 JSON 控制指令（如 LED 开关）
- **本地显示**：TFT LCD 实时显示温湿度、光照数据和日期时间
- **DHCP/DNS**：支持自动获取 IP 和域名解析
- **串口调试**：USART1 输出调试信息和日志

## 工程目录结构

```
sht35/
├── Core/                         # CubeMX 生成的核心代码
│   ├── Inc/                      # 头文件（main.h, gpio.h, i2c.h, spi.h, usart.h, rtc.h, fsmc.h, tim.h）
│   └── Src/                      # 源文件（main.c, stm32f4xx_it.c, stm32f4xx_hal_msp.c, system_stm32f4xx.c, syscalls.c）
├── Hardware/                     # 用户硬件驱动层
│   ├── Inc/
│   │   ├── SHT3x.h               # 温湿度传感器（I2C1）
│   │   ├── SY30.h                # 光照传感器（I2C2）
│   │   ├── lcd.h                 # TFT LCD 驱动（FSMC）
│   │   └── font.H                # 字库
│   └── Src/
│       ├── SHT3x.c               # SHT3x I2C 读写 + CRC 校验
│       ├── SY30.c                # SY30/BH1750 光照采集
│       └── lcd.c                 # FSMC 驱动 TFT LCD
├── User/                         # 业务逻辑层
│   ├── wiz_platform/             # 硬件平台抽象层（SPI/UART/TIM/DWT_Delay 适配 ioLibrary）
│   │   ├── wiz_platform.c        # printf 重定向、TIM2 回调、SPI/W5500 GPIO
│   │   └── wiz_platform.h
│   ├── wiz_interface/            # W5500 网络接口封装（DHCP、网络配置）
│   │   ├── wiz_interface.c
│   │   └── wiz_interface.h
│   ├── MQTT/                     # MQTT 客户端业务逻辑
│   │   ├── do_mqtt.c             # MQTT 状态机（CONN→SUB→PUB→KEEPALIVE→RECV）
│   │   └── do_mqtt.h
│   ├── DNS/                      # DNS 域名解析
│   │   ├── do_dns.c
│   │   └── do_dns.h
│   └── CJSON/                    # cJSON JSON 解析/序列化库
│       ├── cJSON.c
│       └── cJSON.h
├── Utils/                        # 工具函数
│   ├── delay.c                   # DWT 微秒/毫秒延时（基于 Cortex-M DWT 计数器）
│   └── delay.h
├── ioLibrary_Driver/             # WIZnet ioLibrary（TCP/IP 协议栈）
│   ├── Ethernet/                 # socket API, wizchip_conf, w5500.c
│   └── Internet/                 # DHCP, DNS, MQTT Client, MQTTPacket
├── Drivers/                      # STM32 HAL Driver + CMSIS
│   ├── STM32F4xx_HAL_Driver/     # ST 官方 HAL 库
│   └── CMSIS/                    # ARM CMSIS-Core M4 + Device F407
├── build/                        # 编译输出（.hex, .bin, .elf, .map）
├── Makefile                      # GCC Makefile 构建脚本
├── STM32F407XX_FLASH.ld          # 链接脚本（Flash 0x08000000/1MB, RAM 0x20000000/192KB）
├── startup_stm32f407xx.s         # Cortex-M4 启动文件
├── sht35.ioc                     # STM32CubeMX 工程配置
└── Note.md                       # 开发笔记
```

## 软件架构分层

```
┌─────────────────────────────────────────────┐
│           User Layer（业务逻辑）              │
│  do_mqtt.c (MQTT状态机)  do_dns.c  cJSON    │
├─────────────────────────────────────────────┤
│       Hardware Abstraction（平台适配层）       │
│  wiz_platform.c (SPI/UART/TIM/DWT_Delay)     │
│  wiz_interface.c (DHCP/网络配置)              │
├─────────────────────────────────────────────┤
│       ioLibrary_Driver（WIZnet 协议栈）       │
│  socket.c  wizchip_conf.c  w5500.c          │
│  dhcp.c  dns.c  MQTTClient.c  MQTTPacket    │
├─────────────────────────────────────────────┤
│       Hardware Driver（硬件驱动层）            │
│  SHT3x.c (I2C)  SY30.c (I2C)  lcd.c (FSMC) │
│  gpio.c  spi.c  i2c.c  usart.c              │
├─────────────────────────────────────────────┤
│       HAL Library + CMSIS（ST 官方库）        │
│  stm32f4xx_hal_*.c  core_cm4.h  stm32f407xx.h│
└─────────────────────────────────────────────┘
```

## 程序运行流程

```
main()
  ├── HAL_Init()                          # HAL 库初始化
  ├── SystemClock_Config()                # HSE 8MHz + PLL → 168MHz
  ├── Delay_Init()                        # DWT 微秒延时初始化
  ├── MX_GPIO_Init()                      # LED, W5500_CS/RST/INT
  ├── MX_I2C1_Init()                      # SHT3x (I2C1)
  ├── MX_I2C2_Init()                      # SY30 (I2C2)
  ├── MX_SPI1_Init()                      # W5500 (SPI1)
  ├── MX_USART1_UART_Init()               # 调试串口 (115200)
  ├── MX_RTC_Init()                       # 实时时钟
  ├── MX_TIM2_Init()                      # 1ms 定时器
  ├── LCD_Init()                          # TFT LCD 初始化
  ├── HAL_TIM_Base_Start_IT(&htim2)       # 启动 TIM2 中断
  ├── Wizchip_Init()                      # W5500 初始化
  ├── network_init()                      # DHCP 获取 IP
  ├── mqtt_init()                         # DNS解析 + TCP连接 + MQTT初始化
  └── while(1)
      ├── do_mqtt()                       # MQTT 状态机循环
      ├── ReadSHT3x()                     # 温湿度采集
      ├── GY30_GetData()                  # 光照采集
      └── LCD_ShowString()                # 数据显示
```

## 快速开始

### 编译

```bash
# 确保已安装 arm-none-eabi-gcc 工具链
make

# 清理编译产物
make clean

# 烧录（需要 OpenOCD + ST-Link）
make flash
```

### 编译产物

| 文件 | 说明 |
|------|------|
| `build/sht35.bin` | 二进制固件（可直接烧录） |
| `build/sht35.hex` | Intel HEX 格式 |
| `build/sht35.elf` | ELF 可执行文件（调试用） |
| `build/sht35.map` | 内存映射文件 |

### 烧录

```bash
# 使用 OpenOCD + ST-Link
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program build/sht35.elf verify reset exit"

# 或使用 STM32CubeProgrammer
STM32_Programmer_CLI -c port=swd -w build/sht35.bin 0x08000000 -v -rst
```

## 网络配置

### OneNET 云平台

- **MQTT Broker**: `mqtts.heclouds.com:1883`（TLS 加密端口）
- **协议**: MQTT 3.1.1
- **发布 Topic**: `$sys/{product_id}/{device_name}/thing/property/post`
- **订阅 Topic**: `$sys/{product_id}/{device_name}/thing/property/set`
- **认证方式**: 设备签名认证（SHA1）

> 注意：MQTT 连接参数（clientid、username、passwd）在 `User/MQTT/do_mqtt.c` 中配置。

### 网络参数

在 `Core/Src/main.c` 中配置：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| Gateway_IP | 手动配置 | 网关地址 |
| Sub_Mask | 手动配置 | 子网掩码 |
| Phy_Addr | 手动配置 | MAC 地址 |
| IP_Addr | 手动配置 | 本机 IP |

## 编译环境

- **MCU**: STM32F407ZGT6（Cortex-M4F，带 FPU）
- **编译器**: GCC ARM Embedded（arm-none-eabi-gcc）
- **标准库**: STM32 HAL Library v1.6+
- **构建工具**: GNU Make
- **IDE**: VSCode + CMake/Makefile 插件（可选）
- **调试器**: ST-Link / J-Link + OpenOCD

### 编译器选项

```
-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
-Og -Wall -fdata-sections -ffunction-sections
-DUSE_HAL_DRIVER -DSTM32F407xx
-specs=nano.specs
```

## 已知问题与待办

- [ ] MQTT payload 仍为固定值（26.6°C），未与 SHT3x 实时数据联动
- [ ] OneNET 凭证（passwd）硬编码，需定期更新 SHA1 签名
- [ ] `wiz_platform.c` 中仍有部分旧 F103 注释代码待清理
- [ ] ESP8266 WiFi 方案尚未集成，当前仅支持 W5500 以太网

## 参考资源

- [Sensirion SHT3x 数据手册](https://www.sensirion.com/products/catalog/SHT3x/)
- [WizNet W5500 数据手册](https://www.wiznet.io/product-item/w5500/)
- [OneNET MQTT 接入文档](https://open.iot.10086.cn/doc/mqtt/book/connect.html)
- [Paho MQTT C 客户端](https://github.com/eclipse/paho.mqtt.embedded-c)
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405-415-stm32f407-417-stm32f415-417-arm-based-32-bit-mcus-stmicroelectronics.pdf)
