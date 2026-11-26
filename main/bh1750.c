#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "driver/i2c_master.h"

#include "bh1750.h"
#include "i2c_bus.h"   // I2C_MASTER_TIMEOUT_MS

// Log tag 
static const char *TAG_SENSOR = "BH1750";

// Send command to BH1750
static esp_err_t bh1750_write_command(
    i2c_master_dev_handle_t dev_handle,
    uint8_t command)

{
    return i2c_master_transmit(
        dev_handle,
        &command,
        1,
        I2C_MASTER_TIMEOUT_MS
    );
}

//  Read raw light data from BH1750
//  BH1750 returns 2 bytes of measurement data  
esp_err_t bh1750_read_data(
    i2c_master_dev_handle_t dev_handle,
    uint16_t *raw_data)
{
    uint8_t data[2];

    esp_err_t ret = i2c_master_receive(
        dev_handle,
        data,
        2,
        I2C_MASTER_TIMEOUT_MS
    );

    if (ret != ESP_OK){
        return ret;
    }

    *raw_data = ((uint16_t)data[0] << 8) | data[1];

    return ret;
}


float bh1750_lux(uint16_t raw_data)
{
    return (float)raw_data/1.2f;
}

esp_err_t bh1750_init(
    i2c_master_dev_handle_t dev_handle)
{
    ESP_RETURN_ON_ERROR(
        bh1750_write_command(dev_handle, BH1750_POWER_ON),
        TAG_SENSOR,
        "BH1750 Power ON failed"
    );
    ESP_RETURN_ON_ERROR(
        bh1750_write_command(dev_handle, BH1750_RESET),
        TAG_SENSOR,
        "BH1750 Reset failed"
    );
    ESP_RETURN_ON_ERROR(
        bh1750_write_command(dev_handle, BH1750_CONT_H_RES_MODE),
        TAG_SENSOR,
        "BH1750 mode configuration failed"
    );

    vTaskDelay(pdMS_TO_TICKS(180));

    ESP_LOGI(TAG_SENSOR, "BH1750 initialized successfully");

    return ESP_OK;
}
