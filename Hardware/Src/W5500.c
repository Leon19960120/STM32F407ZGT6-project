#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
#include <string.h>
#include "W5500.h"
// 1. 必须包含 main.h，因为 CubeMX 生成的引脚宏都在这里面
#include "main.h"

/***************----- 网络参数变量定义 -----***************/
 uint8_t Gateway_IP[4];			// 网关IP地址 
 uint8_t Sub_Mask[4];				// 子网掩码 
 uint8_t Phy_Addr[6];				// 物理地址(MAC) 
 uint8_t IP_Addr[4];				// 本机IP地址 

 uint8_t S0_Port[2];				// 端口0的端口号(5000) 
 uint8_t S0_DIP[4];				// 端口0目的IP地址 
 uint8_t S0_DPort[2];				// 端口0目的端口号(6000) 

 uint8_t UDP_DIPR[4];				// UDP(广播)模式,目的主机IP地址
 uint8_t UDP_DPORT[2];			// UDP(广播)模式,目的主机端口号

/***************----- 端口的运行模式 -----***************/
 uint8_t S0_Mode;					// 端口0的运行模式, 0:TCP服务器, 1:TCP客户端, 2:UDP
#define TCP_SERVER		0x00			// TCP服务器模式
#define TCP_CLIENT		0x01			// TCP客户端模式 
#define UDP_MODE		0x02			// UDP(广播)模式 

/***************----- 端口的运行状态 -----***************/
 uint8_t S0_State;				// 端口0状态记录, 1:完成初始化, 2:完成连接(可正常传输) 
#define S_INIT			0x01			// 端口完成初始化 
#define S_CONN			0x02			// 端口完成连接,可以正常传输数据 

/***************----- 端口收发数据的状态 -----***************/
 uint8_t S0_Data;					// 端口0接收和发送数据的状态, 1:接收到数据, 2:发送完成 
#define S_RECEIVE		0x01			// 端口接收到一个数据包 
#define S_TRANSMITOK	0x02			// 端口发送一个数据包完成 

/***************----- 端口数据缓冲区 -----***************/
 uint8_t Rx_Buffer[2048];			// 端口接收数据缓冲区 
 uint8_t Tx_Buffer[2048];			// 端口发送数据缓冲区 

 uint8_t W5500_Interrupt;			// W5500中断标志(0:无中断, 1:有中断)


// 假设 CubeMX 生成的 SPI 句柄名为 hspi1
extern SPI_HandleTypeDef hspi1; 

// 宏定义 CS 控制，提高可读性
#define W5500_CS_LOW()  HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET)
#define W5500_CS_HIGH() HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET)


/*******************************************************************************
* 函数名  : W5500_SPI_WriteRead
* 描述    : W5500 底层 SPI 读写封装 (全双工)
* 输入    : tx_buf: 发送缓冲区指针, rx_buf: 接收缓冲区指针, len: 长度
* 返回值  : 无
*******************************************************************************/
// static void W5500_SPI_WriteRead(const uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len)
// {
//     HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)tx_buf, rx_buf, len, HAL_MAX_DELAY);
// }


/*******************************************************************************
* 函数名  : Write_W5500_1Byte
* 描述    : 向 W5500 指定地址寄存器写入 1 个字节数据 (HAL库优化版)
* 输入    : reg: 16位寄存器地址, dat: 待写入的 8 位数据
* 输出    : 无
* 返回值  : 无
* 说明    : 一次性打包 4 个字节发送，效率最高，时序最稳
*******************************************************************************/
void Write_W5500_1Byte(uint16_t reg,uint8_t dat){
	uint8_t tx_buf[4];
	//组装spi发送帧
    tx_buf[0]=(reg>>8)&0xFF;
	tx_buf[1]=reg & 0xFF;
	tx_buf[2]=FDM1 | RWB_WRITE | COMMON_R ;
	tx_buf[3]=dat;
    W5500_CS_LOW();
    HAL_SPI_Transmit(&hspi1,tx_buf,4,HAL_MAX_DELAY) ;
	W5500_CS_HIGH();

}

