#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "st7789.h"
#include "unity.h"

st7789_dev_t dev;
st7789_config_t config = {
    .bl = 9,
    .rst = 8,
    .dc = 4,
    .cs = 5,
    .clk = 6,
    .mosi = 7,
    .host = SPI2_HOST,
};

st7789_config_t config2 = {
    .bl = 45,
    .rst = 47,
    .dc = 13,
    .cs = 14,
    .clk = 21,
    .mosi = 48,
    .host = SPI2_HOST,
};


TEST_CASE("st7789 full screen test", "[st7789][full screen]")
{
    st7789_init(&dev, &config2, 240, 240);

    uint16_t test_color[] = {RED, GREEN, WHITE, GRAY, BLUE, BLACK};
    for (int i = 0; i < sizeof(test_color) / sizeof(test_color[0]); i++)
    {
        st7789_draw_fill_rect(&dev, 0, 0, 240 - 1, 240 - 1, test_color[i]);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    ESP_ERROR_CHECK(st7789_del(&dev));
}

void app_main(void)
{

    unity_run_menu();
}