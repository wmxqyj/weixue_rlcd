#include "app_services.h"

#include <algorithm>
#include <ctype.h>
#include <iterator>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_sleep.h"
#include "esp_wifi_bsp.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "i2c_equipment.h"
#include "nvs.h"
#include "sdcard_bsp.h"

namespace {

constexpr EventBits_t kRefreshRequested = (1U << 0);
constexpr EventBits_t kWebStartRequested = (1U << 1);
constexpr EventBits_t kWebStopRequested = (1U << 2);
constexpr TickType_t kWebIdleTimeout = pdMS_TO_TICKS(10 * 60 * 1000);
constexpr TickType_t kManualWakeOverride = pdMS_TO_TICKS(30 * 60 * 1000);
constexpr TickType_t kMarketTradingRefresh = pdMS_TO_TICKS(60 * 1000);
constexpr TickType_t kMarketIdleRefresh = pdMS_TO_TICKS(15 * 60 * 1000);
constexpr size_t kHttpResponseCapacity = 24 * 1024;
constexpr size_t kPostBodyCapacity = 1024;

const char *kTag = "app_services";
const char *kConfigNamespace = "weixue_app";
const char *kMarketCachePath = "/sdcard/cache/market.json";

struct AppConfig {
    char stocks[APP_MARKET_ITEM_COUNT][20];
    char stock_names[APP_MARKET_ITEM_COUNT][32];
    bool sleep_schedule_enabled;
    uint8_t sleep_hour;
    uint8_t sleep_minute;
    uint8_t wake_hour;
    uint8_t wake_minute;
};

struct HttpBuffer {
    char *data;
    size_t length;
    size_t capacity;
    bool overflow;
};

SemaphoreHandle_t s_mutex = nullptr;
EventGroupHandle_t s_events = nullptr;
CustomSDPort *s_sd = nullptr;
AppConfig s_config = {};
AppServiceSnapshot s_snapshot = {};
httpd_handle_t s_http_server = nullptr;
TickType_t s_web_last_activity = 0;
bool s_web_desired = true;
bool s_sntp_initialized = false;
bool s_ntp_synced_this_boot = false;
TickType_t s_schedule_override_until = 0;

void StopWebServer();

void Lock()
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
}

void Unlock()
{
    xSemaphoreGive(s_mutex);
}

void SetDefaultConfig()
{
    strlcpy(s_config.stocks[0], "000001.SS", sizeof(s_config.stocks[0]));
    strlcpy(s_config.stocks[1], "000688.SS", sizeof(s_config.stocks[1]));
    strlcpy(s_config.stocks[2], "601899.SS", sizeof(s_config.stocks[2]));
    strlcpy(s_config.stocks[3], "001203.SZ", sizeof(s_config.stocks[3]));
    strlcpy(s_config.stock_names[0], "上证",
            sizeof(s_config.stock_names[0]));
    strlcpy(s_config.stock_names[1], "科创",
            sizeof(s_config.stock_names[1]));
    strlcpy(s_config.stock_names[2], "紫金矿业",
            sizeof(s_config.stock_names[2]));
    strlcpy(s_config.stock_names[3], "大中矿业",
            sizeof(s_config.stock_names[3]));
    s_config.sleep_schedule_enabled = true;
    s_config.sleep_hour = 22;
    s_config.sleep_minute = 0;
    s_config.wake_hour = 8;
    s_config.wake_minute = 30;
}

void ReadNvsString(nvs_handle_t handle, const char *key, char *value,
                   size_t capacity)
{
    size_t length = capacity;
    if (nvs_get_str(handle, key, value, &length) != ESP_OK) {
        nvs_set_str(handle, key, value);
    }
}

void LoadConfig()
{
    SetDefaultConfig();
    nvs_handle_t handle;
    if (nvs_open(kConfigNamespace, NVS_READWRITE, &handle) != ESP_OK) {
        return;
    }
    uint32_t config_schema = 0;
    nvs_get_u32(handle, "schema", &config_schema);
    if (config_schema < 5) {
        for (size_t i = 0; i < APP_MARKET_ITEM_COUNT; ++i) {
            char key[12];
            snprintf(key, sizeof(key), "stock%u", static_cast<unsigned>(i));
            nvs_set_str(handle, key, s_config.stocks[i]);
            snprintf(key, sizeof(key), "name%u", static_cast<unsigned>(i));
            nvs_set_str(handle, key, s_config.stock_names[i]);
        }
    } else {
        for (size_t i = 0; i < APP_MARKET_ITEM_COUNT; ++i) {
            char key[12];
            snprintf(key, sizeof(key), "stock%u", static_cast<unsigned>(i));
            ReadNvsString(handle, key, s_config.stocks[i],
                          sizeof(s_config.stocks[i]));
            snprintf(key, sizeof(key), "name%u", static_cast<unsigned>(i));
            ReadNvsString(handle, key, s_config.stock_names[i],
                          sizeof(s_config.stock_names[i]));
        }
    }
    uint8_t enabled = s_config.sleep_schedule_enabled ? 1 : 0;
    if (nvs_get_u8(handle, "sleep_on", &enabled) != ESP_OK) {
        nvs_set_u8(handle, "sleep_on", enabled);
    }
    s_config.sleep_schedule_enabled = enabled != 0;
    if (nvs_get_u8(handle, "sleep_h", &s_config.sleep_hour) != ESP_OK) {
        nvs_set_u8(handle, "sleep_h", s_config.sleep_hour);
    }
    if (nvs_get_u8(handle, "sleep_m", &s_config.sleep_minute) != ESP_OK) {
        nvs_set_u8(handle, "sleep_m", s_config.sleep_minute);
    }
    if (nvs_get_u8(handle, "wake_h", &s_config.wake_hour) != ESP_OK) {
        nvs_set_u8(handle, "wake_h", s_config.wake_hour);
    }
    if (nvs_get_u8(handle, "wake_m", &s_config.wake_minute) != ESP_OK) {
        nvs_set_u8(handle, "wake_m", s_config.wake_minute);
    }
    if (s_config.sleep_hour > 23 || s_config.sleep_minute > 59 ||
        s_config.wake_hour > 23 || s_config.wake_minute > 59 ||
        (s_config.sleep_hour == s_config.wake_hour &&
         s_config.sleep_minute == s_config.wake_minute)) {
        s_config.sleep_hour = 22;
        s_config.sleep_minute = 0;
        s_config.wake_hour = 8;
        s_config.wake_minute = 30;
        nvs_set_u8(handle, "sleep_h", s_config.sleep_hour);
        nvs_set_u8(handle, "sleep_m", s_config.sleep_minute);
        nvs_set_u8(handle, "wake_h", s_config.wake_hour);
        nvs_set_u8(handle, "wake_m", s_config.wake_minute);
    }
    nvs_set_u32(handle, "schema", 5);
    nvs_commit(handle);
    nvs_close(handle);
    s_snapshot.sleep_schedule_enabled = s_config.sleep_schedule_enabled;
    s_snapshot.sleep_hour = s_config.sleep_hour;
    s_snapshot.sleep_minute = s_config.sleep_minute;
    s_snapshot.wake_hour = s_config.wake_hour;
    s_snapshot.wake_minute = s_config.wake_minute;
}

