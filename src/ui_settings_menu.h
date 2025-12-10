#ifndef UI_SETTINGS_MENU_H
#define UI_SETTINGS_MENU_H

#include "../lvgl/lvgl.h"

void create_settings_screen(void);
lv_font_t *get_chinese_font(void);
void apply_settings_menu_theme(void); // 應用主題到設定選單

#endif // UI_SETTINGS_MENU_H
