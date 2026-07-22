#ifndef LFS_PORT_H
#define LFS_PORT_H

#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  初始化并挂载 LittleFS (推荐入口)
 * @note   如果首次使用或文件系统损坏，此函数会自动执行格式化并重新挂载
 * @return 0: 成功, 负数: 失败 (如 LFS_ERR_IO, LFS_ERR_CORRUPT 等)
 */
int lfs_port_init(void);

/**
 * @brief  强制格式化 LittleFS 分区
 * @warning 此操作会清空该分区内的所有数据！
 * @return 0: 成功, 负数: 失败
 */
int lfs_port_format(void);

/**
 * @brief  卸载 LittleFS，确保所有缓存数据物理写入 Flash
 * @return 0: 成功, 负数: 失败
 */
int lfs_port_unmount(void);

/**
 * @brief  全局 LittleFS 实例
 * @note   业务层可直接使用此变量调用 lfs_file_open, lfs_dir_open 等标准 API
 *         例如: lfs_file_open(&g_lfs, &file, "test.txt", LFS_O_RDWR | LFS_O_CREAT);
 */
extern lfs_t g_lfs;
int save_log_to_flash(const char *log_str);
void read_all_logs(void);
#ifdef __cplusplus
}
#endif

#endif /* LFS_PORT_H */