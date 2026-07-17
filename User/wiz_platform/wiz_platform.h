#ifndef __WIZ_PLATFORM_H__
#define __WIZ_PLATFORM_H__

#include <stdint.h>
#include "main.h"


#define WIZ_RST_PIN GPIO_Pin_8
#define WIZ_RST_PORT GPIOD
#define WIZ_INT_PIN GPIO_Pin_9
#define WIZ_INT_PORT GPIOD
#define WIZ_SCS_PIN GPIO_Pin_7
#define WIZ_SCS_PORT GPIOD
#define WIZ_SCK_PIN GPIO_Pin_13
#define WIZ_SCK_PORT GPIOB
#define WIZ_MISO_PIN GPIO_Pin_14
#define WIZ_MISO_PORT GPIOB
#define WIZ_MOSI_PIN GPIO_Pin_15
#define WIZ_MOSI_PORT GPIOB

/**
 * @brief   delay init
 * @param   none
 * @return  none
 */
void delay_init(void);

/**
 * @brief   delay us
 * @param   none
 * @return  none
 */
void delay_us(uint32_t nus);

/**
 * @brief   delay ms
 * @param   none
 * @return  none
 */
void delay_ms(uint32_t nms);

/**
 * @brief   delay s
 * @param   none
 * @return  none
 */
void delay_s(uint32_t ns);

/**
 * @brief   debug usart init
 * @param   none
 * @return  none
 */
void debug_usart_init(void);

/**
 * @brief   wiz timer init
 * @param   none
 * @return  none
 */
void wiz_timer_init(void);

/**
 * @brief   wiz spi init
 * @param   none
 * @return  none
 */
void wiz_spi_init(void);

/**
 * @brief   wiz rst and int pin init
 * @param   none
 * @return  none
 */
void wiz_rst_int_init(void);

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

/**
 * @brief   Turn on wiz timer interrupt
 * @param   none
 * @return  none
 */
void wiz_tim_irq_enable(void);

/**
 * @brief   Turn off wiz timer interrupt
 * @param   none
 * @return  none
 */
void wiz_tim_irq_disable(void);
#endif
