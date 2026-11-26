#ifndef I2C_BUS_H
#define I2C_BUS_H

#include "driver/i2c_master.h"

// I2C configuration 
#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           21
#define I2C_MASTER_NUM              0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TIMEOUT_MS       1000

//Initialize I2C master and BH1750 device
void i2c_master_init(
    i2c_master_bus_handle_t *bus_handle,
    i2c_master_dev_handle_t *dev_handle);

#endif // I2C_BUS_H
