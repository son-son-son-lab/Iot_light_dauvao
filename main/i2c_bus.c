#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#include "i2c_bus.h"
#include "bh1750.h"   // BH1750_SENSOR_ADDR

//Initialize I2C master and BH1750 device

void i2c_master_init(
    i2c_master_bus_handle_t *bus_handle,
    i2c_master_dev_handle_t *dev_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK_WITHOUT_ABORT(
        i2c_new_master_bus(&bus_config, bus_handle)
    );

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BH1750_SENSOR_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK_WITHOUT_ABORT(
        i2c_master_bus_add_device(
            *bus_handle,
            &dev_config,
            dev_handle
        )
    );
}
