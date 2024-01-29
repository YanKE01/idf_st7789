#include "st7789.h"
#include "string.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void st7789_test()
{
    printf("test\n");
}

static bool spi_master_write_byte(st7789_dev_t *dev, const uint8_t *data, size_t length)
{
    spi_transaction_t trans;
    esp_err_t ret;
    if (length > 0)
    {
        memset(&trans, 0, sizeof(trans));
        trans.length = length;
        trans.tx_buffer = data;
    }

    ret = spi_device_transmit(dev->spi_handle, &trans);

    assert(ret == ESP_OK);
    return true;
}

bool st7789_write_cmd(st7789_dev_t *dev, uint8_t cmd)
{
    gpio_set_level(dev->config.dc, 0);
    return spi_master_write_byte(dev, &cmd, 1);
}

bool st7789_write_byte(st7789_dev_t *dev, uint8_t byte)
{
    gpio_set_level(dev->config.dc, 1);
    return spi_master_write_byte(dev, &byte, 1);
}

st7789_dev_t st7789_init(st7789_dev_t *dev, st7789_config_t *config)
{
    /*!< init gpio */
    gpio_config_t gpio_conf =
        {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pin_bit_mask = (1ULL << config->cs) | (1ULL << config->dc) | (1ULL << config->rst) | (1ULL << config->bl),
        };
    gpio_config(&gpio_conf);

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

    st7789_write_cmd(dev, 0x01); /*!< software restart */
    vTaskDelay(150 / portTICK_PERIOD_MS);

    st7789_write_cmd(dev, 0x11); /*!< sleep out */
    vTaskDelay(200 / portTICK_PERIOD_MS);

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

    return *dev;
}
