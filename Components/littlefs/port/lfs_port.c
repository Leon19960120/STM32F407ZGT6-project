/*
 * LittleFS W25Qxx 移植优化版
 * 修复了整片擦除的致命 Bug，优化了参数，并增加了自动初始化逻辑。
 */

#include "lfs_port.h"
//#include "driver_w25qxx.h"
#include "driver_w25qxx_advance.h" 
#include <string.h>

// ========== 硬件参数配置 ==========
#define LFS_FLASH_BASE_ADDR   0x00000000  // LittleFS 分区起始地址

// 注意：请根据实际 Flash 容量调整 BLOCK_COUNT！
// W25Q16 (2MB) = 512, W25Q32 (4MB) = 1024, W25Q64 (8MB) = 2048, W25Q128 (16MB) = 4096
#define LFS_BLOCK_SIZE        4096        // 块大小 = Flash 最小擦除单位 (4KB)
#define LFS_BLOCK_COUNT       512         // 总块数 (以 2MB 为例)

#define LFS_READ_SIZE         256         // 读取大小 = Flash 页大小
#define LFS_PROG_SIZE         256         // 写入大小 = Flash 页大小
#define LFS_LOOKAHEAD_SIZE    (LFS_BLOCK_COUNT / 8) // 磨损均衡前瞻缓存 (必须是 8 的倍数, 512/8 = 64)
// ========================================================

// 全局 LittleFS 实例与配置结构体
lfs_t g_lfs;
static struct lfs_config g_lfs_cfg;

// 静态缓存（静态分配，避免堆内存碎片）
static uint8_t read_buf[LFS_READ_SIZE];
static uint8_t prog_buf[LFS_PROG_SIZE];
static uint8_t lookahead_buf[LFS_LOOKAHEAD_SIZE];

// ========== 4 个底层硬件回调函数 ==========

static int lfs_flash_read(const struct lfs_config *c, lfs_block_t block,
                          lfs_off_t off, void *buffer, lfs_size_t size)
{
    uint32_t addr = LFS_FLASH_BASE_ADDR + block * c->block_size + off;
    uint8_t res = w25qxx_advance_read(addr, (uint8_t *)buffer, size);
    return (res == 0) ? 0 : LFS_ERR_IO; // 推荐使用 LittleFS 标准错误码
}

static int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block,
                          lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint32_t addr = LFS_FLASH_BASE_ADDR + block * c->block_size + off;   
    uint8_t res = w25qxx_advance_page_program(addr, (uint8_t *)buffer, size);
    return (res == 0) ? 0 : LFS_ERR_IO;
}

static int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block)
{
    uint32_t addr = LFS_FLASH_BASE_ADDR + block * c->block_size;
    
    // 【致命 Bug 修复】：必须使用 4KB 扇区擦除，绝不能使用整片擦除 (chip_erase)！
    // 请确保你的驱动中有类似 w25qxx_basic_sector_erase 的函数
    uint8_t res = w25qxx_advance_sector_erase_4k(addr); 
    
    return (res == 0) ? 0 : LFS_ERR_IO;
}

static int lfs_flash_sync(const struct lfs_config *c)
{
    // W25Qxx 驱动内部通常自带忙等待 (Wait Busy)，此处直接返回成功即可
    // 如果驱动是异步的，需要在这里加入等待 Flash 空闲的逻辑
    return 0;
}

// ========== 线程安全回调 (可选，针对 RTOS) ==========
/*
static int lfs_flash_lock(const struct lfs_config *c) {
    // xSemaphoreTake(mutex, portMAX_DELAY);
    return 0;
}

static int lfs_flash_unlock(const struct lfs_config *c) {
    // xSemaphoreGive(mutex);
    return 0;
}
*/

// ========== 对外通用接口 ==========

/**
 * @brief  初始化配置并挂载 LittleFS (推荐使用的入口函数)
 * @note   如果挂载失败（如首次使用或数据损坏），会自动格式化后重新挂载
 * @return 0 成功，负数失败
 */
