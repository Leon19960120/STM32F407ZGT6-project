7.16日 将标准库的代码移植到hal库工程下
## Step 3: 重构 SPI 底层通信函数 (核心)
## 7.17日
1. Core 文件夹里到底是什么？
Core = CubeMX 自动生成的系统代码，主要包括：
main.c / main.h
stm32f4xx_hal_msp.c（外设底层初始化）
stm32f4xx_it.c（中断服务函数）
syscalls.c、sysmem.c 等系统文件
gpio.c、usart.c、spi.c 等 HAL 初始化代码
它的定位是：
系统骨架、外设初始化、中断处理，不是写业务逻辑的地方。
2. 那真正的 “业务层” 是谁？
真正的业务层 = User 文件夹
你自己写的：
逻辑控制
状态机
应用任务
业务算法
应用层协议处理（TCP 业务、MQTT 业务等）
3. Hardware 又是什么？
Hardware = 硬件驱动层
你基于 HAL 封装的驱动：
led.c、key.c
beep.c
lcd.c、oled.c
w5500.c（SPI/CS/ 复位驱动）
传感器驱动（SHT30、BH1750 等）
4. 最清晰的四层结构（嵌入式标准分层）
Drivers → ST 官方 HAL 库（最底层）
Core → 系统初始化、中断、基本配置
Hardware → 你写的硬件驱动
User → 业务逻辑（应用层）
5. 一句话总结
Core：系统骨架，CubeMX 生成，尽量少改
Hardware：驱动层，操作硬件
User：业务层，实现功能逻辑