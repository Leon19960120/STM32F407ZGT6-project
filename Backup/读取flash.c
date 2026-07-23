//   printf("\r\n========================================\r\n");
//   printf("  W25Q16 (LibDriver) Test Start\r\n");
//   printf("========================================\r\n");
//   // 2. 【极其重要】先将整个结构体清零，防止野指针或垃圾数据
//   memset(&g_w25qxx_handle, 0, sizeof(w25qxx_handle_t));

//   // 3. 绑定你之前写好的底层接口函数 (回调函数)
//   g_w25qxx_handle.spi_qspi_init       = w25qxx_interface_spi_qspi_init;
//   g_w25qxx_handle.debug_print         = w25qxx_interface_debug_print;
//   g_w25qxx_handle.spi_qspi_deinit     = w25qxx_interface_spi_qspi_deinit;
//   g_w25qxx_handle.spi_qspi_write_read = w25qxx_interface_spi_qspi_write_read;
//   g_w25qxx_handle.delay_ms            = w25qxx_interface_delay_ms;
//   g_w25qxx_handle.delay_us            = w25qxx_interface_delay_us;
//   // 4. 配置硬件参数 (源码中会检查这些值)
//   g_w25qxx_handle.spi_qspi = W25QXX_INTERFACE_SPI; // 告诉驱动：我用的是标准 SPI，不是 QSPI
//   g_w25qxx_handle.type     = W25Q16;               // ⚠️ 告诉驱动：我的芯片型号是 W25Q16！

//   g_w25qxx_handle.address_mode       = W25QXX_ADDRESS_MODE_3_BYTE;  // 3字节地址（普通Flash都用这个）
//   g_w25qxx_handle.dual_quad_spi_enable = 0;                         // 标准SPI，不用双线/四线
//   g_w25qxx_handle.dummy              = 0;                           // 标准SPI不需要dummy周期
//   g_w25qxx_handle.param              = 0;                           // 保留参数，默认0即可

//   // 5. 初始化 W25Qxx 驱动 (内部会自动调用你写的 w25qxx_interface_spi_qspi_init)
//   if (w25qxx_init(&g_w25qxx_handle) != 0) {
//       printf("[ERROR] W25Q16 init failed! Check SPI wiring or CS pin.\r\n");
//       while(1); // 初始化失败则卡死，方便排查
//   }
//   printf("[INFO] W25Q16 init success!\r\n");
