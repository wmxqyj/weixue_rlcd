#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_bsp.h"
#include "nvs.h"
#include "nvs_flash.h"

#if __has_include("wifi_credentials.local.h")
#include "wifi_credentials.local.h"
#else
#define WEIXUE_WIFI_SSID ""
#define WEIXUE_WIFI_PASSWORD ""
#endif

#define WIFI_EVENT_STARTED      (1U << 0)
#define WIFI_EVENT_SCAN_DONE    (1U << 1)
#define WIFI_EVENT_GOT_IP       (1U << 2)
#define WIFI_EVENT_DISCONNECTED (1U << 3)

static const char *TAG = "weixue_wifi";
static const char *NVS_NAMESPACE = "weixue_wifi";

EventGroupHandle_t wifi_even_ = NULL;
esp_bsp_t user_esp_bsp;

static esp_netif_t *net = NULL;
static char current_ssid[33] = WEIXUE_WIFI_SSID;
static char current_password[65] = WEIXUE_WIFI_PASSWORD;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);
static void wifi_manager_task(void *arg);

static void load_credentials(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) {
        return;
    }

    size_t ssid_length = sizeof(current_ssid);
    if (nvs_get_str(handle, "ssid", current_ssid, &ssid_length) != ESP_OK) {
        nvs_set_str(handle, "ssid", current_ssid);
    }
    size_t password_length = sizeof(current_password);
    if (nvs_get_str(handle, "password", current_password,
                    &password_length) != ESP_OK) {
        nvs_set_str(handle, "password", current_password);
    }
    nvs_commit(handle);
    nvs_close(handle);
}

void espwifi_init(void)
{
    memset(&user_esp_bsp, 0, sizeof(user_esp_bsp));
    wifi_even_ = xEventGroupCreate();

    esp_err_t nvs_status = nvs_flash_init();
    if (nvs_status == ESP_ERR_NVS_NO_FREE_PAGES ||
        nvs_status == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_status = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_status);
    load_credentials();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    net = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.sta.ssid, current_ssid,
            sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, current_password,
            sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));
    ESP_LOGI(TAG, "WiFi minimum modem-sleep enabled");

    xTaskCreatePinnedToCore(wifi_manager_task, "wifi_manager", 4096, NULL, 3,
                            NULL, 0);
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    (void)arg;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        xEventGroupSetBits(wifi_even_, WIFI_EVENT_STARTED);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t *event = (const ip_event_got_ip_t *)event_data;
        const uint32_t address = event->ip_info.ip.addr;
        snprintf(user_esp_bsp._ip, sizeof(user_esp_bsp._ip), "%u.%u.%u.%u",
                 (uint8_t)address, (uint8_t)(address >> 8),
                 (uint8_t)(address >> 16), (uint8_t)(address >> 24));

        wifi_ap_record_t ap_info = {0};
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            user_esp_bsp.rssi = ap_info.rssi;
        }
        user_esp_bsp.connected = true;
        ESP_LOGI(TAG, "Connected to %s, IP=%s, RSSI=%d dBm",
                 current_ssid, user_esp_bsp._ip, user_esp_bsp.rssi);
        xEventGroupSetBits(wifi_even_, WIFI_EVENT_GOT_IP);
    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
        const wifi_event_sta_disconnected_t *event =
            (const wifi_event_sta_disconnected_t *)event_data;
        user_esp_bsp.connected = false;
        user_esp_bsp._ip[0] = '\0';
        user_esp_bsp.rssi = 0;
        ESP_LOGW(TAG, "Disconnected from %s, reason=%u",
                 current_ssid, event->reason);
        xEventGroupSetBits(wifi_even_, WIFI_EVENT_DISCONNECTED);
    }
}

static void wifi_manager_task(void *arg)
{
    (void)arg;
    uint16_t ap_count = 0;
    EventBits_t events = xEventGroupWaitBits(
        wifi_even_, WIFI_EVENT_STARTED, pdTRUE, pdTRUE, pdMS_TO_TICKS(15000));

    if (events & WIFI_EVENT_STARTED) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_scan_start(NULL, true));
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_scan_get_ap_num(&ap_count));
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_clear_ap_list());
    }
    user_esp_bsp.apNum = (ap_count > INT8_MAX) ? INT8_MAX : (int8_t)ap_count;
    xEventGroupSetBits(wifi_even_, WIFI_EVENT_SCAN_DONE);

    if (current_ssid[0] == '\0') {
        ESP_LOGW(TAG, "No local WiFi credentials; scan-only mode");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Connecting to %s", current_ssid);
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());

    uint32_t retry_count = 0;
    for (;;) {
        events = xEventGroupWaitBits(
            wifi_even_, WIFI_EVENT_GOT_IP | WIFI_EVENT_DISCONNECTED,
            pdTRUE, pdFALSE, portMAX_DELAY);
        if (events & WIFI_EVENT_GOT_IP) {
            retry_count = 0;
        }
        if (events & WIFI_EVENT_DISCONNECTED) {
            ++retry_count;
            const uint32_t delay_ms = retry_count < 5 ? 2000 : 10000;
            ESP_LOGI(TAG, "Reconnect attempt %lu in %lu ms",
                     (unsigned long)retry_count, (unsigned long)delay_ms);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
        }
    }
}

void espwifi_deinit(void)
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_stop());
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_deinit());
    if (net != NULL) {
        esp_netif_destroy_default_wifi(net);
        net = NULL;
    }
}

bool espwifi_is_connected(void)
{
    return user_esp_bsp.connected;
}

void espwifi_get_ssid(char *ssid, size_t capacity)
{
    if (ssid == NULL || capacity == 0) {
        return;
    }
    strlcpy(ssid, current_ssid, capacity);
}

esp_err_t espwifi_update_credentials(const char *ssid, const char *password)
{
    if (ssid == NULL || password == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    const size_t ssid_length = strlen(ssid);
    const size_t password_length = strlen(password);
    if (ssid_length == 0 || ssid_length > 32 ||
        (password_length != 0 &&
         (password_length < 8 || password_length > 63))) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle;
    esp_err_t status = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (status != ESP_OK) {
        return status;
    }
    status = nvs_set_str(handle, "ssid", ssid);
    if (status == ESP_OK) {
        status = nvs_set_str(handle, "password", password);
    }
    if (status == ESP_OK) {
        status = nvs_commit(handle);
    }
    nvs_close(handle);
    if (status != ESP_OK) {
        return status;
    }

    strlcpy(current_ssid, ssid, sizeof(current_ssid));
    strlcpy(current_password, password, sizeof(current_password));

    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.sta.ssid, current_ssid,
            sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, current_password,
            sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    user_esp_bsp.connected = false;
    user_esp_bsp._ip[0] = '\0';
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_disconnect());
    status = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (status == ESP_OK) {
        status = esp_wifi_connect();
    }
    return status;
}