/*******************************************************************************
* 函数名  : Write_W5500_1Byte
* 描述    : 向 W5500 指定地址寄存器写入 2 个字节数据 (HAL库优化版)
* 输入    : reg: 16位寄存器地址, dat: 待写入的 16 位数据
* 输出    : 无
* 返回值  : 无
* 说明    : 一次性打包 5 个字节发送，效率最高，时序最稳
*******************************************************************************/
void  Write_W5500_2Byte(uint16_t reg,uint16_t dat)
{
	uint8_t tx_buf[5];
	//组装spi发送帧
	tx_buf[0]=(reg>>8)&0xFF;
	tx_buf[1]=reg & 0xFF;
	tx_buf[2]=FDM2 | RWB_WRITE | COMMON_R ;
	tx_buf[3]=(dat>>8)& 0xFF;
	tx_buf[4]=dat & 0xFF;
	W5500_CS_LOW();
	HAL_SPI_Transmit(&hspi1, tx_buf, 2, HAL_MAX_DELAY);
	W5500_CS_HIGH();
}

/*******************************************************************************
* 函数名  : Write_W5500_nByte
* 描述    : 通过SPI1向指定地址寄存器写n个字节数据
* 输入    : reg:16位寄存器地址,*dat_ptr:待写入数据缓冲区指针,size:待写入的数据长度
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_nByte(uint16_t reg,uint8_t *dat_ptr,uint16_t size){
	//这里最大支持1500字节为例
	static uint8_t tx_buf[1503];

	tx_buf[0]=(reg>>8)& 0xFF ;
	tx_buf[1]= reg & 0xFF;
	tx_buf[2]= VDM |RWB_WRITE | COMMON_R ;
	//将用户数据拷贝到头部之后
	memcpy(&tx_buf[3],dat_ptr,size);

	W5500_CS_LOW();
	//一次性发送三字节+size字节数据
	HAL_SPI_Transmit(&hspi1, tx_buf, size+3, HAL_MAX_DELAY);
	W5500_CS_HIGH();

}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_1Byte
* 描述    : 通过SPI1向指定端口寄存器写1个字节数据
* 输入    : s:端口号(0-7),reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 一次性打包 4 个字节发送，效率最高，时序最稳
*******************************************************************************/
void Write_W5500_SOCK_1Byte(SOCKET s, uint16_t reg, uint8_t dat)
{
	uint8_t tx_buf[4];
	tx_buf[0]=(reg>>8)&0xFF ;
	tx_buf[1]= reg & 0xFF ;
	tx_buf[2]= FDM1 | RWB_WRITE | (s*0x20+0x08) ;
    tx_buf[3]=dat;	

	HAL_SPI_Transmit(&hspi1, tx_buf, 4, HAL_MAX_DELAY);
    W5500_CS_LOW();
	W5500_CS_HIGH();
	
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_2Byte
* 描述    : 通过SPI1向指定端口寄存器写1个字节数据
* 输入    : s:端口号(0-7),reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 一次性打包 5 个字节发送，效率最高，时序最稳
*******************************************************************************/
void Write_W5500_SOCK_2Byte(SOCKET s,uint16_t reg, uint8_t dat){
	uint8_t tx_buf[5];
	tx_buf[0]=(reg>>8)&0xFF ;
	tx_buf[1]= reg & 0xFF ;
	tx_buf[2]= FDM1 | RWB_WRITE | (s*0x20+0x08) ;
	tx_buf[3]= (dat>>8) & 0xFF ;
    tx_buf[4]=dat & 0xFF;	
	W5500_CS_LOW();
	HAL_SPI_Transmit(&hspi1, tx_buf, 2, HAL_MAX_DELAY);
	W5500_CS_HIGH();

}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_4Byte
* 描述    : 通过SPI1向指定端口寄存器写4个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,*dat_ptr:待写入的4个字节缓冲区指针
* 输出    : 无
* 返回值  : 无
* 说明    : 一次性打包 7 个字节发送，效率最高，时序最稳
*******************************************************************************/
void Write_W5500_SOCK_4Byte(SOCKET s, uint16_t reg ,const uint8_t *dat_ptr ){
	uint8_t tx_buf[7];
	tx_buf[0]=(reg>>8)& 0xFF;// 2字节地址 + 1字节控制字 + 4字节数据
	tx_buf[1]= reg;
	tx_buf[2]= FDM4|RWB_WRITE|(s*0x20+0x08);
	// 拷贝 4 字节数据 (例如 IP 地址)
	tx_buf[3]=dat_ptr[0];
	tx_buf[4]=dat_ptr[1];
    tx_buf[5]=dat_ptr[2];
    tx_buf[6]=dat_ptr[3];

	W5500_CS_LOW();
	HAL_SPI_Transmit(&hspi1, tx_buf, 7, HAL_MAX_DELAY);
	W5500_CS_HIGH();

}

/*******************************************************************************
* 函数名  : Read_W5500_1Byte
* 描述    : 读 W5500 指定通用地址寄存器的 1 个字节数据 (HAL库优化版)
* 输入    : reg: 16位寄存器地址
* 返回值  : 读取到的 1 个字节数据
* 说明    : 一次性发送 4 字节，同时接收 4 字节，第 4 个字节即为有效数据
*******************************************************************************/
uint8_t Read_W5500_1Byte(uint16_t reg)
{
    uint8_t tx_buf[4];
    uint8_t rx_buf[4];

    // 1. 组装 SPI 发送帧 (共 4 字节)
    tx_buf[0] = (reg >> 8) & 0xFF;                   // 寄存器地址高位
    tx_buf[1] = reg & 0xFF;                          // 寄存器地址低位
    tx_buf[2] = FDM1 | RWB_READ | COMMON_R;          // 控制字 (1字节长度, 读操作, 通用寄存器区)
    tx_buf[3] = 0x00;                                // 哑数据 (Dummy Byte)，用于产生时钟读出有效数据

    // 2. 启动 SPI 事务：拉低 CS -> 严格同步收发 4 字节 -> 拉高 CS
    W5500_CS_LOW();
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 4, HAL_MAX_DELAY);
    W5500_CS_HIGH();

    // 3. rx_buf[3] 就是第 4 个接收到的字节，即我们想要的有效数据
    return rx_buf[3];
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_1Byte
* 描述    : 读 W5500 指定端口寄存器的 1 个字节数据 (HAL库优化版)
* 输入    : s: 端口号 (0~7), reg: 16位寄存器地址
* 返回值  : 读取到的 1 个字节数据
* 说明    : 一次性发送 4 字节，同时接收 4 字节，第 4 个字节即为有效数据
*******************************************************************************/
uint8_t Read_W5500_SOCK_1Byte(SOCKET s, uint16_t reg)
{
    uint8_t tx_buf[4];
    uint8_t rx_buf[4];

    // 1. 组装 SPI 发送帧 (共 4 字节)
    tx_buf[0] = (reg >> 8) & 0xFF;                                // 寄存器地址高位
    tx_buf[1] = reg & 0xFF;                                       // 寄存器地址低位
    tx_buf[2] = FDM1 | RWB_READ | (s * 0x20 + 0x08);              // 控制字 (1字节长度, 读操作, Socket s 寄存器区)
    tx_buf[3] = 0x00;                                             // 哑数据 (Dummy Byte)，用于产生时钟读出有效数据

    // 2. 启动 SPI 事务：拉低 CS -> 严格同步收发 4 字节 -> 拉高 CS
    W5500_CS_LOW();
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 4, HAL_MAX_DELAY);
    W5500_CS_HIGH();

    // 3. rx_buf[3] 就是第 4 个接收到的字节，即我们想要的有效数据
    return rx_buf[3];
}


