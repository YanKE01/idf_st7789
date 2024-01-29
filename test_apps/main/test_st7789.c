#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "st7789.h"
#include "unity.h"

st7789_config_t st7789_conf = {
    .host = SPI2_HOST,
    .mosi = GPIO_NUM_7,
    .clk = GPIO_NUM_6,
    .cs = GPIO_NUM_5,
    .dc = GPIO_NUM_4,
    .rst = GPIO_NUM_8,
    .bl = GPIO_NUM_9,
};

st7789_dev_t dev;

TEST_CASE("test st7789", "test")
{
    st7789_init(&dev, &st7789_conf);
}

void app_main(void)
{

    unity_run_menu();
}