esp_err_t SaveConfig()
{
    AppConfig config;
    Lock();
    config = s_config;
    Unlock();

    nvs_handle_t handle;
    esp_err_t status = nvs_open(kConfigNamespace, NVS_READWRITE, &handle);
    if (status != ESP_OK) {
        return status;
    }
    for (size_t i = 0; i < APP_MARKET_ITEM_COUNT && status == ESP_OK; ++i) {
        char key[12];
        snprintf(key, sizeof(key), "stock%u", static_cast<unsigned>(i));
        status = nvs_set_str(handle, key, config.stocks[i]);
        if (status == ESP_OK) {
            snprintf(key, sizeof(key), "name%u", static_cast<unsigned>(i));
            status = nvs_set_str(handle, key, config.stock_names[i]);
        }
    }
    if (status == ESP_OK) {
        status = nvs_set_u8(handle, "sleep_on",
                            config.sleep_schedule_enabled ? 1 : 0);
    }
    if (status == ESP_OK) {
        status = nvs_set_u8(handle, "sleep_h", config.sleep_hour);
    }
    if (status == ESP_OK) {
        status = nvs_set_u8(handle, "sleep_m", config.sleep_minute);
    }
    if (status == ESP_OK) {
        status = nvs_set_u8(handle, "wake_h", config.wake_hour);
    }
    if (status == ESP_OK) {
        status = nvs_set_u8(handle, "wake_m", config.wake_minute);
    }
    if (status == ESP_OK) {
        status = nvs_commit(handle);
    }
    nvs_close(handle);
    return status;
}

bool ReadSmallFile(const char *path, char *buffer, size_t capacity)
{
    FILE *file = fopen(path, "rb");
    if (file == nullptr) {
        return false;
    }
    const size_t length = fread(buffer, 1, capacity - 1, file);
    const bool at_end = feof(file) != 0;
    fclose(file);
    if (!at_end || length == 0) {
        return false;
    }
    buffer[length] = '\0';
    return true;
}

bool WriteLatestFile(const char *path, const char *data)
{
    char temporary_path[96];
    snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path);
    FILE *file = fopen(temporary_path, "wb");
    if (file == nullptr) {
        return false;
    }
    const size_t length = strlen(data);
    const bool written = fwrite(data, 1, length, file) == length;
    fflush(file);
    fclose(file);
    if (!written) {
        unlink(temporary_path);
        return false;
    }
    unlink(path);
    if (rename(temporary_path, path) != 0) {
        unlink(temporary_path);
        return false;
    }
    return true;
}

void LoadMarketCache()
{
    char *buffer = static_cast<char *>(malloc(8192));
    if (buffer == nullptr || !ReadSmallFile(kMarketCachePath, buffer, 8192)) {
        free(buffer);
        return;
    }
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (root == nullptr) {
        return;
    }
    cJSON *items = cJSON_GetObjectItem(root, "items");
    if (cJSON_IsArray(items)) {
        Lock();
        for (size_t i = 0; i < APP_MARKET_ITEM_COUNT; ++i) {
            cJSON *item = cJSON_GetArrayItem(items, static_cast<int>(i));
            if (!cJSON_IsObject(item)) {
                continue;
            }
            cJSON *symbol = cJSON_GetObjectItem(item, "symbol");
            cJSON *name = cJSON_GetObjectItem(item, "name");
            cJSON *price = cJSON_GetObjectItem(item, "price");
            cJSON *change = cJSON_GetObjectItem(item, "change");
            cJSON *change_amount =
                cJSON_GetObjectItem(item, "change_amount");
            if (cJSON_IsString(symbol) && cJSON_IsString(price) &&
                cJSON_IsString(change)) {
                strlcpy(s_snapshot.market[i].name,
                        cJSON_IsString(name) ? name->valuestring
                                             : s_config.stock_names[i],
                        sizeof(s_snapshot.market[i].name));
                strlcpy(s_snapshot.market[i].symbol, symbol->valuestring,
                        sizeof(s_snapshot.market[i].symbol));
                strlcpy(s_snapshot.market[i].price, price->valuestring,
                        sizeof(s_snapshot.market[i].price));
                strlcpy(s_snapshot.market[i].change, change->valuestring,
                        sizeof(s_snapshot.market[i].change));
                strlcpy(s_snapshot.market[i].change_amount,
                        cJSON_IsString(change_amount)
                            ? change_amount->valuestring
                            : "--",
                        sizeof(s_snapshot.market[i].change_amount));
                s_snapshot.market[i].valid = true;
            }
        }
        cJSON *updated = cJSON_GetObjectItem(root, "updated");
        if (cJSON_IsString(updated)) {
            strlcpy(s_snapshot.last_update, updated->valuestring,
                    sizeof(s_snapshot.last_update));
        }
        Unlock();
    }
    cJSON_Delete(root);
}