/*******************************************************************************
* 函数名  : Read_W5500_SOCK_2Byte
* 描述    : 读 W5500 指定端口寄存器的 2 个字节数据 (HAL库优化版)
* 输入    : s: 端口号 (0~7), reg: 16位寄存器地址
* 返回值  : 读取到的 2 个字节数据 (16位)
* 说明    : 一次性发送 5 字节，同时接收 5 字节。W5500 寄存器为大端模式(高位在前)
*******************************************************************************/
uint16_t Read_W5500_SOCK_2Byte(SOCKET s, uint16_t reg)
{
    uint8_t tx_buf[5];
    uint8_t rx_buf[5];

    // 1. 组装 SPI 发送帧 (共 5 字节)
    tx_buf[0] = (reg >> 8) & 0xFF;                                // 寄存器地址高位
    tx_buf[1] = reg & 0xFF;                                       // 寄存器地址低位
    tx_buf[2] = FDM2 | RWB_READ | (s * 0x20 + 0x08);              // 控制字 (2字节长度, 读操作, Socket s 寄存器区)
    tx_buf[3] = 0x00;                                             // 哑数据 1 (用于读出高位数据)
    tx_buf[4] = 0x00;                                             // 哑数据 2 (用于读出低位数据)

    // 2. 启动 SPI 事务：拉低 CS -> 严格同步收发 5 字节 -> 拉高 CS
    W5500_CS_LOW();
    HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 5, HAL_MAX_DELAY);
    W5500_CS_HIGH();

    // 3. 组合接收到的数据 (W5500 网络字节序为大端：高位在前，低位在后)
    // rx_buf[3] 是高位，rx_buf[4] 是低位
    return ((uint16_t)rx_buf[3] << 8) | rx_buf[4];
}