int lfs_port_init(void)
{   // 【绝对关键】：在这里初始化 Advance 驱动的全局句柄！
    // 参数：芯片型号, 接口类型(标准SPI), 是否开启双/四线(关闭)
    if (w25qxx_advance_init(W25Q16, W25QXX_INTERFACE_SPI, W25QXX_BOOL_FALSE) != 0) {
        printf("[ERROR] W25Q16 Advance Init Failed!\r\n");
        return -1;
    }
    printf("[INFO] W25Q16 Hardware Init Success.\r\n");

    // 1. 清空并配置结构体
    memset(&g_lfs_cfg, 0, sizeof(g_lfs_cfg));
    
    g_lfs_cfg.read  = lfs_flash_read;
    g_lfs_cfg.prog  = lfs_flash_prog;
    g_lfs_cfg.erase = lfs_flash_erase;
    g_lfs_cfg.sync  = lfs_flash_sync;
    // g_lfs_cfg.lock  = lfs_flash_lock;   // 如果使用 RTOS，请取消注释
    // g_lfs_cfg.unlock = lfs_flash_unlock;
    
    g_lfs_cfg.read_size     = LFS_READ_SIZE;
    g_lfs_cfg.prog_size     = LFS_PROG_SIZE;
    g_lfs_cfg.block_size    = LFS_BLOCK_SIZE;
    g_lfs_cfg.block_count   = LFS_BLOCK_COUNT;
    // 必须加这一行，不能为 0
    g_lfs_cfg.cache_size     = 256;  // 或直接写 512
    g_lfs_cfg.lookahead_size = LFS_LOOKAHEAD_SIZE;
    g_lfs_cfg.block_cycles  = 500;          // 建议添加：块擦除周期限制，有助于磨损均衡 (通常 100~1000)
    
    g_lfs_cfg.read_buffer      = read_buf;
    g_lfs_cfg.prog_buffer      = prog_buf;
    g_lfs_cfg.lookahead_buffer = lookahead_buf;
    
    // 2. 尝试挂载
    int err = lfs_mount(&g_lfs, &g_lfs_cfg);
    if (err == 0) {
        printf("挂载成功\r\n");
        printf("[INFO] LittleFS Mount Success.\r\n");
        return 0; // 挂载成功
       
    }
    
    // 3. 挂载失败，尝试格式化
    // 注意：格式化会清空该分区所有数据！
    printf("[INFO] Mount Failed, Formatting...\r\n");
    err = lfs_format(&g_lfs, &g_lfs_cfg);
    if (err != 0) {
        // 打印出具体的负数错误码 (例如 -5 代表 IO 错误，-84 代表挂载/损坏错误)
        printf("[ERROR] Format Failed! Code: %d\r\n", err);
        return err; // 格式化失败，硬件可能存在问题
        
    }
    
    // 4. 格式化后再次挂载
    return lfs_mount(&g_lfs, &g_lfs_cfg);
    if (err ==0) {
        printf("[INFO] Format & Mount Success.\r\n");
    }
}

/**
 * @brief  仅格式化 Flash 为 LittleFS 格式 (手动调用)
 */
int lfs_port_format(void)
{
    return lfs_format(&g_lfs, &g_lfs_cfg);
}

/**
 * @brief  卸载文件系统，确保数据全部落盘
 */
int lfs_port_unmount(void)
{
    return lfs_unmount(&g_lfs);
}

// ==========================================
// 日志读写业务函数实现
// ==========================================

/**
 * @brief  向 Flash 中追加一条日志
 * @param  log_str: 要写入的日志字符串内容
 * @return 0: 成功, 负数: 失败 (如 -28 LFS_ERR_NOSPC 表示空间不足)
 */
int save_log_to_flash(const char *log_str)
{
    lfs_file_t log_file;
    int res;
    char log_buf[256];
    
    if (log_str == NULL) {
        return -1;
    }

    // 1. 打开日志文件
    // 标志位说明:
    // LFS_O_WRONLY : 只写模式
    // LFS_O_CREAT  : 如果文件不存在则自动创建
    // LFS_O_APPEND : 追加模式 (每次写入都会自动移动到文件末尾，不会覆盖旧日志)
    res = lfs_file_open(&g_lfs, &log_file, "system.log", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);
    if (res < 0) {
        printf("[LOG ERR] Open file failed: %d\r\n", res);
        return res;
    }
    
    // 2. 格式化日志内容 (自动在末尾添加换行符，方便后续查看)
    // 如果你的系统有 RTC，可以改成: snprintf(log_buf, sizeof(log_buf), "[2026-07-22 12:00:00] %s\r\n", log_str);
    int len = snprintf(log_buf, sizeof(log_buf), "%s\r\n", log_str);
    
    // 3. 写入 Flash
    res = lfs_file_write(&g_lfs, &log_file, log_buf, len);
    if (res < 0) {
        printf("[LOG ERR] Write file failed: %d\r\n", res);
        lfs_file_close(&g_lfs, &log_file);
        return res;
    }
    
    // 4. 关闭文件 (LittleFS 会在 close 时确保元数据落盘)
    lfs_file_close(&g_lfs, &log_file);
    
    return 0;
}

/**
 * @brief  读取 Flash 中的所有日志并打印到串口
 * @note   采用分块读取，避免日志过大时占用过多 RAM
 */
void read_all_logs(void)
{
    lfs_file_t log_file;
    int res;
    char read_buf[128]; // 每次读取 128 字节，兼顾效率和栈空间
    
    // 1. 以只读模式打开文件
    res = lfs_file_open(&g_lfs, &log_file, "system.log", LFS_O_RDONLY);
    if (res < 0) {
        printf("--- No logs found (File not exist or error: %d) ---\r\n", res);
        return;
    }
    
    printf("\r\n========== Flash Logs Start ==========\r\n");
    
    // 2. 循环分块读取并打印
    while (1) {
        // 每次最多读取 sizeof(read_buf) - 1 字节，留出 1 个字节给 '\0'
        res = lfs_file_read(&g_lfs, &log_file, read_buf, sizeof(read_buf) - 1);
        
        if (res <= 0) {
            break; // 读取完毕 (res == 0) 或发生错误 (res < 0)
        }
        
        // 3. 添加字符串结束符，防止 printf 乱码
        read_buf[res] = '\0'; 
        
        // 4. 打印到串口
        printf("%s", read_buf);
    }
    
    printf("========== Flash Logs End ==========\r\n\r\n");
    
    // 5. 关闭文件
    lfs_file_close(&g_lfs, &log_file);
}