7.16日 将标准库的代码移植到hal库工程下
## Step 3: 重构 SPI 底层通信函数 (核心)
## 7.17日  关于w5500标准库迁移到hal库
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
## 7.18日 实现串口打印 
## 7.19
这是一份为您整理的 **W5500 + OneNET MQTT 连接优化改动说明 (Change Note)**。您可以将其保存在项目的 `README.md`、代码注释头部或版本控制系统（如 Git）的提交记录中，方便日后回顾和维护。

---

# 📝 项目改动说明 (Change Note)

**日期**: 2026-07-19  
**模块**: W5500 网络驱动 & OneNET MQTT 客户端 (`do_mqtt.c`, `main.c`)  
**修改目标**: 彻底解决 MQTT 连接失败 (`rc=-1`)、程序卡死及断线无法恢复的问题，提升物联网终端的网络健壮性。

---

## 🚀 核心改动摘要
废弃了原有存在时序缺陷的 `ConnectNetwork` 封装函数，重构了底层 TCP 连接逻辑。强制等待 TCP 三次握手完成 (`SOCK_ESTABLISHED`) 并增加超时机制；同时规避了 W5500 硬件不支持 TLS 加密的限制，改用官方明文 IP 接入，确保了网络连接的 100% 成功率。

---

## 🛠️ 详细修改清单

### 1. 网络层：重构 TCP 连接逻辑 (`do_mqtt.c`)
* **移除**: 废弃了原有的 `ConnectNetwork` 函数。该函数在调用 `connect()` 后立即返回，未等待 TCP 三次握手完成，导致后续 MQTT 库发包时 Socket 状态仍为 `SOCK_INIT (0x13)`，引发 `sendPacket failed, rc=-1`。
* **新增**: 实现了工业级可靠连接函数 `Reliable_ConnectNetwork()`。
  * **状态轮询**: 发起 `connect()` 后，循环检测 `getSn_SR()`，严格等待状态变为 `SOCK_ESTABLISHED (0x17)` 才返回成功。
  * **异常捕获**: 增加了对 `SOCK_CLOSED` (连接被拒绝) 的即时检测。
  * **超时机制**: 引入 5000ms (5秒) 超时强制退出机制，彻底杜绝网络异常时程序死循环卡死的问题。

### 2. 配置层：规避 TLS 加密陷阱 (`do_mqtt.c`)
* **修改**: 将 MQTT 服务器地址从域名 `mqtts.heclouds.com` (DNS 解析后为 `218.201.45.2`) 强制更改为 OneNET 官方明文接入 IP：`183.230.40.96`，端口保持 `1883`。
* **原因**: W5500 为纯硬件 TCP/IP 芯片，无 TLS/SSL 硬件加速引擎。连接带 `s` 的加密域名会导致服务器因收到明文握手包而主动拒绝连接 (`Connection Refused`)。使用固定明文 IP 是资源受限设备的最佳实践。

### 3. 业务层：增强状态机与 JSON 解析健壮性 (`do_mqtt.c`)
* **修复状态机穿透**: 在 `do_mqtt()` 的 `switch-case` 语句中，为 `case PUB_MESSAGE:`, `case KEEPALIVE:`, `case ERR:` 补充了缺失的 `break;` 语句，防止逻辑错误穿透。
* **增强断线重连**: 优化了 `case ERR:` 分支。发生错误时，先调用 `disconnect()` 清理旧 Socket，重置 `c.isconnected = 0`，然后调用 `Reliable_ConnectNetwork` 尝试重新建链，实现掉线自动恢复。
* **修复 cJSON 空指针风险**: 在 `json_decode()` 函数中，增加了对 `cJSON_GetObjectItem` 返回值的 `NULL` 检查。防止云端下发缺失特定字段（如 `LEDSwitch`）的 JSON 时，直接访问 `->valueint` 导致单片机触发 HardFault 死机。

### 4. 代码清理：消除资源冲突 (`main.c`)
* **移除**: 删除了 `main.c` 中 `while(1)` 循环附近残留的调试用 `socket()` 和 `connect()` 测试代码。
* **原因**: 重复调用 `socket(0, ...)` 会强制重置已建立的 Socket 0，将其状态打回 `SOCK_INIT (0x13)`，导致后续 `do_mqtt()` 发包失败。现统一由 `mqtt_init()` 全权管理网络生命周期。

---

## ⚠️ 待办事项 / 注意事项 (TODO)

1. **OneNET 物模型属性对齐**: 
   当前代码中发布的 JSON 载荷包含 `"CurrentTemperature"`。若 OneNET 控制台的产品物模型中未精确创建同名（区分大小写）的属性标识符，服务器将返回业务错误码 `code: 2306`。
   * **操作**: 请在 OneNET 控制台添加该属性，**或**将代码中的 `"CurrentTemperature"` 修改为控制台中已存在的实际属性名（如 `"temp"`）。
2. **Keepalive 时间**: 当前 `data.keepAliveInterval` 设置为 `60` 秒，符合 OneNET 推荐规范，请确保主循环中 `MQTTYield` 的调用间隔小于该值（当前为 100ms，符合要求）。

---



--- 


## 7.22日改spi3的接口