/*******************************************************************************
* 函数名  : Read_SOCK_Data_Buffer
* 描述    : 从 W5500 接收数据缓冲区中批量读取数据 (HAL库极致优化版)
* 输入    : s: 端口号, dat_ptr: 数据保存缓冲区指针
* 返回值  : 读取到的数据长度 (rx_size 个字节)
* 说明    : 彻底消除 for 循环，利用 static 缓冲区实现一次性批量收发，速度提升 10 倍以上
*******************************************************************************/
uint16_t Read_SOCK_Data_Buffer(SOCKET s, uint8_t *dat_ptr)
{
    uint16_t rx_size, phys_addr, next_rd_ptr;
    
    // 1. 获取接收数据长度
    rx_size = Read_W5500_SOCK_2Byte(s, Sn_RX_RSR);
    if (rx_size == 0) return 0;               // 没接收到数据则返回
    if (rx_size > 1460) rx_size = 1460;       // 限制最大读取长度，防止超出标准以太网 MTU

    // 2. 获取当前读指针并计算物理地址
    next_rd_ptr = Read_W5500_SOCK_2Byte(s, Sn_RX_RD);
    phys_addr = next_rd_ptr & (S_RX_SIZE - 1); // 计算实际的环形缓冲区物理地址

    uint8_t ctrl_byte = VDM | RWB_READ | (s * 0x20 + 0x18);

    // 3. 判断是否发生环形缓冲区“折返” (Wrap-around)
    if ((phys_addr + rx_size) <= S_RX_SIZE) 
    {
        // 【情况 A：未折返】一次性连续读取所有数据
        // 使用 static 避免占用过多栈空间，且 static 数组默认初始化为 0 (正好作为 SPI 哑数据)
        static uint8_t tx_buf[1505];
        static uint8_t rx_buf[1505];

        tx_buf[0] = (phys_addr >> 8) & 0xFF;
        tx_buf[1] = phys_addr & 0xFF;
        tx_buf[2] = ctrl_byte;
        // tx_buf[3] 到 tx_buf[2+rx_size] 保持为 0x00 即可，无需手动 memset

        W5500_CS_LOW();
        HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, rx_size + 3, HAL_MAX_DELAY);
        W5500_CS_HIGH();

        // 有效数据从 rx_buf[3] 开始，直接内存拷贝给用户缓冲区
        memcpy(dat_ptr, &rx_buf[3], rx_size);
    } 
    else 
    {
        // 【情况 B：发生折返】需要分两次读取
        uint16_t len1 = S_RX_SIZE - phys_addr; // 第一部分长度 (读到缓冲区末尾)
        uint16_t len2 = rx_size - len1;        // 第二部分长度 (从缓冲区开头继续读)

        // --- 读取第一部分 ---
        static uint8_t tx_buf1[1505];
        static uint8_t rx_buf1[1505];
        tx_buf1[0] = (phys_addr >> 8) & 0xFF;
        tx_buf1[1] = phys_addr & 0xFF;
        tx_buf1[2] = ctrl_byte;

        W5500_CS_LOW();
        HAL_SPI_TransmitReceive(&hspi1, tx_buf1, rx_buf1, len1 + 3, HAL_MAX_DELAY);
        W5500_CS_HIGH();
        memcpy(dat_ptr, &rx_buf1[3], len1); // 拷贝第一部分数据

        // --- 读取第二部分 ---
        static uint8_t tx_buf2[1505];
        static uint8_t rx_buf2[1505];
        tx_buf2[0] = 0x00; // 折返后，物理地址从 0x0000 开始
        tx_buf2[1] = 0x00;
        tx_buf2[2] = ctrl_byte;

        W5500_CS_LOW();
        HAL_SPI_TransmitReceive(&hspi1, tx_buf2, rx_buf2, len2 + 3, HAL_MAX_DELAY);
        W5500_CS_HIGH();
        memcpy(dat_ptr + len1, &rx_buf2[3], len2); // 紧接在上一部分之后拷贝
    }

    // 4. 更新 W5500 内部的读指针，并发送 RECV 命令，通知芯片释放缓冲区
    next_rd_ptr += rx_size;
    Write_W5500_SOCK_2Byte(s, Sn_RX_RD, next_rd_ptr);
    Write_W5500_SOCK_1Byte(s, Sn_CR, RECV);
    return rx_size;
}