void SaveMarketCache()
{
    AppServiceSnapshot snapshot;
    AppServices_GetSnapshot(&snapshot);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "updated", snapshot.last_update);
    cJSON *items = cJSON_AddArrayToObject(root, "items");
    for (const AppMarketItem &market : snapshot.market) {
        if (!market.valid) {
            continue;
        }
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "name", market.name);
        cJSON_AddStringToObject(item, "symbol", market.symbol);
        cJSON_AddStringToObject(item, "price", market.price);
        cJSON_AddStringToObject(item, "change", market.change);
        cJSON_AddStringToObject(item, "change_amount", market.change_amount);
        cJSON_AddItemToArray(items, item);
    }
    char *json = cJSON_PrintUnformatted(root);
    if (json != nullptr) {
        WriteLatestFile(kMarketCachePath, json);
        cJSON_free(json);
    }
    cJSON_Delete(root);
}

void InitStorage()
{
    s_sd = new CustomSDPort("/sdcard");
    const bool ready = s_sd != nullptr && s_sd->SDPort_GetStatus() != 0;
    Lock();
    s_snapshot.sd_ready = ready;
    Unlock();
    if (!ready) {
        ESP_LOGW(kTag, "SD card unavailable; cache disabled");
        return;
    }

    mkdir("/sdcard/cache", 0775);
    uint64_t total_bytes = 0;
    uint64_t free_bytes = 0;
    uint32_t free_mb = 0;
    if (esp_vfs_fat_info("/sdcard", &total_bytes, &free_bytes) == ESP_OK) {
        free_mb = static_cast<uint32_t>(free_bytes / (1024ULL * 1024ULL));
    }
    Lock();
    s_snapshot.sd_free_mb = free_mb;
    Unlock();
    LoadMarketCache();
    ESP_LOGI(kTag, "SD cache ready, free=%lu MB",
             static_cast<unsigned long>(free_mb));
}

esp_err_t HttpEventHandler(esp_http_client_event_t *event)
{
    HttpBuffer *buffer = static_cast<HttpBuffer *>(event->user_data);
    if (event->event_id == HTTP_EVENT_ON_DATA && event->data_len > 0 &&
        buffer != nullptr) {
        if (buffer->length + event->data_len + 1 > buffer->capacity) {
            buffer->overflow = true;
            return ESP_FAIL;
        }
        memcpy(buffer->data + buffer->length, event->data, event->data_len);
        buffer->length += event->data_len;
        buffer->data[buffer->length] = '\0';
    }
    return ESP_OK;
}

bool HttpGet(const char *url, char *response, size_t capacity,
             const char *referer = nullptr)
{
    HttpBuffer buffer = {response, 0, capacity, false};
    response[0] = '\0';
    esp_http_client_config_t config = {};
    config.url = url;
    config.event_handler = HttpEventHandler;
    config.user_data = &buffer;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    config.timeout_ms = 18000;
    config.user_agent =
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
        "Chrome/124.0 Safari/537.36 Weixue-RLCD/1.0";
    config.keep_alive_enable = false;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == nullptr) {
        return false;
    }
    esp_http_client_set_header(client, "Accept", "application/json,text/plain,*/*");
    esp_http_client_set_header(client, "Accept-Encoding", "identity");
    esp_http_client_set_header(client, "Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8");
    esp_http_client_set_header(client, "Connection", "close");
    if (referer != nullptr) {
        esp_http_client_set_header(client, "Referer", referer);
    }
    const esp_err_t status = esp_http_client_perform(client);
    const int http_status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);
    if (status != ESP_OK || http_status != 200 || buffer.overflow ||
        buffer.length == 0) {
        ESP_LOGW(kTag, "GET failed: status=%s http=%d url=%s",
                 esp_err_to_name(status), http_status, url);
        return false;
    }
    return true;
}

bool IsSafeMarketSymbol(const char *symbol)
{
    if (symbol == nullptr || strlen(symbol) != 9) {
        return false;
    }
    for (size_t i = 0; i < 6; ++i) {
        if (!isdigit(static_cast<unsigned char>(symbol[i]))) {
            return false;
        }
    }
    return strcmp(symbol + 6, ".SS") == 0 ||
           strcmp(symbol + 6, ".SZ") == 0;
}

bool FetchIntraday(const char *provider_symbol, double previous_close,
                   AppMarketItem *output, char *response)
{
    if (previous_close <= 0.0) {
        return false;
    }
    char url[192];
    snprintf(url, sizeof(url),
             "https://web.ifzq.gtimg.cn/appstock/app/minute/query?code=%s",
             provider_symbol);
    if (!HttpGet(url, response, kHttpResponseCapacity,
                 "https://gu.qq.com/")) {
        snprintf(url, sizeof(url),
                 "http://web.ifzq.gtimg.cn/appstock/app/minute/query?code=%s",
                 provider_symbol);
        if (!HttpGet(url, response, kHttpResponseCapacity,
                     "http://gu.qq.com/")) {
            return false;
        }
    }

    cJSON *root = cJSON_Parse(response);
    cJSON *data = cJSON_GetObjectItemCaseSensitive(root, "data");
    cJSON *instrument = cJSON_GetObjectItemCaseSensitive(data, provider_symbol);
    cJSON *minute_data =
        cJSON_GetObjectItemCaseSensitive(instrument, "data");
    cJSON *points = cJSON_GetObjectItemCaseSensitive(minute_data, "data");
    if (!cJSON_IsArray(points)) {
        cJSON_Delete(root);
        return false;
    }

    uint8_t valid_points = 0;
    cJSON *point = nullptr;
    cJSON_ArrayForEach(point, points) {
        if (!cJSON_IsString(point)) {
            continue;
        }
        int hhmm = 0;
        double price = 0.0;
        if (sscanf(point->valuestring, "%4d %lf", &hhmm, &price) != 2 ||
            price <= 0.0) {
            continue;
        }
        const int hour = hhmm / 100;
        const int minute = hhmm % 100;
        int trading_minute = -1;
        if ((hour == 9 && minute >= 30) || hour == 10 ||
            (hour == 11 && minute <= 30)) {
            trading_minute = (hour - 9) * 60 + minute - 30;
        } else if ((hour == 13) || hour == 14 ||
                   (hour == 15 && minute == 0)) {
            trading_minute = 121 + (hour - 13) * 60 + minute;
        }
        if (trading_minute < 0 || trading_minute > 241) {
            continue;
        }
        const size_t bucket = std::min(
            APP_MARKET_CHART_POINT_COUNT - 1,
            static_cast<size_t>(trading_minute) *
                APP_MARKET_CHART_POINT_COUNT / 242U);
        const long basis_points = lround((price / previous_close - 1.0) *
                                         10000.0);
        if (output->intraday[bucket] == APP_MARKET_POINT_NONE) {
            ++valid_points;
        }
        output->intraday[bucket] = static_cast<int16_t>(
            std::clamp(basis_points, -30000L, 30000L));
    }
    cJSON_Delete(root);
    output->intraday_points = valid_points;
    return valid_points > 0;
}

