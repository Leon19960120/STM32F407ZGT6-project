/**********************************************************************************
 * 文件名  ：W5500_USER.c
 * 描述    ：W5500 函数库
**********************************************************************************/
#include "stm32f4xx_hal.h"
#include "W5500.h"
#include <string.h>

unsigned int Timer2_Counter = 0; 					//Timer2定时器计数变量(ms)
unsigned int W5500_Send_Delay_Counter = 0;			//W5500发送延时计数变量(ms)



/*******************************************************************************
* 函数名  : Delay
* 描述    : 延时函数(ms)
* 输入    : d:延时系数，单位为毫秒
* 输出    : 无
* 返回    : 无 
* 说明    : 延时是利用Timer2定时器产生的1毫秒的计数来实现的
*******************************************************************************/
void Delay(unsigned int d)
{
	Timer2_Counter = 0; 
	while(Timer2_Counter < d);
}

/*******************************************************************************
* 函数名  : W5500_Initialization
* 描述    : W5500初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5500_Initialization(void)
{
	W5500_Init();				//初始化W5500寄存器函数
	Detect_Gateway();		    //检查网关服务器 
	Socket_Init(0);			//指定Socket(0~7)初始化,初始化端口0
}

/*******************************************************************************
* 函数名  : Load_Net_Parameters_Client
* 描述    : 装载网络参数（客户端模式）
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 网关、掩码、物理地址、本机IP地址、端口号、目的IP地址、目的端口号、端口工作模式
*******************************************************************************/
/*例程网络参数*/
//网关：192.168.1.1
//掩码:	255.255.255.0
//物理地址：0C 29 AB 7C 00 01
//本机IP地址:192.168.1.199
//端口0的端口号：5000
//端口0的目的IP地址：192.168.1.190
//端口0的目的端口号：6000
void Load_Net_Parameters_Client(void)
{
	Gateway_IP[0] = 192;		//加载网关参数
	Gateway_IP[1] = 168;
	Gateway_IP[2] = 1;
	Gateway_IP[3] = 1;

	Sub_Mask[0] = 255;			//加载子网掩码
	Sub_Mask[1] = 255;
	Sub_Mask[2] = 255;
	Sub_Mask[3] = 0;

	Phy_Addr[0] = 0x0c;			//加载物理地址
	Phy_Addr[1] = 0x29;
	Phy_Addr[2] = 0xab;
	Phy_Addr[3] = 0x7c;
	Phy_Addr[4] = 0x00;
	Phy_Addr[5] = 0x01;

	IP_Addr[0] = 192;				//加载本机IP地址
	IP_Addr[1] = 168;
	IP_Addr[2] = 1;
	IP_Addr[3] = 199;

	S0_Port[0] = 0x13;			//加载端口0的端口号5000 
	S0_Port[1] = 0x88;

	S0_DIP[0] = 192;				//加载端口0的目的IP地址
	S0_DIP[1] = 168;
	S0_DIP[2] = 1;
	S0_DIP[3] = 190;
	
	S0_DPort[0] = 0x17;			//加载端口0的目的端口号6000
	S0_DPort[1] = 0x70;

	S0_Mode = TCP_CLIENT;		//加载端口0的工作模式,TCP客户端模式
}

/*******************************************************************************
* 函数名  : Load_Net_Parameters_Server
* 描述    : 装载网络参数(服务端模式)
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 网关、掩码、物理地址、本机IP地址、端口号、目的IP地址、目的端口号、端口工作模式
*******************************************************************************/
/*例程网络参数*/
//网关：192.168.1.1
//掩码:	255.255.255.0
//物理地址：0C 29 AB 7C 00 01
//本机IP地址:192.168.1.199
//端口0的端口号：5000
void Load_Net_Parameters_Server(void)
{
	Gateway_IP[0] = 192;		//加载网关参数
	Gateway_IP[1] = 168;
	Gateway_IP[2] = 1;
	Gateway_IP[3] = 1;

	Sub_Mask[0] = 255;			//加载子网掩码
	Sub_Mask[1] = 255;
	Sub_Mask[2] = 255;
	Sub_Mask[3] = 0;

	Phy_Addr[0] = 0x0c;			//加载物理地址
	Phy_Addr[1] = 0x29;
	Phy_Addr[2] = 0xab;
	Phy_Addr[3] = 0x7c;
	Phy_Addr[4] = 0x00;
	Phy_Addr[5] = 0x01;

	IP_Addr[0] = 192;				//加载本机IP地址
	IP_Addr[1] = 168;
	IP_Addr[2] = 1;
	IP_Addr[3] = 199;

	S0_Port[0] = 0x13;			//加载端口0的端口号5000 
	S0_Port[1] = 0x88;

	S0_Mode = TCP_SERVER;		//加载端口0的工作模式,TCP服务端模式
}

