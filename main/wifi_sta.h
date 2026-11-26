#ifndef WIFI_STA_H
#define WIFI_STA_H

// WiFi configuration
#define ESP_WIFI_SSID       "MAM's WIFI"
#define ESP_WIFI_PASSWORD   "24689999"
#define ESP_MAXIMUM_RETRY  5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

void wifi_init_sta(void);

#endif // WIFI_STA_H