bool FetchMarketItem(const char *symbol, const char *display_name,
                     AppMarketItem *output, char *response)
{
    if (!IsSafeMarketSymbol(symbol)) {
        return false;
    }
    memset(output, 0, sizeof(*output));
    std::fill(std::begin(output->intraday), std::end(output->intraday),
              APP_MARKET_POINT_NONE);
    strlcpy(output->name, display_name, sizeof(output->name));
    strlcpy(output->symbol, symbol, sizeof(output->symbol));

    char provider_symbol[32] = {};
    const size_t length = strlen(symbol);
    if (length > 3 && strcmp(symbol + length - 3, ".SS") == 0) {
        snprintf(provider_symbol, sizeof(provider_symbol), "sh%.*s",
                 static_cast<int>(length - 3), symbol);
    } else if (length > 3 && strcmp(symbol + length - 3, ".SZ") == 0) {
        snprintf(provider_symbol, sizeof(provider_symbol), "sz%.*s",
                 static_cast<int>(length - 3), symbol);
    } else {
        return false;
    }
    char url[160];
    snprintf(url, sizeof(url), "https://qt.gtimg.cn/q=%s", provider_symbol);
    if (!HttpGet(url, response, kHttpResponseCapacity)) {
        snprintf(url, sizeof(url), "http://qt.gtimg.cn/q=%s", provider_symbol);
        if (!HttpGet(url, response, kHttpResponseCapacity)) {
            return false;
        }
    }

    char *record = strchr(response, '"');
    if (record == nullptr) {
        return false;
    }
    ++record;
    char *save = nullptr;
    char *field = strtok_r(record, "~", &save);
    int index = 0;
    const char *price_text = nullptr;
    const char *previous_close_text = nullptr;
    while (field != nullptr) {
        if (index == 3) {
            price_text = field;
        } else if (index == 4) {
            previous_close_text = field;
        }
        if (price_text != nullptr && previous_close_text != nullptr) {
            break;
        }
        field = strtok_r(nullptr, "~", &save);
        ++index;
    }
    if (price_text == nullptr || previous_close_text == nullptr ||
        price_text[0] == '\0' || previous_close_text[0] == '\0') {
        return false;
    }
    const double value = strtod(price_text, nullptr);
    const double previous_close = strtod(previous_close_text, nullptr);
    if (value <= 0.0 || previous_close <= 0.0) {
        return false;
    }
    const double change_amount = value - previous_close;
    const double change_percent = change_amount / previous_close * 100.0;
    snprintf(output->price, sizeof(output->price),
             value >= 10000.0 ? "%.0f" : "%.2f", value);
    snprintf(output->change, sizeof(output->change), "%+.2f%%",
             change_percent);
    snprintf(output->change_amount, sizeof(output->change_amount), "%+.2f",
             change_amount);
    if (!FetchIntraday(provider_symbol, previous_close, output, response)) {
        ESP_LOGW(kTag, "Intraday unavailable for %s", provider_symbol);
    }
    output->valid = true;
    return true;
}

bool FetchMarketData(char *response)
{
    AppConfig config;
    Lock();
    config = s_config;
    Unlock();

    AppMarketItem fetched[APP_MARKET_ITEM_COUNT] = {};
    bool any_success = false;
    for (size_t i = 0; i < APP_MARKET_ITEM_COUNT; ++i) {
        if (FetchMarketItem(config.stocks[i], config.stock_names[i],
                            &fetched[i], response)) {
            any_success = true;
        }
    }
    if (any_success) {
        Lock();
        for (size_t i = 0; i < APP_MARKET_ITEM_COUNT; ++i) {
            if (fetched[i].valid) {
                s_snapshot.market[i] = fetched[i];
            }
        }
        ++s_snapshot.market_generation;
        Unlock();
    }
    return any_success;
}

void SetLastUpdateTime()
{
    const time_t now = time(nullptr);
    struct tm local = {};
    localtime_r(&now, &local);
    char text[24];
    strftime(text, sizeof(text), "%m-%d %H:%M", &local);
    Lock();
    strlcpy(s_snapshot.last_update, text, sizeof(s_snapshot.last_update));
    Unlock();
}

bool RestoreSystemTimeFromRtc()
{
    setenv("TZ", "CST-8", 1);
    tzset();
    rtcTimeStruct_t rtc_time = {};
    Rtc_GetTime(&rtc_time);
    if (rtc_time.year < 2024 || rtc_time.year > 2099 ||
        rtc_time.month < 1 || rtc_time.month > 12 || rtc_time.day < 1 ||
        rtc_time.day > 31 || rtc_time.hour < 0 || rtc_time.hour > 23 ||
        rtc_time.minute < 0 || rtc_time.minute > 59 || rtc_time.second < 0 ||
        rtc_time.second > 59) {
        ESP_LOGW(kTag, "RTC time is not valid yet");
        return false;
    }

    struct tm local = {};
    local.tm_year = rtc_time.year - 1900;
    local.tm_mon = rtc_time.month - 1;
    local.tm_mday = rtc_time.day;
    local.tm_hour = rtc_time.hour;
    local.tm_min = rtc_time.minute;
    local.tm_sec = rtc_time.second;
    local.tm_isdst = -1;
    const time_t restored = mktime(&local);
    if (restored <= 0) {
        return false;
    }
    struct timeval value = {.tv_sec = restored, .tv_usec = 0};
    if (settimeofday(&value, nullptr) != 0) {
        ESP_LOGW(kTag, "Failed to restore system time from RTC");
        return false;
    }
    Lock();
    s_snapshot.time_synced = true;
    Unlock();
    ESP_LOGI(kTag, "System time restored from RTC: %04d-%02d-%02d %02d:%02d:%02d",
             rtc_time.year, rtc_time.month, rtc_time.day, rtc_time.hour,
             rtc_time.minute, rtc_time.second);
    return true;
}

