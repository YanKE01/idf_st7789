#pragma once

#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#define RED 0xf800
#define GREEN 0x07e0
#define BLUE 0x001f
#define BLACK 0x0000
#define WHITE 0xffff
#define GRAY 0x8410
#define YELLOW 0xFFE0
#define CYAN 0x04FA
#define PURPLE 0x8010

typedef struct
{
	spi_host_device_t host;
	gpio_num_t mosi;
	gpio_num_t clk;
	gpio_num_t cs;
	gpio_num_t dc;
	gpio_num_t rst;
	gpio_num_t bl;
} st7789_config_t;

typedef struct
{
	st7789_config_t *config;
	spi_device_handle_t spi_handle;
	uint16_t width;
	uint16_t height;
} st7789_dev_t;

/**
 * @brief st7789 init
 * 
 * @param dev 
 * @param config 
 * @param width 
 * @param height 
 */
void st7789_init(st7789_dev_t *dev, st7789_config_t *config, uint16_t width, uint16_t height);

/**
 * @brief st7789 delete
 * 
 * @param dev 
 * @return esp_err_t 
 */
esp_err_t st7789_del(st7789_dev_t *dev);

/**
 * @brief st7789 draw fill rect
 * 
 * @param dev 
 * @param x1 
 * @param y1 
 * @param x2 
 * @param y2 
 * @param color 
 */
void st7789_draw_fill_rect(st7789_dev_t *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

/**
 * @brief st7780 draw pixel
 * 
 * @param dev 
 * @param x 
 * @param y 
 * @param color 
 */
void st7789_draw_pixel(st7789_dev_t *dev, uint16_t x, uint16_t y, uint16_t color);


/**
 * @brief sreen clean
 * 
 * @param dev 
 * @param color 
 */
void st7789_fullclean(st7789_dev_t *dev, uint16_t color);