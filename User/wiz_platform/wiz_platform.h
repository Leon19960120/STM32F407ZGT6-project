#ifndef __WIZ_PLATFORM_H__
#define __WIZ_PLATFORM_H__

#include <stdint.h>
//#include "main.h"
//#include <stdio.h>      // 必须包含，否则 FILE 会报错







/**
 * @brief   hardware reset wizchip
 * @param   none
 * @return  none
 */
void wizchip_reset(void);

/**
 * @brief   Register the WIZCHIP SPI callback function
 * @param   none
 * @return  none
 */
void wizchip_spi_cb_reg(void);






#endif
