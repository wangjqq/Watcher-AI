#pragma once

#include <stdint.h>

/* 单按键输入事件
 *  SHORT_PRESS：短按 = 确认/选择
 *  LONG_PRESS：长按（>1秒） = 返回
 */
typedef enum {
    BUTTON_EV_SHORT = 0,
    BUTTON_EV_LONG,
    BUTTON_EV_COUNT
} button_event_t;

typedef void (*button_handler_t)(button_event_t ev);

/* 初始化：配置 GPIO（内部上拉）+ 启动 10ms 扫描任务 */
void button_init(void);

/* 注册事件回调（传入 NULL 清除） */
void button_set_handler(button_handler_t cb);
