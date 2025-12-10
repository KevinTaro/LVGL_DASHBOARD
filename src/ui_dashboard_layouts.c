#include "ui_dashboard_layouts.h"
#include "../lvgl/lvgl.h"
#include "ui_state_manager.h"
#include <stdio.h>

// 全域布局物件
static lv_obj_t *g_layout_container = NULL;

// Normal 布局的物件
static lv_obj_t *g_normal_rpm_meter = NULL;
static lv_obj_t *g_normal_speed_meter = NULL;
static lv_obj_t *g_normal_rpm_needle = NULL;
static lv_obj_t *g_normal_speed_needle = NULL;
static lv_obj_t *g_normal_gear_label = NULL;

// Race 布局的物件（稍後實作）
static lv_obj_t *g_race_rpm_bar = NULL;
static lv_obj_t *g_race_speed_label = NULL;
static lv_obj_t *g_race_gear_label = NULL;

// Offroad 布局的物件（稍後實作）
static lv_obj_t *g_offroad_rpm_bar = NULL;
static lv_obj_t *g_offroad_speed_label = NULL;
static lv_obj_t *g_offroad_gear_label = NULL;

// 當前布局類型
static DashboardLayout_t g_current_layout = DASHBOARD_LAYOUT_NORMAL;

// 輔助函數：獲取主題顏色
static lv_color_t get_theme_bg_color(void)
{
    return is_dark_theme ? lv_color_make(15, 15, 20) : lv_color_make(245, 245, 250);
}

static lv_color_t get_theme_text_color(void)
{
    return is_dark_theme ? lv_color_make(220, 220, 230) : lv_color_make(30, 30, 40);
}

static lv_color_t get_theme_accent_color(void)
{
    return is_dark_theme ? lv_color_make(0, 150, 255) : lv_color_make(0, 100, 200);
}

static lv_color_t get_theme_rpm_color(void)
{
    // 轉速表專用顏色 - 橘紅色系
    return is_dark_theme ? lv_color_make(255, 120, 40) : lv_color_make(255, 80, 0);
}

static lv_color_t get_theme_redline_color(void)
{
    // 紅線區專用顏色
    return is_dark_theme ? lv_color_make(255, 60, 60) : lv_color_make(220, 0, 0);
}

static lv_color_t get_theme_scale_bg(void)
{
    // 儀表背景
    return is_dark_theme ? lv_color_make(25, 25, 35) : lv_color_make(235, 235, 245);
}

// ============= Normal 雙環布局 =============