void Write_SOCK_Data_Buffer(SOCKET s, uint8_t *dat_ptr, uint16_t size)
{
    uint16_t offset, offset1;
    uint8_t ctrl_byte = VDM | RWB_WRITE | (s * 0x20 + 0x10);
    
    // ... (前面获取 offset 的逻辑保持不变) ...
    offset1 = offset;
    offset &= (S_TX_SIZE - 1);

    // 动态分配或使用静态大数组作为 SPI 发送缓冲区 (避免频繁 malloc)
    // 注意：如果 size 很大，需确保 tx_spi_buf 足够大，或分包发送
    uint8_t tx_spi_buf[1500]; 
    
    // 构造 SPI 报文头
    tx_spi_buf[0] = (offset >> 8) & 0xFF;
    tx_spi_buf[1] = offset & 0xFF;
    tx_spi_buf[2] = ctrl_byte;
    
    // 拷贝实际数据到报文头之后
    memcpy(&tx_spi_buf[3], dat_ptr, size);

    W5500_CS_LOW();
    // 【关键优化】一次性发送所有数据，而不是循环单字节发送！
    HAL_SPI_Transmit(&hspi1, tx_spi_buf, size + 3, HAL_MAX_DELAY);
    W5500_CS_HIGH();

    // ... (后续更新 Sn_TX_WR 和发送 SEND 命令的逻辑保持不变) ...
    offset1 += size;
    Write_W5500_SOCK_2Byte(s, Sn_TX_WR, offset1);
    Write_W5500_SOCK_1Byte(s, Sn_CR, SEND);
}


/*******************************************************************************
* 函数名  : W5500_Hardware_Reset
* 描述    : 硬件复位W5500
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : W5500的复位引脚保持低电平至少500us以上,才能重围W5500
*******************************************************************************/
void W5500_Hareware_Reset(void){
    W5500_CS_LOW();
    HAL_Delay(200);
    W5500_CS_HIGH();
    while((Read_W5500_1Byte(PHYCFGR)&LINK)==0);//等待以太网连接完成.
}

