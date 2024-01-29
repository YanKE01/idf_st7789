#pragma once

#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

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
    st7789_config_t config;
    spi_device_handle_t spi_handle;
} st7789_dev_t;

st7789_dev_t st7789_init(st7789_dev_t *dev, st7789_config_t *config);

void st7789_test();