bool IsWithinSleepWindow(const AppConfig &config, const struct tm &local)
{
    if (!config.sleep_schedule_enabled) {
        return false;
    }
    const int now_minutes = local.tm_hour * 60 + local.tm_min;
    const int sleep_minutes = config.sleep_hour * 60 + config.sleep_minute;
    const int wake_minutes = config.wake_hour * 60 + config.wake_minute;
    if (sleep_minutes < wake_minutes) {
        return now_minutes >= sleep_minutes && now_minutes < wake_minutes;
    }
    return now_minutes >= sleep_minutes || now_minutes < wake_minutes;
}

uint64_t SecondsUntilWake(const AppConfig &config, time_t now,
                          const struct tm &local)
{
    struct tm wake = local;
    wake.tm_hour = config.wake_hour;
    wake.tm_min = config.wake_minute;
    wake.tm_sec = 0;
    wake.tm_isdst = -1;
    time_t wake_time = mktime(&wake);
    if (wake_time <= now) {
        wake.tm_mday += 1;
        wake_time = mktime(&wake);
    }
    const double seconds = difftime(wake_time, now);
    return seconds > 1 ? static_cast<uint64_t>(seconds) : 60;
}

bool ManualWakeOverrideActive()
{
    return s_schedule_override_until != 0 &&
           static_cast<int32_t>(xTaskGetTickCount() -
                                s_schedule_override_until) < 0;
}

bool ScheduledSleepDue(uint64_t *seconds_until_wake)
{
    if (ManualWakeOverrideActive()) {
        return false;
    }
    AppConfig config;
    Lock();
    config = s_config;
    Unlock();
    const time_t now = time(nullptr);
    if (now < 1704067200) {
        return false;
    }
    struct tm local = {};
    localtime_r(&now, &local);
    if (!IsWithinSleepWindow(config, local)) {
        return false;
    }
    *seconds_until_wake = SecondsUntilWake(config, now, local);
    return true;
}

[[noreturn]] void EnterScheduledSleep(uint64_t seconds_until_wake)
{
    Lock();
    s_snapshot.sleep_imminent = true;
    Unlock();
    StopWebServer();
    ESP_LOGI(kTag,
             "Scheduled deep sleep starts now; timer wake in %llu seconds, "
             "KEY can wake the device manually",
             static_cast<unsigned long long>(seconds_until_wake));

    // Give LVGL enough time to render the sleep status before suspending CPUs.
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(seconds_until_wake * 1000000ULL));
    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(GPIO_NUM_18, 0));
    esp_deep_sleep_start();
    abort();
}

bool SyncNetworkTime()
{
    setenv("TZ", "CST-8", 1);
    tzset();
    if (!s_sntp_initialized) {
        esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
        config.wait_for_sync = true;
        if (esp_netif_sntp_init(&config) != ESP_OK) {
            return false;
        }
        s_sntp_initialized = true;
    }
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(15000)) != ESP_OK) {
        ESP_LOGW(kTag, "NTP synchronization timed out");
        return false;
    }

    const time_t now = time(nullptr);
    struct tm local = {};
    localtime_r(&now, &local);
    Rtc_SetTime(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                local.tm_hour, local.tm_min, local.tm_sec);
    Lock();
    s_snapshot.time_synced = true;
    Unlock();
    s_ntp_synced_this_boot = true;
    ESP_LOGI(kTag, "NTP synchronized: %04d-%02d-%02d %02d:%02d:%02d",
             local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
             local.tm_hour, local.tm_min, local.tm_sec);
    return true;
}

TickType_t MarketRefreshPeriod()
{
    const time_t now = time(nullptr);
    if (now < 1704067200) {
        return kMarketTradingRefresh;
    }
    struct tm local = {};
    localtime_r(&now, &local);
    const int minutes = local.tm_hour * 60 + local.tm_min;
    const bool weekday = local.tm_wday >= 1 && local.tm_wday <= 5;
    const bool trading_session =
        (minutes >= 9 * 60 + 15 && minutes <= 11 * 60 + 35) ||
        (minutes >= 12 * 60 + 55 && minutes <= 15 * 60 + 5);
    return weekday && trading_session ? kMarketTradingRefresh
                                      : kMarketIdleRefresh;
}

