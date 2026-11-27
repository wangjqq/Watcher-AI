# 硬件接线说明（PIN 映射）

> 目标芯片：**ESP32-S3 N16R8**（16MB Flash + 8MB Octal PSRAM），开发板为 **ESP32-S3 NANO**。
> 本文件是硬件接线的唯一说明，固件中的引脚定义见各模块 `.c` 文件顶部 `PIN_*` 宏，两者保持一致。
> 构建目标：`idf.py set-target esp32s3`。
>
> 开发板资料（排针引出 / 原理图 / 官方例程）：[docs/hardware/esp32s3-nano/README.md](../docs/hardware/esp32s3-nano/README.md)。
> 本项目所用引脚均已按该开发板排针核对，全部符合。

## ⚠️ 引脚禁区（N16R8 必须避开）

N16R8 的 **8MB Octal PSRAM 与 Flash 通过 SPI 总线连接，占用 GPIO26~37**，这些引脚在模组内部已被占用，
**绝对不可用作 GPIO**（强行使用会导致系统不稳定 / 反复重启）：

- `GPIO26 ~ GPIO32`：SPI0/1 数据/时钟（Flash + PSRAM）
- `GPIO33 ~ GPIO37`：Octal 模式额外数据线（SPIIO4~7 / SPIDQS），其中 35/36/37 硬连接 PSRAM

其余注意项：

- Strapping 引脚：`GPIO0 / GPIO3 / GPIO45 / GPIO46`（上电决定启动模式，尽量避免做普通 IO）
- USB 引脚：`GPIO19 / GPIO20`（USB D-/D+，用作普通 IO 会禁用 USB-JTAG）
- ADC1 只在 `GPIO1 ~ GPIO10`；ADC2（GPIO11~20）在 WiFi 开启时不可用

---

## 屏幕：ILI9341 2.4" 240x320 + 触摸（XPT2046）

模块引脚：`VCC  GND  CS  RESET  DC/RS  SDI(MOSI)  SCK  LED  SDO(MISO)`
触摸引脚：`T_CLK  T_CS  T_DIN  T_DO  T_IRQ`

LCD 与触摸共用一条 SPI 总线（SPI2）：**T_CLK 与 SCK 连到同一个 GPIO，T_DIN 与 SDI(MOSI) 连到同一个 GPIO**。
`T_DO` 接共享 MISO（**LCD 的 SDO 可以不接**，写屏无需读回）；触摸 T_CS 独立片选。

| 模块引脚 | 功能         | ESP32 GPIO | 定义位置                                                   |
| -------- | ------------ | ---------- | ---------------------------------------------------------- |
| VCC      | 电源 3.3V    | 3V3        | 接开发板 3.3V                                              |
| GND      | 地           | GND        | 共地                                                       |
| CS       | LCD 片选     | GPIO5      | `display/display.c` → `PIN_CS`                             |
| RESET    | 复位         | GPIO16     | `PIN_RST`                                                  |
| DC       | 数据/命令    | GPIO17     | `PIN_DC`（也叫 RS/A0）                                     |
| SDI      | LCD 数据输入 | GPIO48     | `PIN_MOSI`                                                 |
| SCK      | 时钟         | GPIO18     | `PIN_SCLK`                                                 |
| LED      | 背光         | GPIO4      | `PIN_BL`，高电平点亮                                       |
| SDO      | LCD 数据输出 | 悬空       | 写屏无需读回，可不接                                       |
| T_CLK    | 触摸时钟     | GPIO18     | 与 SCK 共用 `PIN_SCLK`                                     |
| T_CS     | 触摸片选     | GPIO9      | `hardware/touch.c` → `TOUCH_PIN_CS`                        |
| T_DIN    | 触摸数据输入 | GPIO48     | 与 MOSI 共用 `PIN_MOSI`                                    |
| T_DO     | 触摸数据输出 | GPIO11     | 共享 MISO，`display.c` → `PIN_MISO`                        |
| T_IRQ    | 触摸中断     | GPIO15     | `hardware/touch.c` → `TOUCH_PIN_IRQ`，按下为低（内部上拉） |

> 注：不同批次 ILI9341 面板颜色/方向可能不同，若偏色或反色，参考
> `display.c` 中 `ili9341_init()` 末尾的注释（MADCTL / INVOFF 调整）。

## 单按键

输入设备：普通按键（iPhone 4 风格），按下为低电平，内部上拉，软件消抖。
短按 = 确认/选择，长按（>1秒） = 返回。

> **深度睡眠唤醒**：GPIO7 处于 RTC 域（GPIO0~21），同时作为深度睡眠唤醒键——
> 整机深度睡眠后按下按键即可随时唤醒。

| 信号 | 功能            | ESP32 GPIO | 定义位置                                          |
| ---- | --------------- | ---------- | ------------------------------------------------- |
| BTN  | 按键（按下=低） | GPIO7      | `hardware/button.c` → `BUTTON_GPIO`（避开 26~37） |

接线速查：

```text
按键 → GPIO7（另一脚 → GND）
```

## 无源蜂鸣器

无源蜂鸣器由 LEDC PWM 驱动，可发不同音调。

| 引脚       | ESP32 GPIO | 定义位置                           |
| ---------- | ---------- | ---------------------------------- |
| 蜂鸣器信号 | GPIO13     | `hardware/buzzer.c` → `PIN_BUZZER` |

```text
蜂鸣器信号 → GPIO13
```

## RGB LED（三引脚）

三路独立 PWM 调亮度，固件默认使用**共阳极**（低电平点亮）。

| 引脚 | ESP32 GPIO | 定义位置                                      |
| ---- | ---------- | --------------------------------------------- |
| R    | GPIO38     | `hardware/led.c` → `LED_GPIO_R`（避开 26~37） |
| G    | GPIO21     | `LED_GPIO_G`                                  |
| B    | GPIO42     | `LED_GPIO_B`                                  |

```text
R → GPIO38
G → GPIO21
B → GPIO42
```

> 当前为共阳极 LED（`LED_ACTIVE_HIGH=0`），低电平点亮。若换成共阴极，改为 `1` 即可（自动反相）。

## 光敏传感器 BH1750（I2C）

| 引脚 | ESP32 GPIO | 定义位置                                            |
| ---- | ---------- | --------------------------------------------------- |
| VCC  | 3V3        | -                                                   |
| GND  | GND        | -                                                   |
| SDA  | GPIO8      | `hardware/light_sensor.c` → `PIN_SDA`（避开 26~37） |
| SCL  | GPIO10     | `PIN_SCL`（避开 26~37）                             |
| ADDR | 悬空       | 地址 0x23                                           |

```text
VCC → 3V3
GND → GND
SDA → GPIO8
SCL → GPIO10
```

## 电池电量（ADC 分压采样）

| 引脚  | ESP32 GPIO              | 说明                            |
| ----- | ----------------------- | ------------------------------- |
| 电池+ | ── R1(100K) ──┬── GPIO1 | GPIO1 = ADC1_CH0                |
| 电池- | ── R2(100K) ──┘         | 分压比 2:1，3.0~4.2V → 1.5~2.1V |

```text
电池+ ──R1(100K)──┬── GPIO1
                  │
电池- ──R2(100K)──┘
```

> S3 的 ADC1 通道仅在 GPIO1~10；GPIO1 = ADC1_CH0。分压后电压须落在
> ADC_ATTEN_DB_12 量程（0~3.1V）内。

## 修改接线

修改外设接线后，同步更新对应固件模块的引脚宏和本页的接线表。
