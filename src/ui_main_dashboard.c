#include "ui_main_dashboard.h"
#include "../lvgl/lvgl.h"
#include "ui_state_manager.h"
#include "ui_settings_menu.h"
#include "ui_dashboard_layouts.h"
#include <stdio.h>
#include <stdlib.h>

// 全域物件
static lv_obj_t *g_main_dashboard_screen = NULL; // 保存主儀表板 screen
static lv_obj_t *g_tachometer;
static lv_obj_t *g_tacho_needle; // 轉速表指針
static lv_obj_t *g_speed_label;
static lv_obj_t *g_gear_label;
static lv_obj_t *g_status_container;
static lv_obj_t *g_throttle_slider;
static lv_obj_t *g_brake_slider;
static lv_obj_t *g_debug_panel; // 除錯面板
static lv_obj_t *g_mode_label;  // 模式顯示標籤
static lv_obj_t *g_abs_label;   // ABS顯示標籤
static lv_obj_t *g_tcs_label;   // TCS顯示標籤
static lv_timer_t *ui_refresh_timer;

// 事件回呼函式
static void settings_event_cb(lv_event_t *e);
static void theme_toggle_event_cb(lv_event_t *e);
static void apply_theme(void);
static void update_ui_theme(lv_color_t bg_color, lv_color_t text_color, lv_color_t panel_bg, lv_color_t border_color);
static void update_container_theme(lv_obj_t *container, lv_color_t text_color, lv_color_t panel_bg, lv_color_t border_color, lv_font_t *chinese_font);
static void update_all_containers_theme(lv_obj_t *container, lv_color_t dark_panel_bg, lv_color_t darker_panel_bg,
                                        lv_color_t control_panel_bg, lv_color_t border_color, lv_color_t text_color, lv_font_t *chinese_font);

// 根據主題獲取合適的顏色
static lv_color_t get_theme_color(lv_color_t dark_color, lv_color_t light_color)
{
    return is_dark_theme ? dark_color : light_color;
}

// 根據主題獲取面板背景色
static lv_color_t get_panel_bg_color(void)
{
    return get_theme_color(lv_color_make(20, 20, 20), lv_color_make(220, 220, 220));
}

// 根據主題獲取較暗的面板背景色
static lv_color_t get_darker_panel_bg_color(void)
{
    return get_theme_color(lv_color_make(10, 10, 10), lv_color_make(200, 200, 200));
}

// 根據主題獲取控制面板背景色
static lv_color_t get_control_panel_bg_color(void)
{
    return get_theme_color(lv_color_make(30, 30, 30), lv_color_make(240, 240, 240));
}

// 根據主題獲取文字顏色
static lv_color_t get_text_color(void)
{
    return get_theme_color(lv_color_white(), lv_color_black());
}

// 根據主題獲取邊框顏色
static lv_color_t get_border_color(void)
{
    return get_theme_color(lv_color_make(100, 100, 100), lv_color_make(200, 200, 200));
}

// 應用主題到整個UI
static void apply_theme(void)
{
    lv_obj_t *scr = lv_scr_act();

    if (is_dark_theme)
    {
        // 深色主題
        lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
        lv_obj_set_style_text_color(scr, lv_color_white(), LV_PART_MAIN);

        // 更新所有UI元件的主題
        update_ui_theme(lv_color_black(), lv_color_white(), lv_color_make(30, 30, 30), lv_color_make(100, 100, 100));
    }
    else
    {
        // 淺色主題
        lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
        lv_obj_set_style_text_color(scr, lv_color_black(), LV_PART_MAIN);

        // 更新所有UI元件的主題
        update_ui_theme(lv_color_white(), lv_color_black(), lv_color_make(240, 240, 240), lv_color_make(200, 200, 200));
    }

    // 同時更新設定選單的主題（如果存在）
    apply_settings_menu_theme();
}

