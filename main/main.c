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

// Log tag 
static const char *TAG = "BH1750";

// WiFi configuration
#define ESP_WIFI_SSID       "2007"
#define ESP_WIFI_PASSWORD   "2007"
#define ESP_MAXIMUM_RETRY  5

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

// I2C configuration 
#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           21
#define I2C_MASTER_NUM              0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TIMEOUT_MS       1000

// BH1750 I2C address
#define BH1750_SENSOR_ADDR          0x23

// BH1750 commands
#define BH1750_POWER_OFF             0x00
#define BH1750_POWER_ON              0x01
#define BH1750_RESET                 0x07
#define BH1750_CONT_H_RES_MODE       0x10

// Measurement interval
#define BH1750_MEASURE_INTERVAL_MS   500


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASSWORD);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASSWORD);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}



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
static esp_err_t bh1750_read_data(
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


//Initialize I2C master and BH1750 device

static void i2c_master_init(
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

static esp_err_t bh1750_init(
    i2c_master_dev_handle_t dev_handle)
{
    ESP_RETURN_ON_ERROR(
        bh1750_write_command(dev_handle, BH1750_POWER_ON),
        TAG,
        "BH1750 Power ON failed"
    );
    ESP_RETURN_ON_ERROR(
        bh1750_write_command(dev_handle, BH1750_RESET),
        TAG,
        "BH1750 Reset failed"
    );
    ESP_RETURN_ON_ERROR(
        bh1750_write_command(dev_handle, BH1750_CONT_H_RES_MODE),
        TAG,
        "BH1750 mode configuration failed"
    );

    vTaskDelay(pdMS_TO_TICKS(180));

    ESP_LOGI(TAG, "BH1750 initialized successfully");

    return ESP_OK;
}

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

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    wifi_init_sta();

    i2c_master_init(&bus_handle, &dev_handle);

    ESP_LOGI(TAG, "I2C initialized successfully");

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
            ESP_LOGI(TAG,"Raw Data: %d | Illuminance: %.2f lux",raw,lux);
            
            printf("{\"status\":\"OK\",\"lux\":%.2f}\n",lux);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read BH1750: %s",
                     esp_err_to_name(ret1));
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(BH1750_MEASURE_INTERVAL_MS));
    }
}
