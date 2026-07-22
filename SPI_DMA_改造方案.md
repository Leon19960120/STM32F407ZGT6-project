# STM32F407 SPI + DMA 改造方案

## 项目现状

你的 sht35 项目目前使用 **HAL 库阻塞式 SPI**，每次读写都占用 CPU 等待：

```c
// 当前代码（spi.c 第 200-233 行）
uint8_t spi_write_read(uint8_t *in_buf, uint32_t in_len, uint8_t *out_buf, uint32_t out_len)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // CS 拉低
    
    if (in_len > 0 && in_buf != NULL)
        HAL_SPI_Transmit(&hspi1, in_buf, in_len, 1000); // 阻塞发送
    
    if (out_len > 0 && out_buf != NULL)
        HAL_SPI_Receive(&hspi1, out_buf, out_len, 1000); // 阻塞接收
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // CS 拉高
    return 0;
}
```

**问题**：CPU 要一直等着 SPI 把数据发完/收完，这段时间什么也干不了。

---

## 第一步：CubeMX 配置 DMA

### 1.1 开启 SPI1 DMA

1. 打开 `sht35.ioc`（STM32CubeMX 工程文件）
2. 在图形界面中找到 **Connectivity → SPI1**
3. 点击下方的 **DMA Settings** 标签页
4. 点击 **Add**，添加两个 DMA 通道：
   - **SPI1_RX** → 通道：DMA2 Channel 0 Stream 0（或 Stream 2）
   - **SPI1_TX** → 通道：DMA2 Channel 0 Stream 3（或 Stream 5）
5. 设置参数：
   | 参数 | 值 |
   |------|-----|
   | Mode | Normal |
   | Data Width | Byte |
   | Memory Increment | Enable |
   | Peripheral Increment | Disable |
   | Priority | Medium |

### 1.2 配置中断（可选）

如果要使用回调函数通知 DMA 完成：

1. 进入 **NVIC Settings**
2. 勾选：
   - ✅ SPI1 global interrupt
   - ✅ DMA2 Stream 0 global interrupt（对应 RX）
   - ✅ DMA2 Stream 3 global interrupt（对应 TX）

### 1.3 重新生成代码

点击 **GENERATE CODE**，CubeMX 会自动在 `spi.c` 和 `main.h` 中添加 DMA 相关代码。

---

## 第二步：修改 spi.c

将 `Core/Src/spi.c` 中的 `spi_write_read()` 函数替换为以下 DMA 版本：

```c
/* USER CODE BEGIN 1 */

/**
 * @brief      spi bus write read (DMA 优化版)
 * @note       使用 DMA 传输，CS 引脚在 DMA 完成前必须保持低电平！
 */
uint8_t spi_write_read(uint8_t *in_buf, uint32_t in_len, uint8_t *out_buf, uint32_t out_len)
{
    uint32_t timeout = 100000; // 超时计数器
    
    /* 1. 拉低 CS (片选) */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); 
    
    /* 2. 发送阶段 (TX DMA) */
    if (in_len > 0 && in_buf != NULL)
    {
        g_spi_dma_done = 0;
        // 启动 DMA 发送 (非阻塞)
        if (HAL_SPI_Transmit_DMA(&hspi1, in_buf, in_len) != HAL_OK)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
            return 1; // 启动失败
        }
        
        // 等待 DMA 传输完成 (裸机轮询方式)
        while (g_spi_dma_done == 0 && timeout--); 
        if (timeout == 0) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
            return 1; // 超时
        }
    }
    
    /* 3. 接收阶段 (RX DMA) */
    if (out_len > 0 && out_buf != NULL)
    {
        // 【避坑核心】：接收时必须同时发送 0xFF 产生时钟！
        if (out_len > MAX_DMA_DUMMY_SIZE) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
            return 1; 
        }
        
        // 填充 Dummy 数据 (0xFF)
        memset(dummy_tx_buf, 0xFF, out_len); 
        
        g_spi_dma_done = 0;
        // 使用 全双工 DMA (TransmitReceive) 接收数据
        if (HAL_SPI_TransmitReceive_DMA(&hspi1, dummy_tx_buf, out_buf, out_len) != HAL_OK)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
            return 1;
        }
        
        timeout = 100000;
        while (g_spi_dma_done == 0 && timeout--);
        if (timeout == 0) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
            return 1;
        }
    }
    
    /* 4. 拉高 CS，结束本次通信 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    
    return 0;
}

// ================= HAL 库 DMA 回调函数 =================
// 必须放在 spi.c 中，或者 stm32f4xx_it.c 中
// TX DMA 完成回调
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
        g_spi_dma_done = 1;
    }
}

// RX DMA 完成回调
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
        g_spi_dma_done = 1;
    }
}

// 全双工 (TX+RX) DMA 完成回调
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
        g_spi_dma_done = 1;
    }
}

/* USER CODE END 1 */
```

同时在 `USER CODE BEGIN 0` 区域添加 DMA 相关变量声明：