// 更新UI元件主題的輔助函數
static void update_ui_theme(lv_color_t bg_color, lv_color_t text_color, lv_color_t panel_bg, lv_color_t border_color)
{
    lv_font_t *chinese_font = get_chinese_font();

    // 更新轉速表
    if (g_tachometer)
    {
        lv_obj_set_style_bg_color(g_tachometer, bg_color, 0);
        lv_obj_set_style_text_color(g_tachometer, text_color, 0);
    }

    // 更新時速和檔位標籤
    if (g_speed_label)
    {
        lv_obj_set_style_text_color(g_speed_label, text_color, 0);
        lv_obj_set_style_text_font(g_speed_label, chinese_font, 0);
    }
    if (g_gear_label)
    {
        lv_obj_set_style_text_color(g_gear_label, text_color, 0);
        lv_obj_set_style_text_font(g_gear_label, chinese_font, 0);
    }

    // 更新設定值顯示標籤
    if (g_mode_label)
    {
        lv_obj_set_style_text_color(g_mode_label, text_color, 0);
        lv_obj_set_style_text_font(g_mode_label, chinese_font, 0);
    }
    if (g_abs_label)
    {
        lv_obj_set_style_text_color(g_abs_label, text_color, 0);
        lv_obj_set_style_text_font(g_abs_label, chinese_font, 0);
    }
    if (g_tcs_label)
    {
        lv_obj_set_style_text_color(g_tcs_label, text_color, 0);
        lv_obj_set_style_text_font(g_tcs_label, chinese_font, 0);
    }

    // 根據主題設置不同的面板背景色
    lv_color_t dark_panel_bg = is_dark_theme ? lv_color_make(20, 20, 20) : lv_color_make(220, 220, 220);
    lv_color_t darker_panel_bg = is_dark_theme ? lv_color_make(10, 10, 10) : lv_color_make(200, 200, 200);
    lv_color_t control_panel_bg = is_dark_theme ? lv_color_make(30, 30, 30) : lv_color_make(240, 240, 240);

    // 更新所有主要容器 - 直接遍歷並設置合適的背景
    lv_obj_t *scr = lv_scr_act();
    update_all_containers_theme(scr, dark_panel_bg, darker_panel_bg, control_panel_bg, border_color, text_color, chinese_font);

    // 更新除錯面板
    if (g_debug_panel)
    {
        lv_obj_set_style_bg_color(g_debug_panel, panel_bg, 0);
        lv_obj_set_style_border_color(g_debug_panel, border_color, 0);
        // 更新除錯面板內的所有標籤
        uint32_t debug_child_cnt = lv_obj_get_child_count(g_debug_panel);
        for (uint32_t i = 0; i < debug_child_cnt; i++)
        {
            lv_obj_t *child = lv_obj_get_child(g_debug_panel, i);
            if (lv_obj_check_type(child, &lv_label_class))
            {
                lv_obj_set_style_text_color(child, text_color, 0);
                lv_obj_set_style_text_font(child, chinese_font, 0);
            }
        }
    }
}

// 遞歸更新所有容器的主題
static void update_all_containers_theme(lv_obj_t *container, lv_color_t dark_panel_bg, lv_color_t darker_panel_bg,
                                        lv_color_t control_panel_bg, lv_color_t border_color, lv_color_t text_color, lv_font_t *chinese_font)
{
    uint32_t child_cnt = lv_obj_get_child_count(container);
    for (uint32_t i = 0; i < child_cnt; i++)
    {
        lv_obj_t *child = lv_obj_get_child(container, i);
        if (child)
        {
            // 設置容器背景 - 根據主題設置不同的深淺度
            lv_obj_set_style_bg_color(child, dark_panel_bg, 0);
            lv_obj_set_style_border_color(child, border_color, 0);

            // 更新標籤顏色
            if (lv_obj_check_type(child, &lv_label_class))
            {
                lv_obj_set_style_text_color(child, text_color, 0);
                lv_obj_set_style_text_font(child, chinese_font, 0);
            }
            // 更新滑桿顏色 (保持功能色)
            else if (lv_obj_check_type(child, &lv_slider_class))
            {
                lv_obj_set_style_bg_color(child, lv_color_make(100, 100, 100), LV_PART_MAIN);
                if (child == g_throttle_slider)
                {
                    lv_obj_set_style_bg_color(child, lv_color_make(255, 100, 100), LV_PART_INDICATOR);
                }
                else if (child == g_brake_slider)
                {
                    lv_obj_set_style_bg_color(child, lv_color_make(100, 100, 255), LV_PART_INDICATOR);
                }
            }
            // 更新按鈕顏色
            else if (lv_obj_check_type(child, &lv_button_class))
            {
                lv_color_t btn_bg = is_dark_theme ? lv_color_make(60, 60, 60) : lv_color_make(180, 180, 180);
                lv_color_t btn_border = is_dark_theme ? lv_color_make(120, 120, 120) : lv_color_make(140, 140, 140);
                lv_obj_set_style_bg_color(child, btn_bg, 0);
                lv_obj_set_style_border_color(child, btn_border, 0);
            }

            // 遞歸處理子容器
            if (lv_obj_get_child_count(child) > 0)
            {
                update_all_containers_theme(child, dark_panel_bg, darker_panel_bg, control_panel_bg, border_color, text_color, chinese_font);
            }
        }
    }
}

