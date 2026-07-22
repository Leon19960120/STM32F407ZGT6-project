# W25Qxx 驱动调用链总结

## 整体架构：四层分离

你的 W25Qxx 驱动采用了经典的 **LibDriver 分层架构**，每一层只关心自己的事，通过函数指针串联：

```
main.c（应用层）
    ↓ 使用 handle + 回调函数
driver_w25qxx.c（核心驱动层）
    ↓ 通过 handle 里的函数指针调用
stm32f407_driver_w25qxx_interface.c（硬件接口层）
    ↓ 封装成 spi_write_read()
spi.c（HAL 底层）
    ↓ 操作 SPI1 寄存器
W25Qxx Flash 芯片（硬件）
```

---

## 第一步：main.c 初始化

在 `main.c` 的第 199-218 行，做了三件事：

### 1. 创建句柄并清零

```c
w25qxx_handle_t g_w25qxx_handle;
memset(&g_w25qxx_handle, 0, sizeof(w25qxx_handle_t));
```

这个 `handle` 是一个结构体（定义在 `Components/w25qxx/src/driver_w25qxx.h`），里面存了一堆东西：

```c
typedef struct w25qxx_handle_s {
    uint8_t (*spi_qspi_init)(void);           // 指向初始化函数
    uint8_t (*spi_qspi_deinit)(void);         // 指向反初始化函数
    uint8_t (*spi_qspi_write_read)(...);      // 指向读写函数（最核心的）
    void (*delay_ms)(uint32_t ms);            // 指向延时函数
    void (*delay_us)(uint32_t us);            // 指向微秒延时函数
    void (*debug_print)(const char *const fmt, ...); // 指向打印函数
    uint8_t inited;                           // 初始化标志
    uint16_t type;                            // 芯片型号（W25Q16）
    uint8_t spi_qspi;                         // 接口类型（SPI/QSPI）
    uint8_t buf[262];                         // 内部缓冲区
    uint8_t buf_4k[4097];                     // 4K 大缓冲区
} w25qxx_handle_t;
```

你可以把它理解为一个 **"配置包"**，把芯片型号、接口方式、以及所有要调用的底层函数都打包在一起。

### 2. 绑定回调函数

```c
g_w25qxx_handle.debug_print         = w25qxx_interface_debug_print;
g_w25qxx_handle.spi_qspi_init     = w25qxx_interface_spi_qspi_init;
g_w25qxx_handle.spi_qspi_deinit   = w25qxx_interface_spi_qspi_deinit;
g_w25qxx_handle.spi_qspi_write_read = w25qxx_interface_spi_qspi_write_read;
g_w25qxx_handle.delay_ms          = w25qxx_interface_delay_ms;
g_w25qxx_handle.delay_us          = w25qxx_interface_delay_us;
```

这一步相当于告诉核心驱动："你要用的那些底层函数，都在这里，我帮你找好了。"

### 3. 配置硬件参数

```c
g_w25qxx_handle.spi_qspi = W25QXX_INTERFACE_SPI;  // 用标准 SPI
g_w25qxx_handle.type     = W25Q16;                 // W25Q16 芯片
```

### 4. 调用初始化

```c
w25qxx_init(&g_w25qxx_handle);
```

---

## 第二步：核心驱动层 driver_w25qxx.c

`w25qxx_init()` 做的事情是：

1. 调用 `handle->spi_qspi_init()` → 也就是你绑定的 `w25qxx_interface_spi_qspi_init()`
2. 读取 Flash 的 JEDEC ID（manufacturer + device id），验证芯片是否在线
3. 设置地址模式（W25Q16 用 3 字节地址）
4. 标记 `inited = 1`

这个层不关心 SPI 怎么实现，它只通过 `handle->spi_qspi_write_read(...)` 来发指令、读数据。

---

## 第三步：硬件接口层 stm32f407_driver_w25qxx_interface.c

这一层是 **适配器**，负责把 LibDriver 复杂的 SPI 接口参数，转换成你 `spi.c` 里简单的 `spi_write_read()` 调用：

```c
// LibDriver 传进来一大堆参数：instruction, address, dummy, data_line...
uint8_t w25qxx_interface_spi_qspi_write_read(
    uint8_t instruction, uint8_t instruction_line,
    uint32_t address, uint8_t address_line, uint8_t address_len,
    uint32_t alternate, uint8_t alternate_line, uint8_t alternate_len,
    uint8_t dummy, uint8_t *in_buf, uint32_t in_len,
    uint8_t *out_buf, uint32_t out_len, uint8_t data_line)
{
    // 只支持标准 SPI（data_line == 1），QSPI 模式直接返回失败
    if ((instruction_line != 0) || (address_line != 0) || 
        (alternate_line != 0) || (dummy != 0) || (data_line != 1))
    {
        return 1;
    }
    
    // 最终调用你写的 SPI 读写函数
    return spi_write_read(in_buf, in_len, out_buf, out_len);
}
```