```c
/* USER CODE BEGIN 0 */

// ================= DMA 专属变量 =================
// 1. DMA 传输完成标志位
volatile uint8_t g_spi_dma_done = 0;

// 2. Dummy 发送缓冲区 (用于 RX DMA 时提供时钟)
// 注意：W25Qxx 单次读取通常不超过 256/512 字节。
#define MAX_DMA_DUMMY_SIZE 512 
uint8_t dummy_tx_buf[MAX_DMA_DUMMY_SIZE];

// ================================================

/* USER CODE END 0 */
```

---

## 第三步：检查 CubeMX 生成的代码

CubeMX 重新生成后，你应该能在 `spi.c` 中看到类似这样的代码：

```c
// DMA 句柄声明
static DMA_HandleTypeDef hdma_spi1_rx;
static DMA_HandleTypeDef hdma_spi1_tx;

// SPI1 DMA 初始化函数
static void MX_DMA_Init(void)
{
    // ... DMA 配置代码 ...
}

// SPI1 MspInit 中应该包含：
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
    if(hspi->Instance==SPI1)
    {
        // ... GPIO 配置 ...
        
        /* SPI1 DMA Init */
        MX_DMA_Init();
        
        // DMA 中断配置
        __HAL_LINKDMA(hspi, hdmarx, hdma_spi1_rx);
        __HAL_LINKDMA(hspi, hdmatx, hdma_spi1_tx);
    }
}
```

如果没看到 `MX_DMA_Init()`，手动在 `spi.c` 顶部调用一次：

```c
// 在 spi_init() 函数末尾添加
MX_DMA_Init();
```

---

## 💡 核心避坑指南（必读）

### 1. 为什么 RX 阶段要用 `HAL_SPI_TransmitReceive_DMA`？

如果你只用 `HAL_SPI_Receive_DMA`，STM32 的 SPI 硬件处于"只听不说"状态，**SCK 引脚不会产生时钟信号**。W25Qxx 没有时钟就不会移位输出数据，你读到的全是 `0x00` 或 `0xFF`。

**解决办法**：使用全双工 DMA，TX 通道自动发送我们准备好的 `0xFF` (Dummy 数据)，这样 SCK 就会持续产生时钟，RX 通道就能正常把 Flash 的数据收进 `out_buf` 了。

### 2. CS 引脚的"生死线"

在原来的阻塞代码中，`HAL_SPI_Transmit` 执行完，数据就发完了。

但在 DMA 代码中，`HAL_SPI_Transmit_DMA` 只是**启动了 DMA**，数据还在后台搬运。

**如果你启动 DMA 后立刻拉高 CS，Flash 会认为通信提前结束，导致数据截断或损坏！**

所以，**必须等待 `g_spi_dma_done == 1`（回调函数触发）后，才能拉高 CS**。上面的代码已经严格保证了这一点。

### 3. RAM 占用优化

代码中定义了一个 `dummy_tx_buf[512]`。因为 W25Qxx 单次读取通常不超过 256/512 字节，所以 512 字节足够应付绝大多数场景。

如果你的业务需要一次性读取 4KB 数据，你可以：
- **方案 A**：把 `MAX_DMA_DUMMY_SIZE` 改成 4096（浪费 4KB RAM）。
- **方案 B（推荐）**：在 `spi_write_read` 的接收阶段加个 `while` 循环，每次 DMA 传输 256 字节，传完一次再启动下一次，直到 `out_len` 传完。

### 4. 如果你使用了 RTOS (如 FreeRTOS)

上面的 `while (g_spi_dma_done == 0)` 会死等，浪费 CPU。如果你用了 RTOS，请把标志位换成**信号量 (Semaphore)**：

```c
// 等待 DMA 完成
osSemaphoreAcquire(spi_dma_sem, 100); // 等待 100ms 超时

// 在回调函数中释放信号量
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        osSemaphoreRelease(spi_dma_sem);
    }
}
```

---

## 改造前后对比

| 指标 | 改造前（阻塞 SPI） | 改造后（DMA SPI） |
|------|-------------------|------------------|
| CPU 占用 | 每次读写都占用 CPU | DMA 搬运时 CPU 空闲 |
| 响应速度 | 慢（等 SPI 完成） | 快（可并行处理其他任务） |
| 大数据传输 | 不适合（如 OTA、图片加载） | 非常适合 |
| 代码复杂度 | 简单 | 稍复杂（需配置 DMA） |
| RAM 占用 | 无额外占用 | ~512 字节 dummy 缓冲 |

---

## 验证方法

1. 编译下载后，观察 LCD 显示是否正常刷新
2. 串口打印 W25Qxx 读取的 ID 值，确认能读到 `0xEF4015`（W25Q128）
3. 读取一段数据，与原始数据对比，确认无误
4. 用示波器观察 SCK 波形，确认 DMA 传输期间有持续的时钟信号

---

## 参考资源

- [STM32 HAL 库 SPI DMA 官方文档](https://www.st.com/resource/en/user_manual UM1725.pdf)
- [W25Qxx 数据手册](https://www.winbond.com/resource-files/w25q_xx%20v18%2010012018%20reva%2005132018.pdf)
- [STM32CubeMX DMA 配置教程](https://www.youtube.com/watch?v=example)