// RPM 動畫回呼
static void rpm_anim_cb(void *var, int32_t value)
{
    // 更新轉速表指針位置
    if (g_tacho_needle != NULL && g_tachometer != NULL)
    {
        lv_scale_set_line_needle_value(g_tachometer, g_tacho_needle, 80, value);

        // 根據指針位置亮起對應刻度
        // 轉速表範圍：0-14000 RPM，對應角度約240度
        // 計算指針當前指向的RPM範圍
        int highlighted_rpm_start = value - 500; // 指針前後500 RPM範圍
        int highlighted_rpm_end = value + 500;

        // 根據RPM範圍設定顏色
        lv_color_t normal_color = is_dark_theme ? lv_color_make(100, 100, 100) : lv_color_make(200, 200, 200);
        lv_color_t highlight_color;

        if (value >= 12000) // 紅色危險區
            highlight_color = lv_color_make(255, 100, 100);
        else if (value >= 10000) // 黃色警告區
            highlight_color = lv_color_make(255, 255, 100);
        else if (value >= 8000) // 綠色正常區
            highlight_color = lv_color_make(100, 255, 100);
        else // 藍色低轉速區
            highlight_color = lv_color_make(100, 100, 255);

        // 設置刻度顏色 - 這裡我們使用整體顏色，因為LVGL scale widget限制
        // 在實際實現中，可能需要自定義繪圖來實現精確的刻度亮起
        lv_obj_set_style_line_color(g_tachometer, highlight_color, LV_PART_ITEMS);
        lv_obj_set_style_line_color(g_tachometer, highlight_color, LV_PART_INDICATOR);

        // 同時更新指針顏色
        lv_obj_set_style_line_color(g_tacho_needle, highlight_color, 0);
    }
}

