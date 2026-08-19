# weixue_rlcd

微雪 ESP32-S3-RLCD-4.2 的 ESP-IDF 固件工程。项目以官方 `10_FactoryProgram` 为硬件基线，当前已经改造成中文桌面信息屏，提供首页、A 股行情和设备设置三页界面。

## 当前功能

- 首页：显示时间、日期、温度、湿度、Wi-Fi 状态和电量。
- A 股行情：一屏显示 4 只股票/指数的当前价格、涨跌幅和当天分时图。
- 默认标的：上证指数、科创指数、紫金矿业、大中矿业。
- 行情刷新：A 股交易时段每 1 分钟刷新，非交易时段每 15 分钟刷新；也可通过按键或网页立即刷新。
- 时间同步：联网后通过 NTP 校时，并同步到板载 RTC。
- 网页配置：在局域网内修改 Wi-Fi、4 个股票名称和代码、休眠计划，无需长期打开网页。
- 定时休眠：默认每天 22:00 进入深度休眠，次日 08:30 自动唤醒；KEY 手动唤醒后暂停休眠计划 30 分钟。
- 配置持久化：Wi-Fi、股票和休眠设置保存到 NVS，断电后保留。
- microSD：可用时只保存最新一份行情缓存 `/sdcard/cache/market.json`，不记录长期历史。
- 蓝牙已关闭，新闻模块已移除。

## 页面和按键

页面顺序为：首页 → 行情 → 设置。

| 按键 | 单击 | 双击 | 长按 |
| --- | --- | --- | --- |
| KEY | 下一页 | 立即刷新行情 | 首页提示；行情页立即刷新；设置页开启/关闭配置网页 |
| BOOT | 上一页 | 跳转设置页 | 返回首页 |

行情页为全屏四宫格布局，不显示额外的上下操作提示。

## 网页配置

设备连接 Wi-Fi 后会在设置页显示访问地址，通常为：

```text
http://<设备局域网 IP>/
```

网页可以修改：

- Wi-Fi 名称和密码；密码留空表示不修改。
- 4 个 A 股名称和代码。
- 每日休眠和唤醒时间。
- 手动触发行情刷新。

股票代码使用六位代码加市场后缀：沪市为 `.SS`，深市为 `.SZ`，例如 `601899.SS`、`001203.SZ`。网页空闲 10 分钟后自动关闭，需要时可在设置页长按 KEY 再次开启。

## Wi-Fi 凭据与隐私

仓库不会提交实际 Wi-Fi 名称和密码。已配置设备从 NVS 读取凭据；首次烧录新设备时，可复制本地模板：

```powershell
Copy-Item `
  .\ESP32-S3-RLCD-4.2-Demo\02_ESP-IDF\10_FactoryProgram\components\app_bsp\wifi_credentials.example.h `
  .\ESP32-S3-RLCD-4.2-Demo\02_ESP-IDF\10_FactoryProgram\components\app_bsp\wifi_credentials.local.h
```

然后只在 `wifi_credentials.local.h` 中填写初始 Wi-Fi。该文件已被 `.gitignore` 排除；联网后可改用设备网页更新凭据。

## 硬件与软件配置

| 项目 | 配置 |
| --- | --- |
| 开发板 | Waveshare ESP32-S3-RLCD-4.2 |
| SoC | ESP32-S3，双核 160 MHz |
| ESP-IDF | v5.5.2 |
| Flash | 16 MB，QIO，80 MHz |
| PSRAM | 8 MB Octal PSRAM，80 MHz |
| 应用分区 | Factory，偏移 `0x10000`，大小 8 MB |
| 显示 | 400 × 300 RLCD，LVGL 8.4.0 |
| 串口监视 | 115200 baud |
| 本机烧录端口 | COM8，可通过参数修改 |

当前固件已经完成 ESP-IDF 编译、COM8 烧录和串口验证：Flash/PSRAM 初始化正常，能够连接 Wi-Fi、完成 NTP 同步并刷新 4 只 A 股行情。

microSD 属于可选缓存。挂载失败时设备仍可正常显示和联网，只是不读取或写入行情缓存；可从串口日志进一步检查卡格式、接触和驱动状态。

## 仓库结构

- `ESP32-S3-RLCD-4.2-Demo/02_ESP-IDF/10_FactoryProgram/`：当前固件工程。
- `components/user_app/user_app.cpp`：三页 LVGL 界面和按键交互。
- `components/user_app/app_services.cpp`：行情、NTP、网页配置、NVS、SD 缓存和定时休眠服务。
- `components/app_bsp/esp_wifi_bsp.c`：Wi-Fi STA、NVS 凭据和自动重连。
- `.vscode/tasks.json`：Trae/VS Code 构建、配置、烧录及监视任务。
- `tools/esp-idf.ps1`：固定使用仓库内 E 盘 ESP-IDF 环境的 PowerShell 入口。

ESP-IDF 工具链、Python 虚拟环境、缓存、`managed_components` 和 `build` 目录均不会提交到 Git。

## 本机构建与烧录

本机 ESP-IDF 和工具位于仓库根目录下，不占用 C 盘：

```text
.espressif/v5.5.2/esp-idf
.idf-tools
```

在仓库根目录打开 PowerShell：

```powershell
.\tools\esp-idf.ps1 build
.\tools\esp-idf.ps1 menuconfig
.\tools\esp-idf.ps1 flash -Port COM8
.\tools\esp-idf.ps1 monitor -Port COM8
```

也可以在 Trae 中运行对应任务。串口监视器使用 `Ctrl+]` 退出。

## 当前说明

- 行情数据来自公开网络接口，需要设备正常联网；接口不可用时保留屏幕上的最近有效数据。
- 涨跌幅由当前价格和昨收价在设备本地计算。
- 当前只包含 A 股行情，不包含 BTC、海外市场或新闻模块。
- 中文字体文件及许可说明见工程内 `FONT_LICENSES.md`。

## 资料来源

硬件资料与初始 FactoryProgram 来自 [Waveshare ESP32-S3-RLCD-4.2 官方资源](https://docs.waveshare.net/ESP32-S3-RLCD-4.2/Resources-And-Documents)。后续自定义开发在本仓库中维护。