void PerformRefresh()
{
    char *response = static_cast<char *>(heap_caps_malloc(
        kHttpResponseCapacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (response == nullptr) {
        response = static_cast<char *>(malloc(kHttpResponseCapacity));
    }
    if (response == nullptr) {
        return;
    }
    Lock();
    s_snapshot.refresh_in_progress = true;
    Unlock();
    const bool market_ok = FetchMarketData(response);
    if (market_ok) {
        SetLastUpdateTime();
        bool sd_ready;
        Lock();
        sd_ready = s_snapshot.sd_ready;
        Unlock();
        if (sd_ready && market_ok) {
            SaveMarketCache();
        }
    }
    Lock();
    s_snapshot.data_online = market_ok;
    s_snapshot.refresh_in_progress = false;
    Unlock();
    free(response);
    ESP_LOGI(kTag, "Market refresh finished: %s",
             market_ok ? "ok" : "failed");
}

void MarkWebActivity()
{
    s_web_last_activity = xTaskGetTickCount();
}

esp_err_t SendJson(httpd_req_t *request, cJSON *root)
{
    char *json = cJSON_PrintUnformatted(root);
    if (json == nullptr) {
        return httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR,
                                   "json allocation failed");
    }
    httpd_resp_set_type(request, "application/json; charset=utf-8");
    const esp_err_t status = httpd_resp_sendstr(request, json);
    cJSON_free(json);
    return status;
}

esp_err_t IndexHandler(httpd_req_t *request)
{
    MarkWebActivity();
    static const char kHtml[] = R"HTML(<!doctype html>
<html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>微雪墨水屏配置</title><style>
body{font-family:system-ui,sans-serif;max-width:760px;margin:20px auto;padding:0 16px;background:#f3f5f7;color:#18212b}h1{font-size:24px}section{background:#fff;border-radius:12px;padding:16px;margin:12px 0;box-shadow:0 1px 5px #0002}label{display:block;margin:10px 0 4px}input{box-sizing:border-box;width:100%;padding:10px;border:1px solid #aeb8c2;border-radius:7px}button{padding:10px 16px;margin:10px 8px 0 0;border:0;border-radius:7px;background:#1769aa;color:#fff}.danger{background:#a33}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:8px}.muted{color:#647383;font-size:14px}pre{white-space:pre-wrap}</style></head>
<body><h1>ESP32-S3 RLCD 配置</h1><section><h2>设备状态</h2><div id="status">读取中…</div></section>
<section><h2>网络、A股与休眠</h2><form id="config"><label>Wi-Fi 名称</label><input id="ssid" maxlength="32"><label>新密码（留空表示不修改）</label><input id="password" type="password" maxlength="63"><div class="grid"><div><label>股票1名称</label><input id="name0" maxlength="20"><label>股票1代码</label><input id="stock0" maxlength="9"></div><div><label>股票2名称</label><input id="name1" maxlength="20"><label>股票2代码</label><input id="stock1" maxlength="9"></div><div><label>股票3名称</label><input id="name2" maxlength="20"><label>股票3代码</label><input id="stock2" maxlength="9"></div><div><label>股票4名称</label><input id="name3" maxlength="20"><label>股票4代码</label><input id="stock3" maxlength="9"></div></div><label><input id="sleepEnabled" type="checkbox" style="width:auto"> 启用每日深度休眠</label><div class="grid"><div><label>休眠时间</label><input id="sleepTime" type="time"></div><div><label>唤醒时间</label><input id="wakeTime" type="time"></div></div><p class="muted">A 股在交易时段每 1 分钟刷新，非交易时段降低频率。蓝牙已关闭。</p><p class="muted">默认每天 22:00 休眠、次日 08:30 唤醒；按 KEY 可临时唤醒 30 分钟。仅接受六位 A 股代码：沪市使用 .SS，深市使用 .SZ。</p><button type="submit">保存并应用</button></form><button id="refreshNow">立即刷新</button><button class="danger" id="stopWeb">关闭配置网页</button><pre id="message"></pre></section>
<script>
const $=id=>document.getElementById(id);async function getJson(url,opt){const r=await fetch(url,opt);if(!r.ok)throw new Error(await r.text());return r.json()}
async function load(){try{const [s,c]=await Promise.all([getJson('/api/status'),getJson('/api/config')]);$('status').innerHTML=`无线网络：${s.wifi_connected?'已连接':'未连接'} ${s.ip||''}<br>蓝牙：已关闭<br>存储卡：${s.sd_ready?'正常，可用 '+s.sd_free_mb+' MB':'不可用'}<br>时间同步：${s.time_synced?'完成':'等待中'}<br>休眠计划：${c.sleep_schedule_enabled?c.sleep_time+' → '+c.wake_time:'关闭'}<br>数据更新：${s.last_update||'暂无'}<br>网页将在空闲 10 分钟后自动关闭`;$('ssid').value=c.ssid;for(let i=0;i<4;i++){$('stock'+i).value=c.stocks[i];$('name'+i).value=c.stock_names[i]}$('sleepEnabled').checked=c.sleep_schedule_enabled;$('sleepTime').value=c.sleep_time;$('wakeTime').value=c.wake_time}catch(e){$('message').textContent=e}}
$('config').onsubmit=async e=>{e.preventDefault();const st=$('sleepTime').value.split(':').map(Number),wt=$('wakeTime').value.split(':').map(Number);const body={ssid:$('ssid').value,password:$('password').value,stocks:[$('stock0').value,$('stock1').value,$('stock2').value,$('stock3').value],stock_names:[$('name0').value,$('name1').value,$('name2').value,$('name3').value],sleep_schedule_enabled:$('sleepEnabled').checked,sleep_hour:st[0],sleep_minute:st[1],wake_hour:wt[0],wake_minute:wt[1]};try{await getJson('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});$('password').value='';$('message').textContent='保存成功，已请求刷新。修改 Wi-Fi 后本页面可能暂时断开。';setTimeout(load,1500)}catch(e){$('message').textContent=e}};
$('refreshNow').onclick=async()=>{await getJson('/api/refresh',{method:'POST'});$('message').textContent='已请求立即刷新。'};$('stopWeb').onclick=async()=>{await getJson('/api/stop',{method:'POST'});$('message').textContent='网页服务正在关闭，可通过设备设置页长按 KEY 再次开启。'};load();setInterval(load,10000);
</script></body></html>)HTML";
    httpd_resp_set_type(request, "text/html; charset=utf-8");
    return httpd_resp_send(request, kHtml, HTTPD_RESP_USE_STRLEN);
}

esp_err_t StatusHandler(httpd_req_t *request)
{
    MarkWebActivity();
    AppServiceSnapshot snapshot;
    AppServices_GetSnapshot(&snapshot);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "wifi_connected", espwifi_is_connected());
    cJSON_AddStringToObject(root, "ip", user_esp_bsp._ip);
    cJSON_AddBoolToObject(root, "sd_ready", snapshot.sd_ready);
    cJSON_AddNumberToObject(root, "sd_free_mb", snapshot.sd_free_mb);
    cJSON_AddBoolToObject(root, "time_synced", snapshot.time_synced);
    cJSON_AddStringToObject(root, "last_update", snapshot.last_update);
    cJSON_AddBoolToObject(root, "refreshing", snapshot.refresh_in_progress);
    const esp_err_t status = SendJson(request, root);
    cJSON_Delete(root);
    return status;
}

esp_err_t ConfigGetHandler(httpd_req_t *request)
{
    MarkWebActivity();
    AppConfig config;
    Lock();
    config = s_config;
    Unlock();
    char ssid[33] = {};
    espwifi_get_ssid(ssid, sizeof(ssid));
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "ssid", ssid);
    cJSON *stocks = cJSON_AddArrayToObject(root, "stocks");
    for (const auto &stock : config.stocks) {
        cJSON_AddItemToArray(stocks, cJSON_CreateString(stock));
    }
    cJSON *stock_names = cJSON_AddArrayToObject(root, "stock_names");
    for (const auto &name : config.stock_names) {
        cJSON_AddItemToArray(stock_names, cJSON_CreateString(name));
    }
    cJSON_AddBoolToObject(root, "sleep_schedule_enabled",
                          config.sleep_schedule_enabled);
    char sleep_time[8];
    char wake_time[8];
    snprintf(sleep_time, sizeof(sleep_time), "%02u:%02u",
             static_cast<unsigned>(config.sleep_hour),
             static_cast<unsigned>(config.sleep_minute));
    snprintf(wake_time, sizeof(wake_time), "%02u:%02u",
             static_cast<unsigned>(config.wake_hour),
             static_cast<unsigned>(config.wake_minute));
    cJSON_AddStringToObject(root, "sleep_time", sleep_time);
    cJSON_AddStringToObject(root, "wake_time", wake_time);
    const esp_err_t status = SendJson(request, root);
    cJSON_Delete(root);
    return status;
}

bool ReceivePostBody(httpd_req_t *request, char *body, size_t capacity)
{
    if (request->content_len <= 0 ||
        static_cast<size_t>(request->content_len) >= capacity) {
        return false;
    }
    size_t received = 0;
    while (received < static_cast<size_t>(request->content_len)) {
        const int result = httpd_req_recv(
            request, body + received, request->content_len - received);
        if (result <= 0) {
            return false;
        }
        received += result;
    }
    body[received] = '\0';
    return true;
}

esp_err_t ConfigPostHandler(httpd_req_t *request)
{
    MarkWebActivity();
    char body[kPostBodyCapacity];
    if (!ReceivePostBody(request, body, sizeof(body))) {
        return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST,
                                   "invalid request body");
    }
    cJSON *root = cJSON_Parse(body);
    cJSON *ssid = cJSON_GetObjectItem(root, "ssid");
    cJSON *password = cJSON_GetObjectItem(root, "password");
    cJSON *stocks = cJSON_GetObjectItem(root, "stocks");
    cJSON *stock_names = cJSON_GetObjectItem(root, "stock_names");
    cJSON *sleep_enabled =
        cJSON_GetObjectItem(root, "sleep_schedule_enabled");
    cJSON *sleep_hour = cJSON_GetObjectItem(root, "sleep_hour");
    cJSON *sleep_minute = cJSON_GetObjectItem(root, "sleep_minute");
    cJSON *wake_hour = cJSON_GetObjectItem(root, "wake_hour");
    cJSON *wake_minute = cJSON_GetObjectItem(root, "wake_minute");
    bool valid = cJSON_IsObject(root) && cJSON_IsString(ssid) &&
                 cJSON_IsString(password) && cJSON_IsArray(stocks) &&
                 cJSON_GetArraySize(stocks) == APP_MARKET_ITEM_COUNT &&
                 cJSON_IsArray(stock_names) &&
                 cJSON_GetArraySize(stock_names) == APP_MARKET_ITEM_COUNT &&
                 cJSON_IsBool(sleep_enabled) &&
                 cJSON_IsNumber(sleep_hour) && sleep_hour->valueint >= 0 &&
                 sleep_hour->valueint <= 23 && cJSON_IsNumber(sleep_minute) &&
                 sleep_minute->valueint >= 0 && sleep_minute->valueint <= 59 &&
                 cJSON_IsNumber(wake_hour) && wake_hour->valueint >= 0 &&
                 wake_hour->valueint <= 23 && cJSON_IsNumber(wake_minute) &&
                 wake_minute->valueint >= 0 && wake_minute->valueint <= 59 &&
                 (sleep_hour->valueint != wake_hour->valueint ||
                  sleep_minute->valueint != wake_minute->valueint);
    AppConfig new_config = {};
    if (valid) {
        new_config.sleep_schedule_enabled = cJSON_IsTrue(sleep_enabled);
        new_config.sleep_hour = sleep_hour->valueint;
        new_config.sleep_minute = sleep_minute->valueint;
        new_config.wake_hour = wake_hour->valueint;
        new_config.wake_minute = wake_minute->valueint;
        for (size_t i = 0; i < APP_MARKET_ITEM_COUNT; ++i) {
            cJSON *symbol = cJSON_GetArrayItem(stocks, static_cast<int>(i));
            cJSON *name =
                cJSON_GetArrayItem(stock_names, static_cast<int>(i));
            if (!cJSON_IsString(symbol) ||
                !IsSafeMarketSymbol(symbol->valuestring) ||
                !cJSON_IsString(name) || name->valuestring[0] == '\0' ||
                strlen(name->valuestring) >= sizeof(new_config.stock_names[i])) {
                valid = false;
                break;
            }
            strlcpy(new_config.stocks[i], symbol->valuestring,
                    sizeof(new_config.stocks[i]));
            strlcpy(new_config.stock_names[i], name->valuestring,
                    sizeof(new_config.stock_names[i]));
        }
    }
    if (!valid || strlen(ssid->valuestring) == 0 ||
        strlen(ssid->valuestring) > 32) {
        cJSON_Delete(root);
        return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST,
                                   "invalid configuration");
    }

    Lock();
    s_config = new_config;
    s_snapshot.sleep_schedule_enabled =
        new_config.sleep_schedule_enabled;
    s_snapshot.sleep_hour = new_config.sleep_hour;
    s_snapshot.sleep_minute = new_config.sleep_minute;
    s_snapshot.wake_hour = new_config.wake_hour;
    s_snapshot.wake_minute = new_config.wake_minute;
    Unlock();
    const esp_err_t save_status = SaveConfig();

    char current_ssid[33] = {};
    espwifi_get_ssid(current_ssid, sizeof(current_ssid));
    const bool wifi_change = strcmp(current_ssid, ssid->valuestring) != 0 ||
                             password->valuestring[0] != '\0';
    esp_err_t wifi_status = ESP_OK;
    if (wifi_change) {
        if (password->valuestring[0] == '\0') {
            wifi_status = ESP_ERR_INVALID_ARG;
        } else {
            wifi_status = espwifi_update_credentials(ssid->valuestring,
                                                     password->valuestring);
        }
    }
    cJSON_Delete(root);
    if (save_status != ESP_OK || wifi_status != ESP_OK) {
        return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST,
                                   "configuration save failed");
    }
    AppServices_RequestRefresh();
    cJSON *reply = cJSON_CreateObject();
    cJSON_AddBoolToObject(reply, "ok", true);
    const esp_err_t status = SendJson(request, reply);
    cJSON_Delete(reply);
    return status;
}

