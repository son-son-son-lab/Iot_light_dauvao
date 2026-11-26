#include <stdio.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include <string.h>
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_sta.h"
#include "i2c_bus.h"
#include "bh1750.h"

// Log tag 
static const char *TAG_SENSOR = "BH1750";
static const char *TAG_WIFI = "wifi station";

void app_main(void)
{
    uint16_t raw;
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if (CONFIG_LOG_MAXIMUM_LEVEL > CONFIG_LOG_DEFAULT_LEVEL) {
        esp_log_level_set("wifi", CONFIG_LOG_MAXIMUM_LEVEL);
    }

    ESP_LOGI(TAG_WIFI, "ESP_WIFI_MODE_STA");

    wifi_init_sta();

    i2c_master_init(&bus_handle, &dev_handle);

    ESP_LOGI(TAG_SENSOR, "I2C initialized successfully");

    ESP_ERROR_CHECK_WITHOUT_ABORT(bh1750_init(dev_handle));

    while (1)
    {
        esp_err_t ret1 = bh1750_read_data(
            dev_handle,
            &raw
        );

        if (ret1 == ESP_OK)
        {
            float lux = bh1750_lux(raw);
            ESP_LOGI(TAG_SENSOR,"Raw Data: %d | Illuminance: %.2f lux",raw,lux);
            
            printf("{\"status\":\"OK\",\"lux\":%.2f}\n",lux);
        }
        else
        {
            ESP_LOGE(TAG_SENSOR, "Failed to read BH1750: %s",
                     esp_err_to_name(ret1));
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(BH1750_MEASURE_INTERVAL_MS));
    }
}
