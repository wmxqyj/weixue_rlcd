# weixue_rlcd

ESP32-S3-RLCD-4.2 的 ESP-IDF 开发工程。当前以微雪官方 `10_FactoryProgram` 为硬件验证和后续功能开发基线。

## 当前硬件与软件配置

| 项目 | 配置 |
| --- | --- |
| SoC | ESP32-S3，双核 160 MHz |
| ESP-IDF | v5.5.2 |
| Flash | 16 MB，QIO，80 MHz |
| PSRAM | 8 MB Octal PSRAM，80 MHz |
| 应用分区 | Factory，偏移 `0x10000`，大小 8 MB |
| 串口监视 | 115200 baud |
| 本机烧录端口 | COM8（仅本机配置，不提交） |
| GUI | LVGL 8.4.0 |
| 音频 | ES8311 输出、ES7210 输入、I2S/TDM |

首次验证已经完成：编译、COM8 烧录和 SHA 校验成功；Flash、PSRAM 内存测试、温湿度传感器、Wi-Fi、BLE、音频和 LVGL 显示驱动均正常初始化。未插入 microSD 卡时，官方 FactoryProgram 会记录 `ESP_ERR_TIMEOUT (0x107)`，随后继续运行，这不是致命错误。

## 仓库内容

- `ESP32-S3-RLCD-4.2-Demo/02_ESP-IDF/10_FactoryProgram/`：当前固件源码、板级组件、`sdkconfig`、分区表和依赖锁定文件。
- `.vscode/tasks.json`：Trae/VS Code 构建、menuconfig、烧录及监视任务。
- `tools/esp-idf.ps1`：固定使用仓库内 E 盘工具环境的 PowerShell 入口。

ESP-IDF 工具链、Python 虚拟环境、组件缓存、完整微雪示例包、`managed_components` 和 `build` 都不会提交到 Git。

## 本机构建

当前本机把 ESP-IDF 和工具安装在仓库根目录下：

```text
.espressif/v5.5.2/esp-idf
.idf-tools
```

在 PowerShell 中执行：

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tools\esp-idf.ps1 build
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tools\esp-idf.ps1 menuconfig
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tools\esp-idf.ps1 flash -Port COM8
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tools\esp-idf.ps1 monitor -Port COM8
```

也可以在 Trae 中使用“任务：运行任务”。退出串口监视器使用 `Ctrl+]`。

## 来源

硬件资料与初始 FactoryProgram 来自 [Waveshare ESP32-S3-RLCD-4.2 官方资源](https://docs.waveshare.net/ESP32-S3-RLCD-4.2/Resources-And-Documents)。后续自定义开发在本仓库中维护。
