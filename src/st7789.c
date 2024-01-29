#include "st7789.h"
#include "string.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static bool spi_master_write_byte(st7789_dev_t *dev, const uint8_t *data, size_t length)
{
    spi_transaction_t trans;
    esp_err_t ret;
    if (length > 0)
    {
        memset(&trans, 0, sizeof(spi_transaction_t));
        trans.length = length * 8;
        trans.tx_buffer = data;
    }

    ret = spi_device_transmit(dev->spi_handle, &trans);

    assert(ret == ESP_OK);
    return true;
}

bool st7789_write_cmd(st7789_dev_t *dev, uint8_t cmd)
{
    gpio_set_level(dev->config->dc, 0);
    return spi_master_write_byte(dev, &cmd, 1);
}

bool st7789_write_byte(st7789_dev_t *dev, uint8_t byte)
{
    gpio_set_level(dev->config->dc, 1);
    return spi_master_write_byte(dev, &byte, 1);
}

bool st7789_write_addr(st7789_dev_t *dev, uint16_t addr1, uint16_t addr2)
{
    uint8_t bytes[4];
    bytes[0] = (addr1 >> 8) & 0xFF;
    bytes[1] = addr1 & 0xFF;
    bytes[2] = (addr2 >> 8) & 0xFF;
    bytes[3] = addr2 & 0xFF;
    gpio_set_level(dev->config->dc, 1);
    return spi_master_write_byte(dev, bytes, 4);
}

bool st7789_write_color(st7789_dev_t *dev, uint16_t color, uint16_t size)
{
    static uint8_t bytes[1024];
    int index = 0;
    for (int i = 0; i < size; i++)
    {
        bytes[index++] = (color >> 8) & 0xFF;
        bytes[index++] = color & 0xFF;
    }
    gpio_set_level(dev->config->dc, 1);
    return spi_master_write_byte(dev, bytes, size * 2);
}

void st7789_draw_pixel(st7789_dev_t *dev, uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= dev->width || y >= dev->height)
    {
        return;
    }
    st7789_write_cmd(dev, 0x2A);
    st7789_write_addr(dev, x, x);
    st7789_write_cmd(dev, 0x2B);
    st7789_write_addr(dev, y, y);
    st7789_write_cmd(dev, 0x2C);
    st7789_write_color(dev, color, 1);
}

void st7789_draw_fill_rect(st7789_dev_t *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    if (x1 >= dev->width || y1 >= dev->height)
    {
        return;
    }

    if (x2 >= dev->width)
    {
        x2 = dev->width - 1;
    }

    if (y2 >= dev->height)
    {
        y2 = dev->height - 1;
    }

    st7789_write_cmd(dev, 0x2A);
    st7789_write_addr(dev, x1, x2);
    st7789_write_cmd(dev, 0x2B);
    st7789_write_addr(dev, y1, y2);
    st7789_write_cmd(dev, 0x2C);

    for (int i = x1; i <= x2; i++)
    {
        uint16_t size = y2 - y1 + 1;
        st7789_write_color(dev, color, size);
    }
}

void st7789_fullclean(st7789_dev_t *dev, uint16_t color)
{
    st7789_draw_fill_rect(dev, 0, 0, dev->width - 1, dev->height - 1, color);
}

void st7789_init(st7789_dev_t *dev, st7789_config_t *config, uint16_t width, uint16_t height)
{
    gpio_config_t gpio_conf = {
        .pin_bit_mask = (1ULL << config->dc) | (1ULL << config->rst) | (1ULL << config->bl) | (1ULL << config->cs),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&gpio_conf);
    gpio_set_level(config->rst, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(config->rst, 1);

    /*!< init spi */
    spi_bus_config_t bus_conf = {
        .mosi_io_num = config->mosi,
        .miso_io_num = -1,
        .sclk_io_num = config->clk,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
        .max_transfer_sz = 0,
        .flags = 0,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(config->host, &bus_conf, SPI_DMA_CH_AUTO));
    spi_device_interface_config_t devconf;
    memset(&devconf, 0, sizeof(devconf));
    devconf.clock_speed_hz = SPI_MASTER_FREQ_20M;
    devconf.queue_size = 7;
    devconf.mode = 3;
    devconf.flags = SPI_DEVICE_NO_DUMMY;
    devconf.spics_io_num = config->cs;

    ESP_ERROR_CHECK(spi_bus_add_device(config->host, &devconf, &dev->spi_handle));
    dev->width = width;
    dev->height = height;
    dev->config = config;

    // write command and data
    st7789_write_cmd(dev, 0x01); /*!< software restart */
    vTaskDelay(150 / portTICK_PERIOD_MS);

    st7789_write_cmd(dev, 0x11); /*!< sleep out */
    vTaskDelay(255 / portTICK_PERIOD_MS);

    st7789_write_cmd(dev, 0x3A);
    st7789_write_byte(dev, 0x55); /*!< pixel formate: rgb565 */
    vTaskDelay(10 / portTICK_PERIOD_MS);

    st7789_write_cmd(dev, 0x36);
    st7789_write_byte(dev, 0x00); /*!< memory data access control */

    st7789_write_cmd(dev, 0x2A);
    st7789_write_byte(dev, 0x00);
    st7789_write_byte(dev, 0x00);
    st7789_write_byte(dev, 0x00);
    st7789_write_byte(dev, 0xF0); /*!< column address set: 0~240 */

    st7789_write_cmd(dev, 0x2B);
    st7789_write_byte(dev, 0x00);
    st7789_write_byte(dev, 0x00);
    st7789_write_byte(dev, 0x00);
    st7789_write_byte(dev, 0xF0); /*!< row address set: 0~240 */

    st7789_write_cmd(dev, 0x21); /*!< display inversion on */
    vTaskDelay(10 / portTICK_PERIOD_MS);

    st7789_write_cmd(dev, 0x13); /*!< normal display mode on */
    vTaskDelay(10 / portTICK_PERIOD_MS);

    st7789_write_cmd(dev, 0x29); /*!< display on */
    vTaskDelay(255 / portTICK_PERIOD_MS);

    gpio_set_level(config->bl, 1); /*!< backlight on */
}

esp_err_t st7789_del(st7789_dev_t *dev)
{
    if (dev == NULL)
    {
        return ESP_FAIL;
    }
    ESP_ERROR_CHECK(spi_bus_remove_device(dev->spi_handle));
    ESP_ERROR_CHECK(spi_bus_free(dev->config->host));
    return ESP_OK;
}