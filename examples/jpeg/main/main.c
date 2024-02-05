#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "st7789.h"
#include "esp_spiffs.h"
#include "decode_jpeg.h"

static const char *TAG = "main";
#define rgb565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

st7789_dev_t dev;
st7789_config_t config = {
    .bl = 45,
    .rst = 47,
    .dc = 13,
    .cs = 14,
    .clk = 21,
    .mosi = 48,
    .host = SPI2_HOST,
};

void app_main()
{
    /*!< st7789 init */
    st7789_init(&dev, &config, 240, 240);
    st7789_fullclean(&dev,BLACK);
    
    /*!< spiffs init */
    esp_err_t ret;
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true, // 如果挂载失败，将格式化文件系统
    };

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    // check spiffs
    ret = esp_spiffs_check(conf.partition_label);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "SPIFFS Check failed:%s", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "SPIFFS Check success");
    }

    // get spiffs info
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Failed to get spiffs partition info:%s", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total:%d,used:%d ", total, used);
    }

    // if used > total, perform spiffs check
    if (used > total)
    {
        ESP_LOGW(TAG, "Number of used bytes cannot be larger than total. Performing SPIFFS_check().");
        ret = esp_spiffs_check(conf.partition_label);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
            return;
        }
        else
        {
            ESP_LOGI(TAG, "SPIFFS_check() successful");
        }
    }

    /*!< decode image */
    uint16_t **pixels;
    int imageWidth;
    int imageHeight;
    esp_err_t err = decode_jpeg(&pixels, "/spiffs/image.jpg", dev.width, dev.height, &imageWidth, &imageHeight);
    ESP_LOGI(__FUNCTION__, "decode_image err=%d imageWidth=%d imageHeight=%d", err, imageWidth, imageHeight);
    if (err == ESP_OK)
    {
        uint16_t _width = dev.width;
        uint16_t _cols = 0;
        if (dev.width > imageWidth)
        {
            _width = imageWidth;
            _cols = (dev.width - imageWidth) / 2;
        }
        ESP_LOGI(__FUNCTION__, "_width=%d _cols=%d", _width, _cols);

        uint16_t _height = dev.height;
        uint16_t _rows = 0;
        if (dev.height > imageHeight)
        {
            _height = imageHeight;
            _rows = (dev.height - imageHeight) / 2;
        }
        ESP_LOGI(__FUNCTION__, "_height=%d _rows=%d", _height, _rows);
        uint16_t *colors = (uint16_t *)malloc(sizeof(uint16_t) * _width);

        for (int y = 0; y < _height; y++)
        {
            for (int x = 0; x < _width; x++)
            {
                pixel_jpeg pixel = pixels[y][x];
                st7789_draw_pixel(&dev, x + _cols, y + _rows, pixel);
            }
        }

        free(colors);
        release_image(&pixels, dev.width, dev.height);
    }
    else
    {
        ESP_LOGE(__FUNCTION__, "decode_image failed");
    }
}