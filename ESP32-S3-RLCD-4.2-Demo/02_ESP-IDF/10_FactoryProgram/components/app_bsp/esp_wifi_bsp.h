#ifndef ESP_WIFI_BSP_H
#define ESP_WIFI_BSP_H

#include <stdbool.h>
#include <stddef.h>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

extern EventGroupHandle_t wifi_even_;

typedef struct
{
    char _ip[25];
    int8_t rssi;
    int8_t apNum;
    bool connected;
} esp_bsp_t;

extern esp_bsp_t user_esp_bsp;

#ifdef __cplusplus
extern "C" {
#endif

void espwifi_init(void);
void espwifi_deinit(void);
bool espwifi_is_connected(void);
void espwifi_get_ssid(char *ssid, size_t capacity);
esp_err_t espwifi_update_credentials(const char *ssid, const char *password);

#ifdef __cplusplus
}
#endif

#endif
