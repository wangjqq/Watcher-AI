# Watcher-AI 股票盯盘设备

基于 **ESP32-S3 NANO（N16R8，16 MB Flash / 8 MB PSRAM）** 的股票盯盘设备，配有 ILI9341 240 × 320 彩屏、XPT2046 电阻触摸和单按键。用户提供返回 JSON 的数据接口，在内置管理网页上选择字段、配置屏幕布局和提醒，设备按各接口的周期刷新数据。

固件的 OTA 双分区和字库需要 16 MB Flash。屏幕背光由 GPIO4 经 S8050 低边驱动，接线见 [PINMAP](firmware/PINMAP.md)。

## 使用流程

装有 SW2 的板卡用拨动开关开关机。关机充电时，拔掉主控调试 USB 线，把充电线接到 TP4056 模块的 USB-C 口。充电状态看模块自带的指示灯。

1. 在设备屏幕打开「系统 → WiFi」，选择无线网络并输入密码。手机或电脑接入同一局域网后，可访问 `http://stockwatcher.local` 或设备 IP，也可扫描「系统 → QR」里的二维码。
2. 在「接口配置」添加并测试数据源，在「应用列表」建立应用，然后在「字段解析」把需要的字段添加到目标应用。
3. 在「屏幕布局」调整显示块并保存。屏幕顶部 16 像素用作状态栏，剩余 240 × 304 像素可配置。
4. 可在「提醒设置」配置字段阈值，在「设备设置」调整休眠或导入导出配置。导出的配置可能包含 Wi-Fi 密码及接口凭据，请妥善保管。

详细操作见网页侧边栏的「用户手册」和设备上的「系统 → Manual」。首次使用触摸屏时，在「系统 → TouchCal」按提示完成两点标定。

## 构建固件

先准备 Node.js、npm 和已初始化的 ESP-IDF 5.x 环境。以下命令从仓库根目录运行，`COMx` 替换为设备串口。

```powershell
cd web
npm ci
npm run build

cd ../firmware
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash
```

Vite 将网页产物写入 `firmware/web_dist/`，固件配置阶段会将产物嵌入应用镜像。已经刷入兼容 OTA 分区表的设备，可在网页「固件升级」上传 `firmware/build/stock_watcher.bin`。

### 中文字库

字库单独存放在 [fonts 分区](firmware/partitions.csv)，可单独写入。设备须先刷入包含该分区的分区表。在 ESP-IDF PowerShell 环境下，从 `firmware/` 运行：

```powershell
python tools/gen_font.py
python "$env:IDF_PATH/components/partition_table/parttool.py" --port COMx write_partition --partition-name=fonts --input build/fonts.bin
```

生成器优先读取 `firmware/tools/HZK16.bin`，也可使用 Pillow 与支持中文的系统字体。两者均不可用时会生成空字库，屏幕上的中文将显示为方块。应用 OTA 保留设备上的字库分区。

## 开发参考

- [项目开发约定](AGENTS.md)
- [产品硬件接线](firmware/PINMAP.md)与[开发板原理图、引脚资料及厂商例程](docs/hardware/esp32s3-nano/README.md)
- [ESP32 固件](firmware/main/)与[网页管理界面](web/src/)
- [设备 REST API 路由](firmware/main/network/http_server.c)与[网页 API 客户端](web/src/api/client.ts)
