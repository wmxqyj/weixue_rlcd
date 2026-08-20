#include <stdio.h>
#include <string.h>
#include <time.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>

#include <esp_log.h>

#include "adc_bsp.h"
#include "app_services.h"
#include "button_bsp.h"
#include "esp_wifi_bsp.h"
#include "gui_guider.h"
#include "i2c_bsp.h"
#include "i2c_equipment.h"
#include "lvgl_bsp.h"
#include "user_app.h"

LV_FONT_DECLARE(lv_font_noto_sans_sc_18)
LV_FONT_DECLARE(lv_font_noto_sans_sc_13)

namespace {

constexpr int kDisplayWidth = 400;
constexpr int kDisplayHeight = 300;
constexpr int kStatusHeight = 38;
constexpr int kFooterHeight = 20;
constexpr int kContentTop = kStatusHeight;
constexpr int kContentHeight = kDisplayHeight - kStatusHeight - kFooterHeight;

constexpr EventBits_t kButtonSingle = (1U << 0);
constexpr EventBits_t kButtonDouble = (1U << 1);
constexpr EventBits_t kButtonLong = (1U << 2);
constexpr EventBits_t kAllButtonEvents =
    kButtonSingle | kButtonDouble | kButtonLong;

constexpr TickType_t kUiPollPeriod = pdMS_TO_TICKS(100);
constexpr TickType_t kRtcUpdatePeriod = pdMS_TO_TICKS(15000);
constexpr TickType_t kSensorUpdatePeriod = pdMS_TO_TICKS(30000);
constexpr TickType_t kBatteryUpdatePeriod = pdMS_TO_TICKS(60000);
constexpr TickType_t kConnectivityUpdatePeriod = pdMS_TO_TICKS(1000);
constexpr TickType_t kFooterMessagePeriod = pdMS_TO_TICKS(3000);

const char *kTag = "weixue_ui";

enum class PageId : uint8_t {
    Home = 0,
    Market1,
    Market2,
    Settings,
    Count,
};

struct UiObjects {
    lv_obj_t *screen = nullptr;
    lv_obj_t *pages[static_cast<size_t>(PageId::Count)] = {};

    lv_obj_t *status_bar = nullptr;
    lv_obj_t *status_time = nullptr;
    lv_obj_t *status_title = nullptr;
    lv_obj_t *status_wifi = nullptr;
    lv_obj_t *status_battery = nullptr;
    lv_obj_t *footer_box = nullptr;
    lv_obj_t *footer = nullptr;

    lv_obj_t *home_time = nullptr;
    lv_obj_t *home_date = nullptr;
    lv_obj_t *home_temperature = nullptr;
    lv_obj_t *home_humidity = nullptr;
    lv_obj_t *home_connection = nullptr;

    lv_obj_t *market_names[APP_MARKET_ITEM_COUNT] = {};
    lv_obj_t *market_codes[APP_MARKET_ITEM_COUNT] = {};
    lv_obj_t *market_prices[APP_MARKET_ITEM_COUNT] = {};
    lv_obj_t *market_changes[APP_MARKET_ITEM_COUNT] = {};
    lv_obj_t *market_charts[APP_MARKET_ITEM_COUNT] = {};
    lv_chart_series_t *market_series[APP_MARKET_ITEM_COUNT] = {};
    lv_obj_t *market_benchmarks[APP_MARKET_BENCHMARK_COUNT] = {};