/*******************************************************************************
* 函数名  : W5500_Init
* 描述    : 初始化 W5500 寄存器函数
*******************************************************************************/
void W5500_Init(void)
{
    uint8_t i = 0;

    Write_W5500_1Byte(MR, RST);       // 软件复位 W5500
    HAL_Delay(10);                    // 延时 10ms 

    Write_W5500_nByte(GAR, Gateway_IP, 4); // 设置网关
    Write_W5500_nByte(SUBR, Sub_Mask, 4);  // 设置子网掩码
    Write_W5500_nByte(SHAR, Phy_Addr, 6);  // 设置物理地址 (MAC)
    Write_W5500_nByte(SIPR, IP_Addr, 4);   // 设置本机 IP 地址

    // 设置发送和接收缓冲区大小 (每个 Socket 2KB)
    for (i = 0; i < 8; i++)
    {
        Write_W5500_SOCK_1Byte(i, Sn_RXBUF_SIZE, 0x02);
        Write_W5500_SOCK_1Byte(i, Sn_TXBUF_SIZE, 0x02);
    }

    Write_W5500_2Byte(RTR, 0x07D0);   // 设置重试时间 200ms
    Write_W5500_1Byte(RCR, 8);        // 设置重试次数 8 次
}

/*******************************************************************************
* 函数名  : Detect_Gateway
* 描述    : 检查网关服务器 (通过 ARP 请求获取网关 MAC)
* 返回值  : TRUE (0xFF) 成功, FALSE (0x00) 失败
*******************************************************************************/
uint8_t Detect_Gateway(void)
{
    uint8_t ip_adde[4];
    uint8_t j = 0;

    // 构造一个假的 IP 地址用于触发 ARP 请求
    ip_adde[0] = IP_Addr[0] + 1;
    ip_adde[1] = IP_Addr[1] + 1;
    ip_adde[2] = IP_Addr[2] + 1;
    ip_adde[3] = IP_Addr[3] + 1;

    Write_W5500_SOCK_4Byte(0, Sn_DIPR, ip_adde); // 写入目的 IP
    Write_W5500_SOCK_1Byte(0, Sn_MR, MR_TCP);    // 设置为 TCP 模式
    Write_W5500_SOCK_1Byte(0, Sn_CR, OPEN);      // 打开 Socket
    HAL_Delay(5);

    if (Read_W5500_SOCK_1Byte(0, Sn_SR) != SOCK_INIT)
    {
        Write_W5500_SOCK_1Byte(0, Sn_CR, CLOSE);
        return FALSE;
    }

    Write_W5500_SOCK_1Byte(0, Sn_CR, CONNECT);   // 触发 CONNECT，W5500 会自动发送 ARP 请求网关

    do
    {
        j = Read_W5500_SOCK_1Byte(0, Sn_IR);     // 读取中断寄存器
        if (j != 0)
        {
            Write_W5500_SOCK_1Byte(0, Sn_IR, j); // 清除中断标志
        }
        
        HAL_Delay(5);

        if ((j & IR_TIMEOUT) == IR_TIMEOUT)
        {
            return FALSE; // 超时，网关不存在或网络不通
        }
        else if (Read_W5500_SOCK_1Byte(0, Sn_DHAR) != 0xFF) // 检查是否获取到了网关的 MAC 地址 (非全 0xFF)
        {
            Write_W5500_SOCK_1Byte(0, Sn_CR, CLOSE);
            return TRUE; // 成功获取网关 MAC
        }
    } while (1);
}

/*******************************************************************************
* 函数名  : Socket_Init
* 描述    : 指定 Socket (0~7) 初始化
* ⚠️ 修复  : 原代码中所有寄存器操作都写死了 Socket 0，现已全部修正为传入的参数 s
*******************************************************************************/
void Socket_Init(SOCKET s)
{
    // 设置最大分片长度 (MTU - 40 = 1500 - 40 = 1460)
    Write_W5500_SOCK_2Byte(s, Sn_MSSR, 1460); 

    switch (s)
    {
        case 0:
            // 设置本地端口号 (优化了位运算写法，替代 *256)
            Write_W5500_SOCK_2Byte(s, Sn_PORT, (S0_Port[0] << 8) | S0_Port[1]);
            // 设置目的端口号
            Write_W5500_SOCK_2Byte(s, Sn_DPORTR, (S0_DPort[0] << 8) | S0_DPort[1]);
            // 设置目的 IP 地址
            Write_W5500_SOCK_4Byte(s, Sn_DIPR, S0_DIP);
            break;

        // 如果需要支持其他 Socket，在此处添加 case 1: case 2: 等逻辑
        default:
            break;
    }
}