// UI 刷新回呼函式
static void ui_refresh_cb(lv_timer_t *timer)
{
    // 更新儀表板布局（新的布局系統）
    update_dashboard_layout();

    // 更新設定值顯示
    if (g_mode_label != NULL)
    {
        const char *mode_names[] = {"Street", "Sport", "Race", "Rain", "Offroad", "自訂"};
        lv_label_set_text_fmt(g_mode_label, "模式: %s", mode_names[g_vehicle_state.riding_mode]);
    }

    if (g_abs_label != NULL)
    {
        const char *abs_names[] = {"全開", "前後輪全關", "後輪關閉", "Offroad"};
        int abs_idx = 0;
        if (g_vehicle_state.abs_level <= 2)
            abs_idx = 0;
        else if (g_vehicle_state.abs_level >= 7)
            abs_idx = 1;
        else if (g_vehicle_state.abs_level >= 5)
            abs_idx = 2;
        else
            abs_idx = 3;
        lv_label_set_text_fmt(g_abs_label, "ABS: %s", abs_names[abs_idx]);
    }

    if (g_tcs_label != NULL)
    {
        lv_label_set_text_fmt(g_tcs_label, "循跡: %d段", g_vehicle_state.tcs_level);
    }

    // 更新當前RPM（使用動畫）
    if (g_vehicle_state.current_rpm != g_vehicle_state.target_rpm)
    {
        // 簡單的線性更新，避免複雜動畫
        int diff = g_vehicle_state.target_rpm - g_vehicle_state.current_rpm;
        if (abs(diff) > 500)
        {
            g_vehicle_state.current_rpm += diff / 4; // 快速接近
        }
        else
        {
            g_vehicle_state.current_rpm = g_vehicle_state.target_rpm; // 直接設定
        }
    }

    // 更新除錯面板（如果存在）
    if (g_debug_panel != NULL)
    {
        // 找到除錯標籤並更新
        lv_obj_t *child = lv_obj_get_child(g_debug_panel, 1); // RPM標籤
        if (child != NULL)
        {
            lv_label_set_text_fmt(child, "RPM: %d/%d", g_vehicle_state.current_rpm, g_vehicle_state.target_rpm);
        }

        child = lv_obj_get_child(g_debug_panel, 2); // 速度標籤
        if (child != NULL)
        {
            lv_label_set_text_fmt(child, "SPD: %d km/h", g_vehicle_state.current_speed);
        }

        child = lv_obj_get_child(g_debug_panel, 3); // 檔位標籤
        if (child != NULL)
        {
            lv_label_set_text_fmt(child, "GEAR: %d", g_vehicle_state.current_gear);
        }

        child = lv_obj_get_child(g_debug_panel, 4); // 油門標籤
        if (child != NULL)
        {
            lv_label_set_text_fmt(child, "THR: %d%%", get_throttle_input());
        }

        child = lv_obj_get_child(g_debug_panel, 5); // 剎車標籤
        if (child != NULL)
        {
            lv_label_set_text_fmt(child, "BRK: %d%%", get_brake_input());
        }
    }
}

// 控制事件回呼函式
static void throttle_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    set_throttle_input(value);
}

static void brake_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    set_brake_input(value);
}

static void gear_up_event_cb(lv_event_t *e)
{
    shift_gear(1); // 升檔
}

static void gear_down_event_cb(lv_event_t *e)
{
    shift_gear(-1); // 降檔
}

static void debug_toggle_event_cb(lv_event_t *e)
{
    if (g_debug_panel == NULL)
    {
        // 創建除錯面板
        g_debug_panel = lv_obj_create(lv_scr_act());
        lv_obj_set_size(g_debug_panel, 200, 150);
        lv_obj_align(g_debug_panel, LV_ALIGN_BOTTOM_LEFT, 10, -10);
        // 使用主題顏色
        lv_obj_set_style_bg_color(g_debug_panel, is_dark_theme ? lv_color_make(0, 0, 0) : lv_color_make(240, 240, 240), 0);
        lv_obj_set_style_border_width(g_debug_panel, 2, 0);
        lv_obj_set_style_border_color(g_debug_panel, lv_color_make(255, 255, 0), 0);

        lv_obj_t *title = lv_label_create(g_debug_panel);
        lv_label_set_text(title, "DEBUG INFO");
        lv_obj_set_style_text_color(title, lv_color_make(255, 255, 0), 0);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);

        // 除錯資訊標籤
        lv_obj_t *rpm_debug = lv_label_create(g_debug_panel);
        lv_obj_set_style_text_color(rpm_debug, is_dark_theme ? lv_color_white() : lv_color_black(), 0);
        lv_obj_set_style_text_font(rpm_debug, &lv_font_montserrat_12, 0);
        lv_obj_align(rpm_debug, LV_ALIGN_TOP_LEFT, 5, 25);
        lv_label_set_text_fmt(rpm_debug, "RPM: %d/%d", g_vehicle_state.current_rpm, g_vehicle_state.target_rpm);

        lv_obj_t *speed_debug = lv_label_create(g_debug_panel);
        lv_obj_set_style_text_color(speed_debug, is_dark_theme ? lv_color_white() : lv_color_black(), 0);
        lv_obj_set_style_text_font(speed_debug, &lv_font_montserrat_12, 0);
        lv_obj_align(speed_debug, LV_ALIGN_TOP_LEFT, 5, 45);
        lv_label_set_text_fmt(speed_debug, "SPD: %d km/h", g_vehicle_state.current_speed);

        lv_obj_t *gear_debug = lv_label_create(g_debug_panel);
        lv_obj_set_style_text_color(gear_debug, is_dark_theme ? lv_color_white() : lv_color_black(), 0);
        lv_obj_set_style_text_font(gear_debug, &lv_font_montserrat_12, 0);
        lv_obj_align(gear_debug, LV_ALIGN_TOP_LEFT, 5, 65);
        lv_label_set_text_fmt(gear_debug, "GEAR: %d", g_vehicle_state.current_gear);

        lv_obj_t *throttle_debug = lv_label_create(g_debug_panel);
        lv_obj_set_style_text_color(throttle_debug, is_dark_theme ? lv_color_white() : lv_color_black(), 0);
        lv_obj_set_style_text_font(throttle_debug, &lv_font_montserrat_12, 0);
        lv_obj_align(throttle_debug, LV_ALIGN_TOP_LEFT, 5, 85);
        lv_label_set_text_fmt(throttle_debug, "THR: %d%%", get_throttle_input());

        lv_obj_t *brake_debug = lv_label_create(g_debug_panel);
        lv_obj_set_style_text_color(brake_debug, is_dark_theme ? lv_color_white() : lv_color_black(), 0);
        lv_obj_set_style_text_font(brake_debug, &lv_font_montserrat_12, 0);
        lv_obj_align(brake_debug, LV_ALIGN_TOP_LEFT, 5, 105);
        lv_label_set_text_fmt(brake_debug, "BRK: %d%%", get_brake_input());
    }
    else
    {
        // 銷毀除錯面板
        lv_obj_delete(g_debug_panel);
        g_debug_panel = NULL;
    }
}

