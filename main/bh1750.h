#ifndef BH1750_H
#define BH1750_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

// BH1750 I2C address
#define BH1750_SENSOR_ADDR          0x23

// BH1750 commands
#define BH1750_POWER_OFF             0x00
#define BH1750_POWER_ON              0x01
#define BH1750_RESET                 0x07
#define BH1750_CONT_H_RES_MODE       0x10

// Measurement interval
#define BH1750_MEASURE_INTERVAL_MS   500

//  Read raw light data from BH1750
esp_err_t bh1750_read_data(
    i2c_master_dev_handle_t dev_handle,
    uint16_t *raw_data);

float bh1750_lux(uint16_t raw_data);

esp_err_t bh1750_init(
    i2c_master_dev_handle_t dev_handle);

#endif // BH1750_H