/*******************************************************************************
* 函数名  : Load_Net_Parameters_UDP
* 描述    : 装载网络参数(UDP模式)
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 网关、掩码、物理地址、本机IP地址、端口号、目的IP地址、目的端口号、端口工作模式
*******************************************************************************/
/*例程网络参数*/
//网关：192.168.1.1
//掩码:	255.255.255.0
//物理地址：0C 29 AB 7C 00 01
//本机IP地址:192.168.1.199
//端口0的端口号：5000
void Load_Net_Parameters_UDP(void)
{
	Gateway_IP[0] = 192;		//加载网关参数
	Gateway_IP[1] = 168;
	Gateway_IP[2] = 1;
	Gateway_IP[3] = 1;

	Sub_Mask[0] = 255;			//加载子网掩码
	Sub_Mask[1] = 255;
	Sub_Mask[2] = 255;
	Sub_Mask[3] = 0;

	Phy_Addr[0] = 0x0c;			//加载物理地址
	Phy_Addr[1] = 0x29;
	Phy_Addr[2] = 0xab;
	Phy_Addr[3] = 0x7c;
	Phy_Addr[4] = 0x00;
	Phy_Addr[5] = 0x01;

	IP_Addr[0] = 192;				//加载本机IP地址
	IP_Addr[1] = 168;
	IP_Addr[2] = 1;
	IP_Addr[3] = 199;

	S0_Port[0] = 0x13;			//加载端口0的端口号5000 
	S0_Port[1] = 0x88;
	
	UDP_DIPR[0] = 192;			//UDP(广播)模式,目的主机IP地址
	UDP_DIPR[1] = 168;
	UDP_DIPR[2] = 1;
	UDP_DIPR[3] = 190;

	UDP_DPORT[0] = 0x17;		//UDP(广播)模式,目的主机端口号
	UDP_DPORT[1] = 0x70;

	S0_Mode = UDP_MODE;			//加载端口0的工作模式,UDP模式
}

/*******************************************************************************
* 函数名  : W5500_Socket_Set
* 描述    : W5500端口初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 分别设置4个端口,根据端口工作模式,将端口置于TCP服务器、TCP客户端或UDP模式.
*			从端口状态字节Socket_State可以判断端口的工作情况
*******************************************************************************/
void W5500_Socket_Set(void)
{
	if(S0_State == 0)//端口0初始化配置
	{
		if(S0_Mode == TCP_SERVER)//TCP服务器模式 
		{
			if(Socket_Listen(0) == TRUE)
				S0_State = S_INIT;
			else
				S0_State = 0;
		}
		else if(S0_Mode==TCP_CLIENT)//TCP客户端模式 
		{
			if(Socket_Connect(0) == TRUE)
				S0_State = S_INIT;
			else
				S0_State = 0;
		}
		else//UDP模式 
		{
			if(Socket_UDP(0) == TRUE)
				S0_State = S_INIT|S_CONN;
			else
				S0_State = 0;
		}
	}
}

/*******************************************************************************
* 函数名  : Process_Socket_Data
* 描述    : W5500接收并发送接收到的数据
* 输入    : s:端口号
* 输出    : 无
* 返回值  : 无
* 说明    : 本过程先调用S_rx_process()从W5500的端口接收数据缓冲区读取数据,
*			然后将读取的数据从Rx_Buffer拷贝到Temp_Buffer缓冲区进行处理。
*			处理完毕，将数据从Temp_Buffer拷贝到Tx_Buffer缓冲区。调用S_tx_process()
*			发送数据。
*******************************************************************************/
void Process_Socket_Data(SOCKET s)
{
	unsigned short size;
	size = Read_SOCK_Data_Buffer(s, Rx_Buffer);
	memcpy(Tx_Buffer, Rx_Buffer, size);			
	Write_SOCK_Data_Buffer(s, Tx_Buffer, size);
}