void create_main_dashboard(lv_obj_t *parent)
{
    if (parent == NULL)
    {
        fprintf(stderr, "[ERROR] Parent object is NULL in create_main_dashboard\n");
        return;
    }

    // 保存 screen 引用
    g_main_dashboard_screen = parent;

    // 主畫面佈局：使用 grid
    // 定義列：中間儀表區、右側控制區（移除左側空白區）
    static int32_t col_dsc[] = {LV_GRID_FR(1), 120, LV_GRID_TEMPLATE_LAST};
    // 定義行：頂部狀態區、中間儀表區、底部資訊區
    static int32_t row_dsc[] = {60, LV_GRID_FR(1), 60, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *main_cont = lv_obj_create(parent);
    if (main_cont == NULL)
    {
        fprintf(stderr, "[ERROR] Failed to create main container\n");
        return;
    }
    lv_obj_set_size(main_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_grid_dsc_array(main_cont, col_dsc, row_dsc);
    // 移除硬編碼的黑色背景，讓主題系統控制背景
    lv_obj_set_style_bg_opa(main_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main_cont, 0, 0);

    // 頂部狀態區 (0,0) - 橫跨所有列
    lv_obj_t *status_cont = lv_obj_create(main_cont);
    lv_obj_set_grid_cell(status_cont, LV_GRID_ALIGN_STRETCH, 0, 2,
                         LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_style_bg_color(status_cont, get_panel_bg_color(), 0);
    lv_obj_set_style_border_width(status_cont, 1, 0);
    lv_obj_set_style_border_color(status_cont, get_border_color(), 0);
    lv_obj_set_flex_flow(status_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 狀態燈號
    lv_obj_t *abs_label = lv_label_create(status_cont);
    lv_label_set_text(abs_label, LV_SYMBOL_WARNING);
    lv_obj_set_style_text_color(abs_label, lv_color_make(255, 0, 0), 0); // 紅色，表示 ABS 啟動

    lv_obj_t *tcs_label = lv_label_create(status_cont);
    lv_label_set_text(tcs_label, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_color(tcs_label, lv_color_make(0, 255, 0), 0); // 綠色，表示 TCS 正常

    lv_obj_t *high_beam_label = lv_label_create(status_cont);
    lv_label_set_text(high_beam_label, LV_SYMBOL_EYE_OPEN);
    lv_obj_set_style_text_color(high_beam_label, lv_color_make(255, 255, 0), 0); // 黃色，表示遠燈

    // 設定按鈕
    lv_obj_t *settings_btn = lv_button_create(status_cont);
    lv_obj_set_size(settings_btn, 40, 40);

    lv_obj_t *settings_label = lv_label_create(settings_btn);
    lv_label_set_text(settings_label, LV_SYMBOL_SETTINGS);
    lv_obj_center(settings_label);

    lv_obj_add_event_cb(settings_btn, settings_event_cb, LV_EVENT_CLICKED, NULL);

    // 除錯切換按鈕
    lv_obj_t *debug_btn = lv_button_create(status_cont);
    lv_obj_set_size(debug_btn, 40, 40);
    lv_obj_set_style_bg_color(debug_btn, lv_color_make(100, 100, 0), 0); // 黃色按鈕

    lv_obj_t *debug_label = lv_label_create(debug_btn);
    lv_label_set_text(debug_label, LV_SYMBOL_EYE_OPEN);
    lv_obj_center(debug_label);

    lv_obj_add_event_cb(debug_btn, debug_toggle_event_cb, LV_EVENT_CLICKED, NULL);

    // 主題切換按鈕
    lv_obj_t *theme_btn = lv_button_create(status_cont);
    lv_obj_set_size(theme_btn, 40, 40);
    lv_obj_set_style_bg_color(theme_btn, lv_color_make(50, 50, 150), 0); // 藍色按鈕

    lv_obj_t *theme_label = lv_label_create(theme_btn);
    lv_label_set_text(theme_label, is_dark_theme ? LV_SYMBOL_EYE_CLOSE : LV_SYMBOL_EYE_OPEN);
    lv_obj_center(theme_label);

    lv_obj_add_event_cb(theme_btn, theme_toggle_event_cb, LV_EVENT_CLICKED, NULL);

    // 中間儀表區 (0,1) - 使用新的布局系統
    lv_obj_t *meter_cont = lv_obj_create(main_cont);
    lv_obj_set_grid_cell(meter_cont, LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_style_bg_color(meter_cont, get_darker_panel_bg_color(), 0);
    lv_obj_set_style_border_width(meter_cont, 0, 0);

    // 創建儀表板布局（根據當前模式）
    create_dashboard_layout(meter_cont, g_vehicle_state.dashboard_layout);

    // 底部資訊區 (0,2) - 橫跨所有列
    lv_obj_t *info_cont = lv_obj_create(main_cont);
    lv_obj_set_grid_cell(info_cont, LV_GRID_ALIGN_STRETCH, 0, 2,
                         LV_GRID_ALIGN_STRETCH, 2, 1);
    lv_obj_set_style_bg_color(info_cont, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_width(info_cont, 1, 0);
    lv_obj_set_style_border_color(info_cont, lv_color_make(100, 100, 100), 0);
    lv_obj_set_flex_flow(info_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(info_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 底部資訊標籤
    lv_obj_t *fuel_label = lv_label_create(info_cont);
    lv_label_set_text(fuel_label, "FUEL: 85%");
    lv_obj_set_style_text_color(fuel_label, lv_color_white(), 0);

    lv_obj_t *temp_label = lv_label_create(info_cont);
    lv_label_set_text(temp_label, "TEMP: 90°C");
    lv_obj_set_style_text_color(temp_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(temp_label, get_chinese_font(), 0);

    lv_obj_t *odo_label = lv_label_create(info_cont);
    lv_label_set_text(odo_label, "ODO: 12345");
    lv_obj_set_style_text_color(odo_label, lv_color_white(), 0);

    // 設定值顯示
    g_mode_label = lv_label_create(info_cont);
    const char *mode_names[] = {"Street", "Sport", "Race", "Rain", "Offroad", "自訂"};
    lv_label_set_text_fmt(g_mode_label, "模式: %s", mode_names[g_vehicle_state.riding_mode]);
    lv_obj_set_style_text_color(g_mode_label, lv_color_make(255, 255, 0), 0); // 黃色
    lv_obj_set_style_text_font(g_mode_label, get_chinese_font(), 0);

    g_abs_label = lv_label_create(info_cont);
    const char *abs_names[] = {"全開", "前後輪全關", "後輪關閉", "Offroad"};
    int abs_idx = 0;
    if (g_vehicle_state.abs_level <= 2)
        abs_idx = 0;
    else if (g_vehicle_state.abs_level >= 7)
        abs_idx = 1;
    else if (g_vehicle_state.abs_level >= 5)
        abs_idx = 2;
    else
        abs_idx = 3;
    lv_label_set_text_fmt(g_abs_label, "ABS: %s", abs_names[abs_idx]);
    lv_obj_set_style_text_color(g_abs_label, lv_color_make(0, 255, 0), 0); // 綠色
    lv_obj_set_style_text_font(g_abs_label, get_chinese_font(), 0);

    g_tcs_label = lv_label_create(info_cont);
    lv_label_set_text_fmt(g_tcs_label, "循跡: %d段", g_vehicle_state.tcs_level);
    lv_obj_set_style_text_color(g_tcs_label, lv_color_make(0, 255, 255), 0); // 青色
    lv_obj_set_style_text_font(g_tcs_label, get_chinese_font(), 0);

    // 右側控制面板 (1,1) - 修正列位置
    lv_obj_t *control_cont = lv_obj_create(main_cont);
    lv_obj_set_grid_cell(control_cont, LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_style_bg_color(control_cont, get_control_panel_bg_color(), 0);
    lv_obj_set_style_border_width(control_cont, 1, 0);
    lv_obj_set_style_border_color(control_cont, get_border_color(), 0);
    lv_obj_set_flex_flow(control_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(control_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(control_cont, 10, 0);

    // 控制面板標題
    lv_obj_t *control_title = lv_label_create(control_cont);
    lv_label_set_text(control_title, "VIRTUAL CONTROLS");
    lv_obj_set_style_text_color(control_title, get_text_color(), 0);
    lv_obj_set_style_text_font(control_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_margin_bottom(control_title, 15, 0);

    // 油門控制
    lv_obj_t *throttle_cont = lv_obj_create(control_cont);
    lv_obj_set_size(throttle_cont, lv_pct(100), 60);
    lv_obj_set_style_bg_color(throttle_cont, get_panel_bg_color(), 0);
    lv_obj_set_style_border_width(throttle_cont, 1, 0);
    lv_obj_set_style_border_color(throttle_cont, lv_color_make(255, 100, 100), 0); // 保持紅色邊框表示油門
    lv_obj_set_style_pad_all(throttle_cont, 5, 0);

    lv_obj_t *throttle_label = lv_label_create(throttle_cont);
    lv_label_set_text(throttle_label, "THROTTLE");
    lv_obj_set_style_text_color(throttle_label, get_text_color(), 0);
    lv_obj_set_style_text_font(throttle_label, &lv_font_montserrat_12, 0);
    lv_obj_align(throttle_label, LV_ALIGN_TOP_MID, 0, 0);

    g_throttle_slider = lv_slider_create(throttle_cont);
    lv_obj_set_size(g_throttle_slider, lv_pct(90), 25);
    lv_obj_align(g_throttle_slider, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_slider_set_range(g_throttle_slider, 0, 100);
    lv_slider_set_value(g_throttle_slider, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(g_throttle_slider, lv_color_make(100, 100, 100), LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_throttle_slider, lv_color_make(255, 100, 100), LV_PART_INDICATOR);
    lv_obj_add_event_cb(g_throttle_slider, throttle_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 剎車控制
    lv_obj_t *brake_cont = lv_obj_create(control_cont);
    lv_obj_set_size(brake_cont, lv_pct(100), 60);
    lv_obj_set_style_bg_color(brake_cont, get_panel_bg_color(), 0);
    lv_obj_set_style_border_width(brake_cont, 1, 0);
    lv_obj_set_style_border_color(brake_cont, lv_color_make(100, 100, 255), 0); // 保持藍色邊框表示剎車
    lv_obj_set_style_pad_all(brake_cont, 5, 0);
    lv_obj_set_style_margin_top(brake_cont, 10, 0);

    lv_obj_t *brake_label = lv_label_create(brake_cont);
    lv_label_set_text(brake_label, "BRAKE");
    lv_obj_set_style_text_color(brake_label, get_text_color(), 0);
    lv_obj_set_style_text_font(brake_label, &lv_font_montserrat_12, 0);
    lv_obj_align(brake_label, LV_ALIGN_TOP_MID, 0, 0);

    g_brake_slider = lv_slider_create(brake_cont);
    lv_obj_set_size(g_brake_slider, lv_pct(90), 25);
    lv_obj_align(g_brake_slider, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_slider_set_range(g_brake_slider, 0, 100);
    lv_slider_set_value(g_brake_slider, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(g_brake_slider, lv_color_make(100, 100, 100), LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_brake_slider, lv_color_make(100, 100, 255), LV_PART_INDICATOR);
    lv_obj_add_event_cb(g_brake_slider, brake_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 換檔控制
    lv_obj_t *gear_cont = lv_obj_create(control_cont);
    lv_obj_set_size(gear_cont, lv_pct(100), 80);
    lv_obj_set_style_bg_color(gear_cont, get_panel_bg_color(), 0);
    lv_obj_set_style_border_width(gear_cont, 1, 0);
    lv_obj_set_style_border_color(gear_cont, lv_color_make(100, 255, 100), 0); // 保持綠色邊框表示換檔
    lv_obj_set_style_pad_all(gear_cont, 5, 0);
    lv_obj_set_style_margin_top(gear_cont, 10, 0);
    lv_obj_set_flex_flow(gear_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(gear_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *gear_label = lv_label_create(gear_cont);
    lv_label_set_text(gear_label, "GEAR SHIFT");
    lv_obj_set_style_text_color(gear_label, get_text_color(), 0);
    lv_obj_set_style_text_font(gear_label, &lv_font_montserrat_12, 0);

    lv_obj_t *gear_btn_cont = lv_obj_create(gear_cont);
    lv_obj_set_size(gear_btn_cont, lv_pct(100), 40);
    lv_obj_set_flex_flow(gear_btn_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(gear_btn_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(gear_btn_cont, LV_OPA_TRANSP, 0);

    // 降檔按鈕
    lv_obj_t *gear_down_btn = lv_button_create(gear_btn_cont);
    lv_obj_set_size(gear_down_btn, 35, 35);
    lv_obj_set_style_bg_color(gear_down_btn, lv_color_make(255, 100, 100), 0);
    lv_obj_t *gear_down_label = lv_label_create(gear_down_btn);
    lv_label_set_text(gear_down_label, LV_SYMBOL_DOWN);
    lv_obj_center(gear_down_label);
    lv_obj_add_event_cb(gear_down_btn, gear_down_event_cb, LV_EVENT_CLICKED, NULL);

    // 升檔按鈕
    lv_obj_t *gear_up_btn = lv_button_create(gear_btn_cont);
    lv_obj_set_size(gear_up_btn, 35, 35);
    lv_obj_set_style_bg_color(gear_up_btn, lv_color_make(100, 255, 100), 0);
    lv_obj_t *gear_up_label = lv_label_create(gear_up_btn);
    lv_label_set_text(gear_up_label, LV_SYMBOL_UP);
    lv_obj_center(gear_up_label);
    lv_obj_add_event_cb(gear_up_btn, gear_up_event_cb, LV_EVENT_CLICKED, NULL);

    // 建立 UI 刷新 timer (50ms - 調整為較慢以提升穩定性)
    ui_refresh_timer = lv_timer_create(ui_refresh_cb, 50, NULL);

    // 應用初始主題
    apply_theme();
}

// 事件回呼函式
static void settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        create_settings_screen();
    }
}

static void theme_toggle_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        is_dark_theme = !is_dark_theme;
        apply_theme();
    }
}

// 返回主儀表板
void return_to_main_dashboard(void)
{
    if (g_main_dashboard_screen)
    {
        // 重新應用主題以確保正確顯示
        apply_theme();
        // 載入主儀表板 screen
        lv_scr_load_anim(g_main_dashboard_screen, LV_SCR_LOAD_ANIM_FADE_OUT, 300, 0, false);
    }
    else
    {
        fprintf(stderr, "[ERROR] Main dashboard screen not initialized\n");
    }
}