esp_err_t RefreshHandler(httpd_req_t *request)
{
    MarkWebActivity();
    AppServices_RequestRefresh();
    cJSON *reply = cJSON_CreateObject();
    cJSON_AddBoolToObject(reply, "ok", true);
    const esp_err_t status = SendJson(request, reply);
    cJSON_Delete(reply);
    return status;
}

esp_err_t StopHandler(httpd_req_t *request)
{
    MarkWebActivity();
    cJSON *reply = cJSON_CreateObject();
    cJSON_AddBoolToObject(reply, "ok", true);
    const esp_err_t status = SendJson(request, reply);
    cJSON_Delete(reply);
    AppServices_SetWebEnabled(false);
    return status;
}

void StartWebServer()
{
    if (s_http_server != nullptr || !espwifi_is_connected()) {
        return;
    }
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;
    config.max_uri_handlers = 8;
    config.lru_purge_enable = true;
    if (httpd_start(&s_http_server, &config) != ESP_OK) {
        s_http_server = nullptr;
        return;
    }
    const httpd_uri_t handlers[] = {
        {.uri = "/", .method = HTTP_GET, .handler = IndexHandler,
         .user_ctx = nullptr},
        {.uri = "/api/status", .method = HTTP_GET, .handler = StatusHandler,
         .user_ctx = nullptr},
        {.uri = "/api/config", .method = HTTP_GET,
         .handler = ConfigGetHandler, .user_ctx = nullptr},
        {.uri = "/api/config", .method = HTTP_POST,
         .handler = ConfigPostHandler, .user_ctx = nullptr},
        {.uri = "/api/refresh", .method = HTTP_POST,
         .handler = RefreshHandler, .user_ctx = nullptr},
        {.uri = "/api/stop", .method = HTTP_POST, .handler = StopHandler,
         .user_ctx = nullptr},
    };
    for (const httpd_uri_t &handler : handlers) {
        httpd_register_uri_handler(s_http_server, &handler);
    }
    char url[64];
    snprintf(url, sizeof(url), "http://%s/", user_esp_bsp._ip);
    Lock();
    s_snapshot.web_running = true;
    strlcpy(s_snapshot.web_url, url, sizeof(s_snapshot.web_url));
    Unlock();
    MarkWebActivity();
    ESP_LOGI(kTag, "Web configuration started at %s", url);
}

