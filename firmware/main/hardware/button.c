#include "button.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "button";

/* 单按键使用原旋钮 SW 的引脚：GPIO7（避开 26~37），另一脚 → GND */
#define BUTTON_GPIO     GPIO_NUM_7

#define ACTIVE_LEVEL    0   /* 按下 = 低电平 */
#define DEBOUNCE_MS     30  /* 按键消抖窗口 */
#define SCAN_PERIOD_MS  10  /* 扫描周期 */
#define LONG_PRESS_MS   1000/* 长按判定 */

static button_handler_t s_handler = NULL;

/* 消抖状态 */
typedef struct {
    uint8_t stable;
    uint8_t pending;
    uint8_t debounce_cnt;
    uint8_t held;
    uint32_t press_start_ms;
} button_state_t;

static button_state_t s_btn = { 0 };

static void emit(button_event_t ev)
{
    if (s_handler) {
        s_handler(ev);
    }
}

static void scan_button(void)
{
    int level = gpio_get_level(BUTTON_GPIO);

    if (level != s_btn.stable) {
        s_btn.pending = (uint8_t)level;
        if (++s_btn.debounce_cnt >= (DEBOUNCE_MS / SCAN_PERIOD_MS)) {
            s_btn.stable = s_btn.pending;
            s_btn.debounce_cnt = 0;
            if (s_btn.stable == ACTIVE_LEVEL) {
                /* 按下：记录时间 */
                s_btn.held = 1;
                s_btn.press_start_ms = (uint32_t)(esp_timer_get_time() / 1000);
            } else if (s_btn.held) {
                /* 松开：检查是短按还是长按 */
                s_btn.held = 0;
                uint32_t press_duration = (uint32_t)(esp_timer_get_time() / 1000) - s_btn.press_start_ms;
                if (press_duration >= LONG_PRESS_MS) {
                    emit(BUTTON_EV_LONG);
                } else {
                    emit(BUTTON_EV_SHORT);
                }
            }
        }
    } else {
        s_btn.debounce_cnt = 0;
    }
}

static void button_task(void *arg)
{
    (void)arg;
    for (;;) {
        scan_button();
        vTaskDelay(pdMS_TO_TICKS(SCAN_PERIOD_MS));
    }
}

void button_init(void)
{
    gpio_config_t io = { 0 };
    io.pin_bit_mask = 1ULL << BUTTON_GPIO;
    io.mode = GPIO_MODE_INPUT;
    io.pull_up_en = GPIO_PULLUP_ENABLE;
    io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io);

    xTaskCreate(button_task, "button", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "init ok (GPIO=%d)", BUTTON_GPIO);
}

void button_set_handler(button_handler_t cb)
{
    s_handler = cb;
}