static void create_normal_layout(lv_obj_t *parent)
{
    printf("[INFO] Creating Normal dual meter layout\n");

    // 創建容器（左右分佈）- 填滿整個父容器
    lv_obj_t *dual_meter_cont = lv_obj_create(parent);
    lv_obj_set_size(dual_meter_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(dual_meter_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(dual_meter_cont, 0, 0);
    lv_obj_set_style_pad_all(dual_meter_cont, 10, 0);
    lv_obj_set_flex_flow(dual_meter_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dual_meter_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 禁用滾動和拖動
    lv_obj_clear_flag(dual_meter_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(dual_meter_cont, LV_OBJ_FLAG_CLICKABLE);

    g_layout_container = dual_meter_cont;

    // ===== 左側：轉速表 (RPM Meter) =====
    lv_obj_t *rpm_cont = lv_obj_create(dual_meter_cont);
    lv_obj_set_size(rpm_cont, 280, 280);
    lv_obj_set_style_bg_color(rpm_cont, get_theme_scale_bg(), 0);
    lv_obj_set_style_bg_opa(rpm_cont, LV_OPA_80, 0);
    lv_obj_set_style_border_width(rpm_cont, 2, 0);
    lv_obj_set_style_border_color(rpm_cont, get_theme_rpm_color(), 0);
    lv_obj_set_style_border_opa(rpm_cont, LV_OPA_60, 0);
    lv_obj_set_style_radius(rpm_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(rpm_cont, 0, 0);

    // 添加陰影效果（僅深色模式）
    if (is_dark_theme)
    {
        lv_obj_set_style_shadow_width(rpm_cont, 20, 0);
        lv_obj_set_style_shadow_color(rpm_cont, get_theme_rpm_color(), 0);
        lv_obj_set_style_shadow_opa(rpm_cont, LV_OPA_30, 0);
    }

    // 禁用滾動和拖動
    lv_obj_clear_flag(rpm_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(rpm_cont, LV_OBJ_FLAG_CLICKABLE);

    // 創建轉速表使用 lv_scale
    g_normal_rpm_meter = lv_scale_create(rpm_cont);
    lv_obj_set_size(g_normal_rpm_meter, 260, 260);
    lv_obj_center(g_normal_rpm_meter);
    lv_scale_set_mode(g_normal_rpm_meter, LV_SCALE_MODE_ROUND_OUTER);

    // 禁用拖動
    lv_obj_clear_flag(g_normal_rpm_meter, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(g_normal_rpm_meter, LV_OBJ_FLAG_CLICKABLE);

    // 設定轉速表刻度 (0-13000 RPM, 每 500 RPM 一個刻度)
    lv_scale_set_label_show(g_normal_rpm_meter, true);
    lv_scale_set_total_tick_count(g_normal_rpm_meter, 27);
    lv_scale_set_major_tick_every(g_normal_rpm_meter, 4);
    lv_scale_set_range(g_normal_rpm_meter, 0, 13000);

    // 自訂標籤
    static const char *tacho_labels[] = {"0", "2k", "4k", "6k", "8k", "10k", "12k", NULL};
    lv_scale_set_text_src(g_normal_rpm_meter, tacho_labels);

    // 樣式設定 - 刻度顏色漸變效果
    lv_obj_set_style_length(g_normal_rpm_meter, 5, LV_PART_ITEMS);
    lv_obj_set_style_length(g_normal_rpm_meter, 10, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(g_normal_rpm_meter, 2, LV_PART_ITEMS);
    lv_obj_set_style_line_color(g_normal_rpm_meter, get_theme_rpm_color(), LV_PART_ITEMS);
    lv_obj_set_style_text_color(g_normal_rpm_meter, get_theme_text_color(), LV_PART_MAIN);
    lv_obj_set_style_text_font(g_normal_rpm_meter, &lv_font_montserrat_12, LV_PART_MAIN);

    // 創建轉速表指針（使用 line）
    g_normal_rpm_needle = lv_line_create(g_normal_rpm_meter);
    lv_obj_set_style_line_width(g_normal_rpm_needle, 5, 0);
    lv_obj_set_style_line_color(g_normal_rpm_needle, get_theme_rpm_color(), 0);
    lv_obj_set_style_line_rounded(g_normal_rpm_needle, true, 0);

    // 指針發光效果（深色模式）
    if (is_dark_theme)
    {
        lv_obj_set_style_shadow_width(g_normal_rpm_needle, 10, 0);
        lv_obj_set_style_shadow_color(g_normal_rpm_needle, get_theme_rpm_color(), 0);
        lv_obj_set_style_shadow_opa(g_normal_rpm_needle, LV_OPA_50, 0);
    }

    // 禁用拖動
    lv_obj_clear_flag(g_normal_rpm_needle, LV_OBJ_FLAG_CLICKABLE);

    // 初始化指針位置
    lv_scale_set_line_needle_value(g_normal_rpm_meter, g_normal_rpm_needle, 80, 0);

    // 轉速表中央顯示數字 RPM
    lv_obj_t *rpm_value_label = lv_label_create(g_normal_rpm_meter);
    lv_obj_set_style_text_font(rpm_value_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(rpm_value_label, get_theme_text_color(), 0);
    lv_label_set_text_fmt(rpm_value_label, "%d", g_vehicle_state.target_rpm);
    lv_obj_align(rpm_value_label, LV_ALIGN_CENTER, 0, -10);
    lv_obj_clear_flag(rpm_value_label, LV_OBJ_FLAG_CLICKABLE);
    // 保存標籤以便更新
    lv_obj_set_user_data(g_normal_rpm_meter, rpm_value_label);

    lv_obj_t *rpm_label = lv_label_create(g_normal_rpm_meter);
    lv_obj_set_style_text_font(rpm_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(rpm_label, get_theme_text_color(), 0);
    lv_label_set_text(rpm_label, "RPM");
    lv_obj_align(rpm_label, LV_ALIGN_CENTER, 0, 20);
    lv_obj_clear_flag(rpm_label, LV_OBJ_FLAG_CLICKABLE);

    // ===== 右側：速度表 (Speed Meter) =====
    lv_obj_t *speed_cont = lv_obj_create(dual_meter_cont);
    lv_obj_set_size(speed_cont, 280, 280);
    lv_obj_set_style_bg_color(speed_cont, get_theme_scale_bg(), 0);
    lv_obj_set_style_bg_opa(speed_cont, LV_OPA_80, 0);
    lv_obj_set_style_border_width(speed_cont, 2, 0);
    lv_obj_set_style_border_color(speed_cont, get_theme_accent_color(), 0);
    lv_obj_set_style_border_opa(speed_cont, LV_OPA_60, 0);
    lv_obj_set_style_radius(speed_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(speed_cont, 0, 0);

    // 添加陰影效果（僅深色模式）
    if (is_dark_theme)
    {
        lv_obj_set_style_shadow_width(speed_cont, 20, 0);
        lv_obj_set_style_shadow_color(speed_cont, get_theme_accent_color(), 0);
        lv_obj_set_style_shadow_opa(speed_cont, LV_OPA_30, 0);
    }

    // 禁用滾動和拖動
    lv_obj_clear_flag(speed_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(speed_cont, LV_OBJ_FLAG_CLICKABLE);

    // 創建速度表使用 lv_scale
    g_normal_speed_meter = lv_scale_create(speed_cont);
    lv_obj_set_size(g_normal_speed_meter, 260, 260);
    lv_obj_center(g_normal_speed_meter);
    lv_scale_set_mode(g_normal_speed_meter, LV_SCALE_MODE_ROUND_OUTER);

    // 禁用拖動
    lv_obj_clear_flag(g_normal_speed_meter, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(g_normal_speed_meter, LV_OBJ_FLAG_CLICKABLE);

    // 設定速度表刻度 (0-200 km/h, 每 10 km/h 一個刻度)
    lv_scale_set_label_show(g_normal_speed_meter, true);
    lv_scale_set_total_tick_count(g_normal_speed_meter, 21);
    lv_scale_set_major_tick_every(g_normal_speed_meter, 5);
    lv_scale_set_range(g_normal_speed_meter, 0, 200);

    // 自訂標籤
    static const char *speed_labels[] = {"0", "50", "100", "150", "200", NULL};
    lv_scale_set_text_src(g_normal_speed_meter, speed_labels);

    // 樣式設定
    lv_obj_set_style_length(g_normal_speed_meter, 5, LV_PART_ITEMS);
    lv_obj_set_style_length(g_normal_speed_meter, 10, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(g_normal_speed_meter, 2, LV_PART_ITEMS);
    lv_obj_set_style_line_color(g_normal_speed_meter, get_theme_accent_color(), LV_PART_ITEMS);
    lv_obj_set_style_text_color(g_normal_speed_meter, get_theme_accent_color(), LV_PART_MAIN);
    lv_obj_set_style_text_font(g_normal_speed_meter, &lv_font_montserrat_12, LV_PART_MAIN);

    // 創建速度表指針（使用 line）
    g_normal_speed_needle = lv_line_create(g_normal_speed_meter);
    lv_obj_set_style_line_width(g_normal_speed_needle, 5, 0);
    lv_obj_set_style_line_color(g_normal_speed_needle, get_theme_accent_color(), 0);
    lv_obj_set_style_line_rounded(g_normal_speed_needle, true, 0);

    // 添加指針發光效果（僅深色模式）
    if (is_dark_theme)
    {
        lv_obj_set_style_shadow_width(g_normal_speed_needle, 10, 0);
        lv_obj_set_style_shadow_color(g_normal_speed_needle, get_theme_accent_color(), 0);
        lv_obj_set_style_shadow_opa(g_normal_speed_needle, LV_OPA_50, 0);
    }

    // 禁用拖動
    lv_obj_clear_flag(g_normal_speed_needle, LV_OBJ_FLAG_CLICKABLE);

    // 初始化指針位置
    lv_scale_set_line_needle_value(g_normal_speed_meter, g_normal_speed_needle, 80, 0);

    // 速度表中央顯示檔位和速度
    g_normal_gear_label = lv_label_create(g_normal_speed_meter);
    lv_obj_set_style_text_font(g_normal_gear_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(g_normal_gear_label, get_theme_accent_color(), 0);
    lv_label_set_text_fmt(g_normal_gear_label, "%d", g_vehicle_state.current_gear);
    lv_obj_align(g_normal_gear_label, LV_ALIGN_CENTER, 0, -25);
    lv_obj_clear_flag(g_normal_gear_label, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *speed_value_label = lv_label_create(g_normal_speed_meter);
    lv_obj_set_style_text_font(speed_value_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(speed_value_label, get_theme_text_color(), 0);
    lv_label_set_text_fmt(speed_value_label, "%d", g_vehicle_state.current_speed);
    lv_obj_align(speed_value_label, LV_ALIGN_CENTER, 0, 15);
    lv_obj_clear_flag(speed_value_label, LV_OBJ_FLAG_CLICKABLE);
    // 保存標籤以便更新
    lv_obj_set_user_data(g_normal_speed_meter, speed_value_label);

    lv_obj_t *speed_unit_label = lv_label_create(g_normal_speed_meter);
    lv_obj_set_style_text_font(speed_unit_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(speed_unit_label, get_theme_text_color(), 0);
    lv_label_set_text(speed_unit_label, "km/h");
    lv_obj_align(speed_unit_label, LV_ALIGN_CENTER, 0, 38);
    lv_obj_clear_flag(speed_unit_label, LV_OBJ_FLAG_CLICKABLE);

    printf("[INFO] Normal layout created successfully\n");
}

static void update_normal_layout(void)
{
    if (g_normal_rpm_needle && g_normal_rpm_meter)
    {
        // 更新指針位置
        lv_scale_set_line_needle_value(g_normal_rpm_meter, g_normal_rpm_needle, 80, g_vehicle_state.current_rpm);

        // 更新中央數值標籤
        lv_obj_t *rpm_value_label = (lv_obj_t *)lv_obj_get_user_data(g_normal_rpm_meter);
        if (rpm_value_label)
        {
            lv_label_set_text_fmt(rpm_value_label, "%d", g_vehicle_state.current_rpm);
        }
    }

    if (g_normal_speed_needle && g_normal_speed_meter)
    {
        // 更新指針位置
        lv_scale_set_line_needle_value(g_normal_speed_meter, g_normal_speed_needle, 80, g_vehicle_state.current_speed);

        // 更新中央數值標籤
        lv_obj_t *speed_value_label = (lv_obj_t *)lv_obj_get_user_data(g_normal_speed_meter);
        if (speed_value_label)
        {
            lv_label_set_text_fmt(speed_value_label, "%d", g_vehicle_state.current_speed);
        }
    }

    if (g_normal_gear_label)
    {
        lv_label_set_text_fmt(g_normal_gear_label, "%d", g_vehicle_state.current_gear);
    }
}

static void destroy_normal_layout(void)
{
    g_normal_rpm_meter = NULL;
    g_normal_speed_meter = NULL;
    g_normal_rpm_needle = NULL;
    g_normal_speed_needle = NULL;
    g_normal_gear_label = NULL;
}

// ============= Race 賽道布局 =============
static void create_race_layout(lv_obj_t *parent)
{
    printf("[INFO] Creating Race track layout (placeholder)\n");
    // TODO: 實作賽道布局
    lv_obj_t *placeholder = lv_label_create(parent);
    lv_label_set_text(placeholder, "Race Layout\n(Coming Soon)");
    lv_obj_center(placeholder);
    lv_obj_set_style_text_font(placeholder, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(placeholder, get_theme_text_color(), 0);
    g_layout_container = placeholder;
}

static void update_race_layout(void)
{
    // TODO: 更新賽道布局
}

static void destroy_race_layout(void)
{
    g_race_rpm_bar = NULL;
    g_race_speed_label = NULL;
    g_race_gear_label = NULL;
}

// ============= Offroad 越野布局 =============
static void create_offroad_layout(lv_obj_t *parent)
{
    printf("[INFO] Creating Offroad layout (placeholder)\n");
    // TODO: 實作越野布局
    lv_obj_t *placeholder = lv_label_create(parent);
    lv_label_set_text(placeholder, "Offroad Layout\n(Coming Soon)");
    lv_obj_center(placeholder);
    lv_obj_set_style_text_font(placeholder, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(placeholder, get_theme_text_color(), 0);
    g_layout_container = placeholder;
}

static void update_offroad_layout(void)
{
    // TODO: 更新越野布局
}

static void destroy_offroad_layout(void)
{
    g_offroad_rpm_bar = NULL;
    g_offroad_speed_label = NULL;
    g_offroad_gear_label = NULL;
}

// ============= 公開函數 =============

void create_dashboard_layout(lv_obj_t *parent, DashboardLayout_t layout)
{
    // 先銷毀舊布局
    if (g_layout_container)
    {
        lv_obj_delete(g_layout_container);
        g_layout_container = NULL;
    }

    // 根據類型創建新布局
    g_current_layout = layout;
    switch (layout)
    {
    case DASHBOARD_LAYOUT_NORMAL:
        create_normal_layout(parent);
        break;
    case DASHBOARD_LAYOUT_RACE:
        create_race_layout(parent);
        break;
    case DASHBOARD_LAYOUT_OFFROAD:
        create_offroad_layout(parent);
        break;
    default:
        printf("[ERROR] Unknown dashboard layout type: %d\n", layout);
        break;
    }
}

void update_dashboard_layout(void)
{
    switch (g_current_layout)
    {
    case DASHBOARD_LAYOUT_NORMAL:
        update_normal_layout();
        break;
    case DASHBOARD_LAYOUT_RACE:
        update_race_layout();
        break;
    case DASHBOARD_LAYOUT_OFFROAD:
        update_offroad_layout();
        break;
    default:
        break;
    }
}

void switch_dashboard_layout(DashboardLayout_t new_layout)
{
    if (new_layout == g_current_layout)
    {
        printf("[INFO] Already using layout %d\n", new_layout);
        return;
    }

    printf("[INFO] Switching dashboard layout from %d to %d\n", g_current_layout, new_layout);

    // 銷毀舊布局
    switch (g_current_layout)
    {
    case DASHBOARD_LAYOUT_NORMAL:
        destroy_normal_layout();
        break;
    case DASHBOARD_LAYOUT_RACE:
        destroy_race_layout();
        break;
    case DASHBOARD_LAYOUT_OFFROAD:
        destroy_offroad_layout();
        break;
    default:
        break;
    }

    // 需要外部重新創建布局
    // 這裡只是清理，實際創建由 create_dashboard_layout 完成
}

DashboardLayout_t get_current_dashboard_layout(void)
{
    return g_current_layout;
}
