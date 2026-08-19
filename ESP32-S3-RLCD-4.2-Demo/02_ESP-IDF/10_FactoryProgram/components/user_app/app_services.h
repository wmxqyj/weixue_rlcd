#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

constexpr size_t APP_MARKET_ITEM_COUNT = 4;
constexpr size_t APP_MARKET_CHART_POINT_COUNT = 60;
constexpr int16_t APP_MARKET_POINT_NONE = INT16_MIN;

struct AppMarketItem {
    char name[32];
    char symbol[20];
    char price[24];
    char change[16];
    char change_amount[16];
    int16_t intraday[APP_MARKET_CHART_POINT_COUNT];
    uint8_t intraday_points;
    bool valid;
};

struct AppServiceSnapshot {
    bool sd_ready;
    uint32_t sd_free_mb;
    bool time_synced;
    bool web_running;
    bool data_online;
    bool refresh_in_progress;
    bool sleep_schedule_enabled;
    bool sleep_imminent;
    uint32_t market_generation;
    uint8_t sleep_hour;
    uint8_t sleep_minute;
    uint8_t wake_hour;
    uint8_t wake_minute;
    char web_url[64];
    char last_update[24];
    AppMarketItem market[APP_MARKET_ITEM_COUNT];
};

void AppServices_Init();
void AppServices_Start();
void AppServices_GetSnapshot(AppServiceSnapshot *snapshot);
void AppServices_RequestRefresh();
void AppServices_SetWebEnabled(bool enabled);
void AppServices_ToggleWeb();