void StopWebServer()
{
    if (s_http_server != nullptr) {
        httpd_stop(s_http_server);
        s_http_server = nullptr;
    }
    Lock();
    s_snapshot.web_running = false;
    s_snapshot.web_url[0] = '\0';
    Unlock();
    ESP_LOGI(kTag, "Web configuration stopped");
}

void ServiceTask(void *argument)
{
    (void)argument;
    bool refresh_due = true;
    TickType_t next_market_refresh = 0;
    TickType_t next_ntp_attempt = 0;

    for (;;) {
        const EventBits_t events = xEventGroupWaitBits(
            s_events,
            kRefreshRequested | kWebStartRequested | kWebStopRequested,
            pdTRUE, pdFALSE, pdMS_TO_TICKS(1000));
        if (events & kRefreshRequested) {
            refresh_due = true;
        }
        if (events & kWebStartRequested) {
            s_web_desired = true;
        }
        if (events & kWebStopRequested) {
            s_web_desired = false;
            StopWebServer();
        }

        const TickType_t now = xTaskGetTickCount();
        uint64_t seconds_until_wake = 0;
        if (ScheduledSleepDue(&seconds_until_wake)) {
            EnterScheduledSleep(seconds_until_wake);
        }
        if (espwifi_is_connected()) {
            if (!s_ntp_synced_this_boot &&
                static_cast<int32_t>(now - next_ntp_attempt) >= 0) {
                if (!SyncNetworkTime()) {
                    next_ntp_attempt = now + pdMS_TO_TICKS(60000);
                }
            }
            if (s_web_desired && s_http_server == nullptr) {
                StartWebServer();
            }

            if (refresh_due ||
                static_cast<int32_t>(now - next_market_refresh) >= 0) {
                refresh_due = false;
                PerformRefresh();
                const TickType_t refreshed_at = xTaskGetTickCount();
                next_market_refresh = refreshed_at + MarketRefreshPeriod();
            }
        }

        if (s_http_server != nullptr &&
            static_cast<int32_t>(now - s_web_last_activity) >=
                static_cast<int32_t>(kWebIdleTimeout)) {
            s_web_desired = false;
            StopWebServer();
        }
    }
}

}  // namespace

void AppServices_Init()
{
    s_mutex = xSemaphoreCreateMutex();
    s_events = xEventGroupCreate();
    memset(&s_snapshot, 0, sizeof(s_snapshot));
    LoadConfig();
    RestoreSystemTimeFromRtc();
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
        s_schedule_override_until =
            xTaskGetTickCount() + kManualWakeOverride;
        ESP_LOGI(kTag,
                 "Manual KEY wake detected; sleep schedule paused for 30 minutes");
    }
    InitStorage();
}

void AppServices_Start()
{
    xTaskCreatePinnedToCore(ServiceTask, "app_services", 10240, nullptr, 3,
                            nullptr, 1);
    xEventGroupSetBits(s_events, kWebStartRequested | kRefreshRequested);
}

void AppServices_GetSnapshot(AppServiceSnapshot *snapshot)
{
    if (snapshot == nullptr || s_mutex == nullptr) {
        return;
    }
    Lock();
    *snapshot = s_snapshot;
    Unlock();
}

void AppServices_RequestRefresh()
{
    if (s_events != nullptr) {
        xEventGroupSetBits(s_events, kRefreshRequested);
    }
}

void AppServices_SetWebEnabled(bool enabled)
{
    if (s_events != nullptr) {
        xEventGroupSetBits(s_events,
                           enabled ? kWebStartRequested : kWebStopRequested);
    }
}

void AppServices_ToggleWeb()
{
    AppServiceSnapshot snapshot;
    AppServices_GetSnapshot(&snapshot);
    AppServices_SetWebEnabled(!snapshot.web_running);
}