注意这里 `in_buf` 和 `out_buf` 的对应关系：

| LibDriver 参数 | 含义 | 传给 spi_write_read |
|---|---|---|
| `instruction` | 指令码（如 0x90 读 ID） | 拼进 `in_buf` |
| `address` | 存储地址 | 拼进 `in_buf` |
| `dummy` | 空闲时钟周期 | 拼进 `in_buf` |
| `in_buf / in_len` | 发送缓冲区 | 直接传 |
| `out_buf / out_len` | 接收缓冲区 | 直接传 |

---

## 第四步：HAL 底层 spi.c

`spi_write_read()` 是你之前贴的那段代码，它做三件事：

```
1. CS 拉低（PA4 = 0）→ 选中 Flash
2. HAL_SPI_Transmit() → 发送指令/地址（TX DMA 改造后）
3. HAL_SPI_Receive() → 接收数据（RX DMA 改造后，同时发 0xFF 产生时钟）
4. CS 拉高（PA4 = 1）→ 释放 Flash
```

实际物理连接：

```
STM32F407 PA5 (SCK) ←————→ W25Qxx SCK
STM32F407 PA6 (MISO) ←————→ W25Qxx DO (MISO)
STM32F407 PA7 (MOSI) ←————→ W25Qxx DI (MOSI)
STM32F407 PA4 (CS)   ←————→ W25Qxx CS (片选)
```

---

## 完整调用链图

```
main()
  │
  ├─ w25qxx_init(&g_w25qxx_handle)
  │     │
  │     ├─ handle->spi_qspi_init()  ───────────────┐
  │     │                                          │
  │     │     硬件接口层                              ▼
  │     │  w25qxx_interface_spi_qspi_init()    spi_init(SPI_MODE_3)
  │     │     │                                    │
  │     │     │                                    ▼
  │     │     │                              MX_SPI1_Init()
  │     │     │                              HAL_SPI_Init()
  │     │     │                              GPIO 配置 (PA5/6/7)
  │     │     │                              CS 配置 (PA4)
  │     │     │
  │     ├─ 读取 JEDEC ID
  │     │     │
  │     │     ├─ handle->spi_qspi_write_read(...)
  │     │     │     │
  │     │     │     ▼
  │     │     │  w25qxx_interface_spi_qspi_write_read()
  │     │     │     │  校验参数，确认是标准 SPI
  │     │     │     ▼
  │     │     │  spi_write_read()
  │     │     │     │
  │     │     │     ├─ HAL_GPIO_WritePin(PA4, RESET)   ← CS 拉低
  │     │     │     ├─ HAL_SPI_Transmit_DMA()          ← 发送指令（DMA 版）
  │     │     │     ├─ HAL_SPI_TransmitReceive_DMA()   ← 收发数据（DMA 版）
  │     │     │     └─ HAL_GPIO_WritePin(PA4, SET)     ← CS 拉高
  │     │     │
  │     │     ▼
  │     │  SPI1 硬件 → W25Qxx Flash 芯片
  │
  └─ w25qxx_read(addr, data, len)  ← 后续读数据也走同样的链路
```

---

## 关键设计思想

这套代码最巧妙的设计是 **函数指针解耦**：

核心驱动 `driver_w25qxx.c` 完全不认识 SPI、GPIO、HAL 库。它只知道：

> "我有一个叫 `spi_qspi_write_read` 的函数，你帮我传进去就行。"

所以如果你以后换芯片（比如从 STM32F407 换成 ESP32），只需要重写 `stm32f407_driver_w25qxx_interface.c` 这一个适配器文件，核心驱动代码一行都不用改。

这就是为什么 LibDriver 的设计让你觉得"好复杂"但又"很清晰"——它在用一层层的间接调用，换取最大的可移植性。

---

## 涉及的文件清单

| 文件 | 层级 | 作用 |
|------|------|------|
| `Core/Src/main.c` | 应用层 | 创建 handle、绑定回调、调用 `w25qxx_init()` |
| `Components/w25qxx/src/driver_w25qxx.h` | 核心驱动 | 定义 `w25qxx_handle_t` 结构体和所有 API 声明 |
| `Components/w25qxx/src/driver_w25qxx.c` | 核心驱动 | `w25qxx_init()` 等实现，通过函数指针调用底层 |
| `Components/w25qxx/interface/stm32f407_driver_w25qxx_interface.c` | 硬件接口层 | 把 LibDriver 的参数转成 `spi_write_read()` 调用 |
| `Core/Src/spi.c` | HAL 底层 | `spi_write_read()` 实现，操作 SPI1 |
| `Core/Inc/spi.h` | HAL 底层 | SPI 函数声明 |