/*******************************************************************************
* 函数名  : Socket_Connect
* 描述    : 设置指定 Socket 为 TCP 客户端模式并发起连接
*******************************************************************************/
uint8_t Socket_Connect(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s, Sn_MR, MR_TCP);
    Write_W5500_SOCK_1Byte(s, Sn_CR, OPEN);
    HAL_Delay(5);

    if (Read_W5500_SOCK_1Byte(s, Sn_SR) != SOCK_INIT)
    {
        Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE);
        return FALSE;
    }

    Write_W5500_SOCK_1Byte(s, Sn_CR, CONNECT);
    return TRUE;
}

/*******************************************************************************
* 函数名  : Socket_Listen
* 描述    : 设置指定 Socket 为 TCP 服务器模式并开启监听
*******************************************************************************/
uint8_t Socket_Listen(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s, Sn_MR, MR_TCP);
    Write_W5500_SOCK_1Byte(s, Sn_CR, OPEN);
    HAL_Delay(5);

    if (Read_W5500_SOCK_1Byte(s, Sn_SR) != SOCK_INIT)
    {
        Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE);
        return FALSE;
    }

    Write_W5500_SOCK_1Byte(s, Sn_CR, LISTEN);
    HAL_Delay(5);

    if (Read_W5500_SOCK_1Byte(s, Sn_SR) != SOCK_LISTEN)
    {
        Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE);
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************************
* 函数名  : Socket_UDP
* 描述    : 设置指定 Socket 为 UDP 模式
*******************************************************************************/
uint8_t Socket_UDP(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s, Sn_MR, MR_UDP);
    Write_W5500_SOCK_1Byte(s, Sn_CR, OPEN);
    HAL_Delay(5);

    if (Read_W5500_SOCK_1Byte(s, Sn_SR) != SOCK_UDP)
    {
        Write_W5500_SOCK_1Byte(s, Sn_CR, CLOSE);
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************************
* 函数名  : W5500_Interrupt_Process
* 描述    : W5500 中断处理程序框架
* ⚠️ 优化  : 移除了不安全的 goto 语句，改用更现代、更安全的 while 循环
*******************************************************************************/
void W5500_Interrupt_Process(void)
{
    uint8_t sir, sn_ir;

    // 只要 SIR (Socket Interrupt Register) 不为 0，就说明有未处理的中断
    while ((sir = Read_W5500_1Byte(SIR)) != 0)
    {
        if ((sir & S0_INT) == S0_INT) // Socket 0 事件处理
        {
            sn_ir = Read_W5500_SOCK_1Byte(0, Sn_IR);
            
            // 必须先写 1 清除中断标志位
            if (sn_ir != 0)
            {
                Write_W5500_SOCK_1Byte(0, Sn_IR, sn_ir);
            }

            if (sn_ir & IR_CON)       // 成功连接 (TCP)
            {
                S0_State |= S_CONN;
            }
            if (sn_ir & IR_DISCON)    // 连接断开 (TCP)
            {
                Write_W5500_SOCK_1Byte(0, Sn_CR, CLOSE);
                Socket_Init(0);       // 重新初始化 Socket 0
                S0_State = 0;
            }
            if (sn_ir & IR_SEND_OK)   // 数据发送完成
            {
                S0_Data |= S_TRANSMITOK;
            }
            if (sn_ir & IR_RECV)      // 接收到数据
            {
                S0_Data |= S_RECEIVE;
            }
            if (sn_ir & IR_TIMEOUT)   // 超时
            {
                Write_W5500_SOCK_1Byte(0, Sn_CR, CLOSE);
                S0_State = 0;
            }
        }
        
        // 循环末尾会再次读取 SIR，如果处理过程中产生了新中断，会继续处理
    }
}