    lv_obj_t *settings_wifi = nullptr;
    lv_obj_t *settings_sd = nullptr;
    lv_obj_t *settings_web = nullptr;
    lv_obj_t *settings_refresh = nullptr;
};

UiObjects s_ui;
PageId s_current_page = PageId::Home;
I2cMasterBus s_i2c_bus(14, 13, 0);
Shtc3Port *s_shtc3 = nullptr;

TickType_t s_footer_restore_at = 0;
bool s_footer_is_message = false;
uint32_t s_market_rendered_generation = UINT32_MAX;

const char *PageTitle(PageId page)
{
    static const char *const kTitles[] = {
        "首页", "行情一", "行情二", "设置"
    };
    return kTitles[static_cast<size_t>(page)];
}

void SetObjectBox(lv_obj_t *object, int radius = 0)
{
    lv_obj_set_style_bg_color(object, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(object, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(object, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(object, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(object, radius, LV_PART_MAIN);
    lv_obj_set_style_pad_all(object, 0, LV_PART_MAIN);
    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t *CreateLabel(lv_obj_t *parent, const char *text,
                      const lv_font_t *font = &lv_font_noto_sans_sc_18)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    return label;
}

lv_obj_t *CreatePage()
{
    lv_obj_t *page = lv_obj_create(s_ui.screen);
    lv_obj_set_pos(page, 0, kContentTop);
    lv_obj_set_size(page, kDisplayWidth, kContentHeight);
    lv_obj_set_style_bg_color(page, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(page, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(page, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(page, 0, LV_PART_MAIN);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    return page;
}

void CreateStatusBar()
{
    lv_obj_t *bar = lv_obj_create(s_ui.screen);
    s_ui.status_bar = bar;
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_size(bar, kDisplayWidth, kStatusHeight);
    lv_obj_set_style_bg_color(bar, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(bar, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(bar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(bar, 0, LV_PART_MAIN);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    s_ui.status_time = CreateLabel(bar, "-- -- 周-");
    lv_obj_set_pos(s_ui.status_time, 4, 8);
    lv_obj_set_size(s_ui.status_time, 116, 22);
    lv_obj_set_style_text_align(s_ui.status_time, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);

    s_ui.status_title = CreateLabel(bar, "首页");
    lv_obj_set_pos(s_ui.status_title, 120, 7);
    lv_obj_set_size(s_ui.status_title, 104, 24);

    s_ui.status_wifi = CreateLabel(bar, "无线 --");
    lv_obj_set_pos(s_ui.status_wifi, 226, 8);
    lv_obj_set_size(s_ui.status_wifi, 76, 22);

    s_ui.status_battery = CreateLabel(bar, "电量 --%");
    lv_obj_set_pos(s_ui.status_battery, 302, 8);
    lv_obj_set_size(s_ui.status_battery, 94, 22);
    lv_obj_set_style_text_align(s_ui.status_battery, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
}

void CreateFooter()
{
    lv_obj_t *footer_box = lv_obj_create(s_ui.screen);
    s_ui.footer_box = footer_box;
    lv_obj_set_pos(footer_box, 0, kDisplayHeight - kFooterHeight);
    lv_obj_set_size(footer_box, kDisplayWidth, kFooterHeight);
    lv_obj_set_style_bg_color(footer_box, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(footer_box, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(footer_box, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(footer_box, 1, LV_PART_MAIN);
    lv_obj_set_style_border_side(footer_box, LV_BORDER_SIDE_TOP, LV_PART_MAIN);
    lv_obj_set_style_radius(footer_box, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(footer_box, 0, LV_PART_MAIN);
    lv_obj_clear_flag(footer_box, LV_OBJ_FLAG_SCROLLABLE);

    s_ui.footer = CreateLabel(footer_box,
                              "KEY 上一页 | BOOT 下一页 | 长按操作",
                              &lv_font_noto_sans_sc_13);
    lv_obj_set_pos(s_ui.footer, 4, 1);
    lv_obj_set_size(s_ui.footer, 392, 17);
}

void CreateHomePage()
{
    lv_obj_t *page = CreatePage();
    s_ui.pages[static_cast<size_t>(PageId::Home)] = page;

    s_ui.home_time = CreateLabel(page, "--:--", &lv_font_MISANSMEDIUM_100);
    lv_obj_set_pos(s_ui.home_time, 6, -8);
    lv_obj_set_size(s_ui.home_time, 388, 108);

    s_ui.home_date = CreateLabel(page, "时间未同步");
    lv_obj_set_pos(s_ui.home_date, 10, 96);
    lv_obj_set_size(s_ui.home_date, 380, 24);

    lv_obj_t *temperature_box = lv_obj_create(page);
    lv_obj_set_pos(temperature_box, 10, 128);
    lv_obj_set_size(temperature_box, 185, 72);
    SetObjectBox(temperature_box, 5);
    lv_obj_t *temperature_title = CreateLabel(temperature_box, "温度");
    lv_obj_set_pos(temperature_title, 10, 23);
    lv_obj_set_size(temperature_title, 55, 20);
    lv_obj_set_style_text_align(temperature_title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    s_ui.home_temperature = CreateLabel(temperature_box, "-- C", &lv_font_MISANSMEDIUM_25);
    lv_obj_set_pos(s_ui.home_temperature, 65, 19);
    lv_obj_set_size(s_ui.home_temperature, 108, 36);
    lv_obj_set_style_text_align(s_ui.home_temperature, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);

    lv_obj_t *humidity_box = lv_obj_create(page);
    lv_obj_set_pos(humidity_box, 205, 128);
    lv_obj_set_size(humidity_box, 185, 72);
    SetObjectBox(humidity_box, 5);
    lv_obj_t *humidity_title = CreateLabel(humidity_box, "湿度");
    lv_obj_set_pos(humidity_title, 10, 23);
    lv_obj_set_size(humidity_title, 55, 20);
    lv_obj_set_style_text_align(humidity_title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    s_ui.home_humidity = CreateLabel(humidity_box, "-- %", &lv_font_MISANSMEDIUM_25);
    lv_obj_set_pos(s_ui.home_humidity, 65, 19);
    lv_obj_set_size(s_ui.home_humidity, 108, 36);
    lv_obj_set_style_text_align(s_ui.home_humidity, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);

    s_ui.home_connection = CreateLabel(page, "数据等待更新  |  22:00 休眠");
    lv_obj_set_pos(s_ui.home_connection, 8, 209);
    lv_obj_set_size(s_ui.home_connection, 384, 20);
}

void CreateMarketPage(PageId page_id, size_t page_index,
                      bool show_benchmarks)
{
    lv_obj_t *page = lv_obj_create(s_ui.screen);
    lv_obj_set_pos(page, 0, 0);
    lv_obj_set_size(page, kDisplayWidth, kDisplayHeight);
    lv_obj_set_style_bg_color(page, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(page, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(page, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(page, 0, LV_PART_MAIN);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    s_ui.pages[static_cast<size_t>(page_id)] = page;

    const int list_top = show_benchmarks ? 30 : 0;
    if (show_benchmarks) {
        lv_obj_t *benchmark_bar = lv_obj_create(page);
        lv_obj_set_pos(benchmark_bar, 0, 0);
        lv_obj_set_size(benchmark_bar, kDisplayWidth, list_top);
        SetObjectBox(benchmark_bar, 0);
        lv_obj_set_style_border_side(benchmark_bar, LV_BORDER_SIDE_BOTTOM,
                                     LV_PART_MAIN);
        for (size_t i = 0; i < APP_MARKET_BENCHMARK_COUNT; ++i) {
            s_ui.market_benchmarks[i] =
                CreateLabel(benchmark_bar,
                            i == 0 ? "上证  --" : "科创  --",
                            &lv_font_noto_sans_sc_18);
            lv_obj_set_pos(s_ui.market_benchmarks[i],
                           static_cast<int>(i) * 190 + 8, 1);
            lv_obj_set_size(s_ui.market_benchmarks[i], 180, 20);
            lv_obj_set_style_text_align(s_ui.market_benchmarks[i],
                                        LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        }
    }

    const int row_height = (kDisplayHeight - list_top) /
                           static_cast<int>(APP_MARKET_ITEMS_PER_PAGE);
    const size_t offset = page_index * APP_MARKET_ITEMS_PER_PAGE;
    for (size_t i = 0; i < APP_MARKET_ITEMS_PER_PAGE; ++i) {
        const size_t item_index = offset + i;
        const int row_y = list_top + static_cast<int>(i) * row_height;
        const int height = i + 1 == APP_MARKET_ITEMS_PER_PAGE
                               ? kDisplayHeight - row_y
                               : row_height;
        lv_obj_t *row = lv_obj_create(page);
        lv_obj_set_pos(row, 0, row_y);
        lv_obj_set_size(row, kDisplayWidth, height);
        SetObjectBox(row, 0);
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);

        s_ui.market_names[item_index] = CreateLabel(row, "待配置");
        lv_label_set_long_mode(s_ui.market_names[item_index], LV_LABEL_LONG_DOT);
        lv_obj_set_pos(s_ui.market_names[item_index], 5, 4);
        lv_obj_set_size(s_ui.market_names[item_index], 76, 23);
        lv_obj_set_style_text_align(s_ui.market_names[item_index],
                                    LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);

        s_ui.market_codes[item_index] =
            CreateLabel(row, "--", &lv_font_noto_sans_sc_13);
        lv_obj_set_pos(s_ui.market_codes[item_index], 5, 32);
        lv_obj_set_size(s_ui.market_codes[item_index], 78, 18);
        lv_obj_set_style_text_align(s_ui.market_codes[item_index],
                                    LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);

        s_ui.market_prices[item_index] =
            CreateLabel(row, "--", &lv_font_MISANSMEDIUM_20);
        lv_obj_set_pos(s_ui.market_prices[item_index], 84, 5);
        lv_obj_set_size(s_ui.market_prices[item_index], 88, 28);
        lv_obj_set_style_text_align(s_ui.market_prices[item_index],
                                    LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);

        s_ui.market_changes[item_index] =
            CreateLabel(row, "--", &lv_font_MISANSMEDIUM_18);
        lv_obj_set_pos(s_ui.market_changes[item_index], 84, 35);
        lv_obj_set_size(s_ui.market_changes[item_index], 88, 25);
        lv_obj_set_style_text_align(s_ui.market_changes[item_index],
                                    LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);

        lv_obj_t *chart = lv_chart_create(row);
        s_ui.market_charts[item_index] = chart;
        lv_obj_set_pos(chart, 180, 5);
        lv_obj_set_size(chart, 214, height - 10);
        lv_obj_set_style_bg_color(chart, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(chart, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(chart, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(chart, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(chart, 2, LV_PART_MAIN);
        lv_obj_set_style_line_color(chart, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_line_opa(chart, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_line_width(chart, 1, LV_PART_MAIN);
        lv_obj_set_style_line_width(chart, 2, LV_PART_ITEMS);
        lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);
        lv_obj_clear_flag(chart, LV_OBJ_FLAG_SCROLLABLE);
        lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
        lv_chart_set_point_count(chart, APP_MARKET_CHART_POINT_COUNT);
        lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -10, 10);
        lv_chart_set_div_line_count(chart, 3, 0);
        s_ui.market_series[item_index] =
            lv_chart_add_series(chart, lv_color_black(),
                                LV_CHART_AXIS_PRIMARY_Y);
        lv_chart_set_all_value(chart, s_ui.market_series[item_index],
                               LV_CHART_POINT_NONE);
    }

    if (page_index > 0) {
        char page_marker[8];
        snprintf(page_marker, sizeof(page_marker), "%u/2",
                 static_cast<unsigned>(page_index + 1));
        lv_obj_t *marker =
            CreateLabel(page, page_marker, &lv_font_noto_sans_sc_13);
        lv_obj_set_pos(marker, 368, 0);
        lv_obj_set_size(marker, 30, 17);
        lv_obj_set_style_bg_color(marker, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(marker, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_text_align(marker, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
        lv_obj_move_foreground(marker);
    }
}

void CreateSettingsPage()
{
    lv_obj_t *page = CreatePage();
    s_ui.pages[static_cast<size_t>(PageId::Settings)] = page;

    lv_obj_t *heading = CreateLabel(page, "设备设置");
    lv_obj_set_pos(heading, 12, 10);
    lv_obj_set_size(heading, 376, 34);

    lv_obj_t *settings_box = lv_obj_create(page);
    lv_obj_set_pos(settings_box, 18, 52);
    lv_obj_set_size(settings_box, 364, 139);
    SetObjectBox(settings_box, 4);

    lv_obj_t *wifi_title = CreateLabel(settings_box, "无线网络");
    lv_obj_set_pos(wifi_title, 12, 9);
    lv_obj_set_size(wifi_title, 75, 22);
    lv_obj_set_style_text_align(wifi_title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    s_ui.settings_wifi = CreateLabel(settings_box, "连接中");
    lv_obj_set_pos(s_ui.settings_wifi, 95, 9);
    lv_obj_set_size(s_ui.settings_wifi, 255, 22);
    lv_obj_set_style_text_align(s_ui.settings_wifi, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);

    lv_obj_t *sd_title = CreateLabel(settings_box, "存储卡");
    lv_obj_set_pos(sd_title, 12, 40);
    lv_obj_set_size(sd_title, 75, 22);
    lv_obj_set_style_text_align(sd_title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    s_ui.settings_sd = CreateLabel(settings_box, "读取中");
    lv_obj_set_pos(s_ui.settings_sd, 95, 40);
    lv_obj_set_size(s_ui.settings_sd, 255, 22);
    lv_obj_set_style_text_align(s_ui.settings_sd, LV_TEXT_ALIGN_RIGHT,
                                LV_PART_MAIN);

    lv_obj_t *web_title = CreateLabel(settings_box, "网页配置");
    lv_obj_set_pos(web_title, 12, 71);
    lv_obj_set_size(web_title, 90, 22);
    lv_obj_set_style_text_align(web_title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    s_ui.settings_web = CreateLabel(settings_box, "等待网络");
    lv_label_set_long_mode(s_ui.settings_web, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(s_ui.settings_web, 105, 71);
    lv_obj_set_size(s_ui.settings_web, 245, 22);
    lv_obj_set_style_text_align(s_ui.settings_web, LV_TEXT_ALIGN_RIGHT,
                                LV_PART_MAIN);

    lv_obj_t *sleep_title = CreateLabel(settings_box, "休眠计划");
    lv_obj_set_pos(sleep_title, 12, 102);
    lv_obj_set_size(sleep_title, 90, 22);
    lv_obj_set_style_text_align(sleep_title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    s_ui.settings_refresh = CreateLabel(settings_box, "每日 22:00 至 08:30");
    lv_obj_set_pos(s_ui.settings_refresh, 105, 102);
    lv_obj_set_size(s_ui.settings_refresh, 245, 22);
    lv_obj_set_style_text_align(s_ui.settings_refresh, LV_TEXT_ALIGN_RIGHT,
                                LV_PART_MAIN);
}

void RestoreFooterHelp()
{
    if (s_current_page == PageId::Market1 ||
        s_current_page == PageId::Market2) {
        lv_label_set_text(s_ui.footer,
                          "KEY 上一页 | BOOT 下一页 | 双击刷新");
    } else {
        lv_label_set_text(s_ui.footer,
                          "KEY 上一页 | BOOT 下一页 | 长按操作");
    }
    s_footer_is_message = false;
}

void ShowFooterMessage(const char *message)
{
    lv_label_set_text(s_ui.footer, message);
    s_footer_restore_at = xTaskGetTickCount() + kFooterMessagePeriod;
    s_footer_is_message = true;
}

void ShowPage(PageId page)
{
    for (size_t i = 0; i < static_cast<size_t>(PageId::Count); ++i) {
        if (i == static_cast<size_t>(page)) {
            lv_obj_clear_flag(s_ui.pages[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(s_ui.pages[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    s_current_page = page;
    const bool market_fullscreen = page == PageId::Market1 ||
                                   page == PageId::Market2;
    const bool hide_footer = market_fullscreen || page == PageId::Home;
    if (market_fullscreen) {
        lv_obj_add_flag(s_ui.status_bar, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(s_ui.status_bar, LV_OBJ_FLAG_HIDDEN);
    }
    if (hide_footer) {
        lv_obj_add_flag(s_ui.footer_box, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(s_ui.footer_box, LV_OBJ_FLAG_HIDDEN);
    }
    if (!market_fullscreen) {
        lv_label_set_text(s_ui.status_title, PageTitle(page));
    }
    if (page == PageId::Market1) {
        AppServices_SetMarketPage(0);
    } else if (page == PageId::Market2) {
        AppServices_SetMarketPage(1);
    }
    RestoreFooterHelp();
}

void MovePage(int delta)
{
    const int count = static_cast<int>(PageId::Count);
    int index = static_cast<int>(s_current_page) + delta;
    index = (index % count + count) % count;
    ShowPage(static_cast<PageId>(index));
}

void RequestRefresh()
{
    AppServices_RequestRefresh();
    ShowFooterMessage("刷新请求已发送");
}

void OpenCurrentPage()
{
    switch (s_current_page) {
        case PageId::Home:
            ShowFooterMessage("首页数据正在实时更新");
            break;
        case PageId::Market1:
        case PageId::Market2:
            RequestRefresh();
            break;
        case PageId::Settings:
            AppServices_ToggleWeb();
            ShowFooterMessage("网页配置开关请求已发送");
            break;
        default:
            break;
    }
}

void HandleButtonEvents(EventBits_t boot_events, EventBits_t key_events)
{
    if (boot_events & kButtonLong) {
        ShowPage(PageId::Home);
        ShowFooterMessage("首页");
    } else if (boot_events & kButtonDouble) {
        ShowPage(PageId::Settings);
        ShowFooterMessage("设备设置");
    } else if (boot_events & kButtonSingle) {
        MovePage(1);
    }

    if (key_events & kButtonLong) {
        OpenCurrentPage();
    } else if (key_events & kButtonDouble) {
        RequestRefresh();
    } else if (key_events & kButtonSingle) {
        MovePage(-1);
    }
}

bool IsRtcValid(const rtcTimeStruct_t &time)
{
    return time.year >= 2020 && time.year <= 2099 &&
           time.month >= 1 && time.month <= 12 &&
           time.day >= 1 && time.day <= 31 &&
           time.hour >= 0 && time.hour <= 23 &&
           time.minute >= 0 && time.minute <= 59;
}

void UpdateRtcLabels(const rtcTimeStruct_t &time)
{
    if (!IsRtcValid(time)) {
        lv_label_set_text(s_ui.status_time, "-- -- 周-");
        lv_label_set_text(s_ui.home_time, "--:--");
        lv_label_set_text(s_ui.home_date, "时间未同步");
        return;
    }

    static const char *const kWeekdays[] = {
        "日", "一", "二", "三", "四", "五", "六"
    };
    struct tm date = {};
    date.tm_year = time.year - 1900;
    date.tm_mon = time.month - 1;
    date.tm_mday = time.day;
    date.tm_hour = 12;
    mktime(&date);
    const int weekday = date.tm_wday >= 0 && date.tm_wday <= 6
                            ? date.tm_wday
                            : 0;

    char buffer[48];
    snprintf(buffer, sizeof(buffer), "%02d-%02d 周%s",
             time.month, time.day, kWeekdays[weekday]);
    lv_label_set_text(s_ui.status_time, buffer);

    snprintf(buffer, sizeof(buffer), "%02d:%02d", time.hour, time.minute);
    lv_label_set_text(s_ui.home_time, buffer);

    snprintf(buffer, sizeof(buffer), "%04d年%02d月%02d日  星期%s",
             time.year, time.month, time.day, kWeekdays[weekday]);
    lv_label_set_text(s_ui.home_date, buffer);
}

void UpdateSensorLabels(float temperature, float humidity, bool valid)
{
    if (!valid) {
        lv_label_set_text(s_ui.home_temperature, "-- C");
        lv_label_set_text(s_ui.home_humidity, "-- %");
        return;
    }

    char buffer[24];
    snprintf(buffer, sizeof(buffer), "%.1f C", temperature);
    lv_label_set_text(s_ui.home_temperature, buffer);
    snprintf(buffer, sizeof(buffer), "%.0f %%", humidity);
    lv_label_set_text(s_ui.home_humidity, buffer);
}

void UpdateBatteryLabel(uint8_t battery_level)
{
    char buffer[24];
    snprintf(buffer, sizeof(buffer), "电量 %u%%",
             static_cast<unsigned>(battery_level));
    lv_label_set_text(s_ui.status_battery, buffer);
}

void UpdateMarketChart(size_t index, const AppMarketItem &market)
{
    int range = 10;
    for (size_t point = 0; point < APP_MARKET_CHART_POINT_COUNT; ++point) {
        const int value = market.intraday[point];
        if (value != APP_MARKET_POINT_NONE) {
            const int magnitude = value < 0 ? -value : value;
            if (magnitude > range) {
                range = magnitude;
            }
        }
    }
    range = ((range + 9) / 10) * 10;
    if (range > 30000) {
        range = 30000;
    }
    lv_chart_set_range(s_ui.market_charts[index],
                       LV_CHART_AXIS_PRIMARY_Y, -range, range);
    for (size_t point = 0; point < APP_MARKET_CHART_POINT_COUNT; ++point) {
        const lv_coord_t value = market.intraday_points > 0 &&
                                 market.intraday[point] !=
                                     APP_MARKET_POINT_NONE
                                     ? market.intraday[point]
                                     : LV_CHART_POINT_NONE;
        lv_chart_set_value_by_id(s_ui.market_charts[index],
                                 s_ui.market_series[index], point, value);
    }
    lv_chart_refresh(s_ui.market_charts[index]);
}

void UpdateConnectivityLabels()
{
    const int ap_count = user_esp_bsp.apNum;
    const bool wifi_connected = user_esp_bsp.connected;
    char wifi_ip[sizeof(user_esp_bsp._ip)] = {};
    strlcpy(wifi_ip, user_esp_bsp._ip, sizeof(wifi_ip));
    const int wifi_rssi = user_esp_bsp.rssi;
    AppServiceSnapshot services = {};
    AppServices_GetSnapshot(&services);

    char buffer[160];
    if (wifi_connected) {
        snprintf(buffer, sizeof(buffer), "无线 OK");
    } else if (ap_count > 0) {
        snprintf(buffer, sizeof(buffer), "无线 ...");
    } else {
        snprintf(buffer, sizeof(buffer), "无线 --");
    }
    lv_label_set_text(s_ui.status_wifi, buffer);

    const char *last_update = services.last_update[0] != '\0'
                                  ? services.last_update
                                  : "等待更新";
    const char *update_time = strrchr(last_update, ' ');
    update_time = update_time != nullptr ? update_time + 1 : last_update;
    if (services.sleep_schedule_enabled) {
        snprintf(buffer, sizeof(buffer), "数据 %s  |  %02u:%02u 休眠",
                 update_time,
                 static_cast<unsigned>(services.sleep_hour),
                 static_cast<unsigned>(services.sleep_minute));
    } else {
        snprintf(buffer, sizeof(buffer), "数据 %s  |  休眠关闭", update_time);
    }
    lv_label_set_text(s_ui.home_connection, buffer);

    if (wifi_connected) {
        snprintf(buffer, sizeof(buffer), "%s  %d dBm", wifi_ip, wifi_rssi);
    } else if (ap_count > 0) {
        snprintf(buffer, sizeof(buffer), "连接中，发现 %d 个热点", ap_count);
    } else {
        snprintf(buffer, sizeof(buffer), "正在扫描和连接");
    }
    lv_label_set_text(s_ui.settings_wifi, buffer);

    if (services.sd_ready) {
        snprintf(buffer, sizeof(buffer), "正常，可用 %lu MB",
                 static_cast<unsigned long>(services.sd_free_mb));
    } else {
        snprintf(buffer, sizeof(buffer), "未插入或读取失败");
    }
    lv_label_set_text(s_ui.settings_sd, buffer);

    if (services.web_running) {
        lv_label_set_text(s_ui.settings_web, services.web_url);
    } else {
        lv_label_set_text(s_ui.settings_web, "长按 KEY 开启");
    }

    if (s_market_rendered_generation != services.market_generation) {
        for (size_t i = 0; i < APP_MARKET_BENCHMARK_COUNT; ++i) {
            snprintf(buffer, sizeof(buffer), "%s  %s",
                     i == 0 ? "上证" : "科创",
                     services.benchmarks[i].valid
                         ? services.benchmarks[i].amplitude
                         : "--");
            lv_label_set_text(s_ui.market_benchmarks[i], buffer);
        }
        for (size_t i = 0; i < APP_MARKET_ITEM_COUNT; ++i) {
            if (services.market[i].valid) {
                lv_label_set_text(s_ui.market_names[i],
                                  services.market[i].name);

                const char *market = strstr(services.market[i].symbol, ".SS")
                                         ? "沪"
                                         : "深";
                snprintf(buffer, sizeof(buffer), "%.6s %s",
                         services.market[i].symbol, market);
                lv_label_set_text(s_ui.market_codes[i], buffer);
                lv_label_set_text(s_ui.market_prices[i],
                                  services.market[i].price);
                lv_label_set_text(s_ui.market_changes[i],
                                  services.market[i].change);
                UpdateMarketChart(i, services.market[i]);
            } else {
                lv_label_set_text(s_ui.market_names[i], "待配置");
                lv_label_set_text(s_ui.market_codes[i], "--");
                lv_label_set_text(s_ui.market_prices[i], "--");
                lv_label_set_text(s_ui.market_changes[i], "--");
                AppMarketItem empty = {};
                UpdateMarketChart(i, empty);
            }
        }
        s_market_rendered_generation = services.market_generation;
    }
    if (services.sleep_imminent) {
        lv_label_set_text(s_ui.settings_refresh, "即将进入深度休眠");
    } else if (services.sleep_schedule_enabled) {
        snprintf(buffer, sizeof(buffer), "每日 %02u:%02u 至 %02u:%02u",
                 static_cast<unsigned>(services.sleep_hour),
                 static_cast<unsigned>(services.sleep_minute),
                 static_cast<unsigned>(services.wake_hour),
                 static_cast<unsigned>(services.wake_minute));
        lv_label_set_text(s_ui.settings_refresh, buffer);
    } else {
        lv_label_set_text(s_ui.settings_refresh, "已关闭");
    }
}

void UiTask(void *argument)
{
    (void)argument;
    TickType_t last_rtc = 0;
    TickType_t last_sensor = 0;
    TickType_t last_battery = 0;
    TickType_t last_connectivity = 0;
    bool first_update = true;

    for (;;) {
        const EventBits_t boot_events = xEventGroupWaitBits(
            BootButtonGroups, kAllButtonEvents, pdTRUE, pdFALSE, 0);
        const EventBits_t key_events = xEventGroupWaitBits(
            GP18ButtonGroups, kAllButtonEvents, pdTRUE, pdFALSE, 0);

        const TickType_t now = xTaskGetTickCount();
        const bool update_rtc = first_update || (now - last_rtc >= kRtcUpdatePeriod);
        const bool update_sensor = first_update ||
                                   (now - last_sensor >= kSensorUpdatePeriod);
        const bool update_battery = first_update ||
                                    (now - last_battery >= kBatteryUpdatePeriod);
        const bool update_connectivity = first_update ||
            (now - last_connectivity >= kConnectivityUpdatePeriod);

        rtcTimeStruct_t rtc_time = {};
        float temperature = 0.0f;
        float humidity = 0.0f;
        bool sensor_valid = false;
        uint8_t battery_level = 0;

        if (update_rtc) {
            Rtc_GetTime(&rtc_time);
            last_rtc = now;
        }
        if (update_sensor && s_shtc3 != nullptr) {
            sensor_valid =
                (s_shtc3->Shtc3_ReadTempHumi(&temperature, &humidity) == NO_ERROR);
            last_sensor = now;
        }
        if (update_battery) {
            battery_level = Adc_GetBatteryLevel();
            last_battery = now;
        }
        if (update_connectivity) {
            last_connectivity = now;
        }

        if (Lvgl_lock(1000)) {
            if (boot_events != 0 || key_events != 0) {
                HandleButtonEvents(boot_events, key_events);
            }
            if (update_rtc) {
                UpdateRtcLabels(rtc_time);
            }
            if (update_sensor) {
                UpdateSensorLabels(temperature, humidity, sensor_valid);
            }
            if (update_battery) {
                UpdateBatteryLabel(battery_level);
            }
            if (update_connectivity) {
                UpdateConnectivityLabels();
            }
            if (s_footer_is_message &&
                static_cast<int32_t>(now - s_footer_restore_at) >= 0) {
                RestoreFooterHelp();
            }
            Lvgl_unlock();
        }

        first_update = false;
        vTaskDelay(kUiPollPeriod);
    }
}

}  // namespace

void UserApp_AppInit()
{
    ESP_LOGI(kTag, "Initializing buttons, sensors, RTC and WiFi; Bluetooth disabled");
    Adc_PortInit();
    Custom_ButtonInit();
    Rtc_Setup(&s_i2c_bus, 0x51);
    s_shtc3 = new Shtc3Port(s_i2c_bus);
    espwifi_init();
    AppServices_Init();

    // Do not set RTC to a fixed build-time value here. The retained RTC value is
    // displayed now and will be synchronized from NTP in the networking phase.
}

void UserApp_UiInit()
{
    s_ui.screen = lv_scr_act();
    lv_obj_clean(s_ui.screen);
    lv_obj_set_style_bg_color(s_ui.screen, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_ui.screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(s_ui.screen, LV_OBJ_FLAG_SCROLLABLE);

    CreateStatusBar();
    CreateHomePage();
    CreateMarketPage(PageId::Market1, 0, false);
    CreateMarketPage(PageId::Market2, 1, true);
    CreateSettingsPage();
    CreateFooter();
    ShowPage(PageId::Home);

    ESP_LOGI(kTag, "Four-page UI initialized");
}

void UserApp_TaskInit()
{
    xTaskCreatePinnedToCore(UiTask, "ui_state_task", 6144, nullptr, 4,
                            nullptr, 1);
    AppServices_Start();
}
