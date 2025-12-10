#include "ui_settings_menu.h"
#include "ui_state_manager.h"
#include "ui_main_dashboard.h"
#include "../lvgl/lvgl.h"
#include <stdio.h>

// 全域物件
static lv_obj_t *settings_screen;
static lv_obj_t *main_list;
static lv_obj_t *brightness_label;   // 亮度百分比標籤
static lv_obj_t *tcs_level_label;    // TCS段數標籤
static lv_obj_t *abs_dropdown;       // ABS下拉選單
static lv_obj_t *font_size_label;    // 字體大小標籤
static lv_obj_t *tcs_slider;         // TCS滑動條
static lv_obj_t *riding_mode_roller; // 騎乘模式選擇器
static int current_page_level;       // 當前頁面層級: 0=主選單, 1=子頁面

// 從主儀表引用主題狀態
extern bool is_dark_theme;

// 函式前向聲明
static void create_riding_mode_page(void);
static void create_display_settings_page(void);
static void create_vehicle_info_page(void);
static void back_event_cb(lv_event_t *e);
static void show_mode_change_warning(void);
static void abs_dropdown_event_cb(lv_event_t *e);
static void show_mode_change_warning_with_data(lv_obj_t *control, int new_value, int setting_type);
static void font_size_event_cb(lv_event_t *e);

// ABS 選項映射函式
static uint16_t calculate_abs_dropdown_selection(uint16_t abs_level);
static uint16_t calculate_abs_level_from_selection(uint16_t selection);

// 設定名稱函式
static const char *get_setting_name(int setting_type);

// UI 更新輔助函式
static void update_tcs_ui(int tcs_value);
static void sync_mode_ui_controls(void);
static void msgbox_cancel_cb(lv_event_t *e);
static void msgbox_ok_cb(lv_event_t *e);
static void apply_theme_to_container(lv_obj_t *cont); // 應用主題到容器

// 模式變更對話框數據結構
typedef struct
{
    lv_obj_t *dialog;
    lv_obj_t *control;
    int new_value;
    int setting_type; // 0=TCS, 1=亮度, 2=ABS
} ModeChangeData_t;

// 事件回呼函式
static void list_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target_obj(e);

    if (code == LV_EVENT_CLICKED)
    {
        // 使用 user_data 來識別按鈕
        uint32_t *id = lv_obj_get_user_data(obj);
        if (id)
        {
            switch (*id)
            {
            case 0: // 騎乘模式
                create_riding_mode_page();
                break;
            case 1: // 顯示設定
                create_display_settings_page();
                break;
            case 2: // 車輛資訊
                create_vehicle_info_page();
                break;
            }
        }
    }
}

static void riding_mode_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *roller = lv_event_get_target_obj(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t sel = lv_roller_get_selected(roller);
        g_vehicle_state.riding_mode = sel;

        // 根據模式調整 TCS/ABS (非自定義模式)
        if (sel != 5)
        {
            g_vehicle_state.tcs_level = riding_mode_defaults[sel].tcs_level;
            g_vehicle_state.abs_level = riding_mode_defaults[sel].abs_level;
        }
        // 自定義模式保持當前值

        // 更新所有 UI 控件
        sync_mode_ui_controls();
    }
}

static void slider_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *slider = lv_event_get_target_obj(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        int value = lv_slider_get_value(slider);

        // 自定義模式：直接更新
        if (g_vehicle_state.riding_mode == 5)
        {
            g_vehicle_state.tcs_level = value;
            update_tcs_ui(value);
            return;
        }

        // 非自定義模式：暫時顯示但不保存
        if (tcs_level_label)
        {
            lv_label_set_text_fmt(tcs_level_label, "段數: %d", value);
        }
    }
    else if (code == LV_EVENT_RELEASED)
    {
        int value = lv_slider_get_value(slider);

        // 手放開後確認是否切換模式
        if (g_vehicle_state.riding_mode != 5)
        {
            show_mode_change_warning_with_data(slider, value, 0);
        }
    }
}

static void arc_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *arc = lv_event_get_target_obj(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        int new_brightness = lv_arc_get_value(arc);

        g_vehicle_state.brightness = new_brightness;
        // 更新亮度百分比顯示
        if (brightness_label)
        {
            lv_label_set_text_fmt(brightness_label, "%d%%", g_vehicle_state.brightness);
        }
        // 這裡可以添加亮度調整邏輯
    }
}

void create_settings_screen(void)
{
    current_page_level = 0; // 初始化為主選單層級

    // 建立設定 screen
    settings_screen = lv_obj_create(NULL);
    // 根據主題設置背景色
    extern bool is_dark_theme;
    lv_obj_set_style_bg_color(settings_screen, is_dark_theme ? lv_color_black() : lv_color_white(), 0);

    // 主選單列表
    main_list = lv_list_create(settings_screen);
    lv_obj_set_size(main_list, lv_pct(80), lv_pct(60));
    lv_obj_center(main_list);

    lv_obj_t *btn;

    // 騎乘模式按鈕
    static uint32_t btn_id_0 = 0;
    btn = lv_list_add_button(main_list, NULL, NULL);
    lv_obj_set_user_data(btn, &btn_id_0);
    lv_obj_add_event_cb(btn, list_event_cb, LV_EVENT_CLICKED, NULL);

    // 創建符號標籤
    lv_obj_t *symbol_0 = lv_label_create(btn);
    lv_label_set_text(symbol_0, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_font(symbol_0, &lv_font_montserrat_16, 0);

    // 創建文字標籤
    lv_obj_t *text_0 = lv_label_create(btn);
    lv_label_set_text(text_0, "騎乘模式");
    lv_obj_set_style_text_font(text_0, get_chinese_font(), 0);
    lv_obj_set_flex_grow(text_0, 1);

    // 顯示設定按鈕
    static uint32_t btn_id_1 = 1;
    btn = lv_list_add_button(main_list, NULL, NULL);
    lv_obj_set_user_data(btn, &btn_id_1);
    lv_obj_add_event_cb(btn, list_event_cb, LV_EVENT_CLICKED, NULL);

    // 創建符號標籤
    lv_obj_t *symbol_1 = lv_label_create(btn);
    lv_label_set_text(symbol_1, LV_SYMBOL_EYE_OPEN);
    lv_obj_set_style_text_font(symbol_1, &lv_font_montserrat_16, 0);

    // 創建文字標籤
    lv_obj_t *text_1 = lv_label_create(btn);
    lv_label_set_text(text_1, "顯示設定");
    lv_obj_set_style_text_font(text_1, get_chinese_font(), 0);
    lv_obj_set_flex_grow(text_1, 1);

    // 車輛資訊按鈕
    static uint32_t btn_id_2 = 2;
    btn = lv_list_add_button(main_list, NULL, NULL);
    lv_obj_set_user_data(btn, &btn_id_2);
    lv_obj_add_event_cb(btn, list_event_cb, LV_EVENT_CLICKED, NULL);

    // 創建符號標籤
    lv_obj_t *symbol_2 = lv_label_create(btn);
    lv_label_set_text(symbol_2, LV_SYMBOL_WARNING);
    lv_obj_set_style_text_font(symbol_2, &lv_font_montserrat_16, 0);

    // 創建文字標籤
    lv_obj_t *text_2 = lv_label_create(btn);
    lv_label_set_text(text_2, "車輛資訊");
    lv_obj_set_style_text_font(text_2, get_chinese_font(), 0);
    lv_obj_set_flex_grow(text_2, 1);

    // 返回按鈕
    lv_obj_t *back_btn = lv_button_create(settings_screen);
    lv_obj_set_size(back_btn, 100, 40);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);

    // 創建符號標籤
    lv_obj_t *back_symbol = lv_label_create(back_btn);
    lv_label_set_text(back_symbol, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(back_symbol, &lv_font_montserrat_16, 0);
    lv_obj_align(back_symbol, LV_ALIGN_LEFT_MID, 5, 0);

    // 創建文字標籤
    lv_obj_t *back_text = lv_label_create(back_btn);
    lv_label_set_text(back_text, "返回");
    lv_obj_set_style_text_font(back_text, get_chinese_font(), 0);
    lv_obj_align(back_text, LV_ALIGN_CENTER, 10, 0);

    lv_obj_add_event_cb(back_btn, back_event_cb, LV_EVENT_CLICKED, NULL);

    // 應用主題
    apply_settings_menu_theme();

    // 載入設定 screen
    lv_scr_load_anim(settings_screen, LV_SCR_LOAD_ANIM_FADE_IN, 300, 0, false);
}

static void create_riding_mode_page(void)
{
    current_page_level = 1; // 進入子頁面

    // 清除主列表
    lv_obj_clean(main_list);

    lv_obj_t *cont = lv_obj_create(main_list);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(cont, 20, 0);

    // 禁用拖動
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_CLICKABLE);

    // 騎乘模式選擇
    lv_obj_t *mode_label = lv_label_create(cont);
    lv_label_set_text(mode_label, "選擇騎乘模式");
    lv_obj_set_style_text_font(mode_label, get_chinese_font(), 0);

    riding_mode_roller = lv_roller_create(cont);
    lv_roller_set_options(riding_mode_roller, "race\nrain\nnormal\noffroad\nsupermoto\n自定義", LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(riding_mode_roller, g_vehicle_state.riding_mode, LV_ANIM_ON);
    lv_obj_add_event_cb(riding_mode_roller, riding_mode_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // TCS 滑動條
    lv_obj_t *tcs_label = lv_label_create(cont);
    lv_label_set_text(tcs_label, "循跡控制 (TCS)");
    lv_obj_set_style_text_font(tcs_label, get_chinese_font(), 0);

    tcs_slider = lv_slider_create(cont);
    lv_obj_set_width(tcs_slider, lv_pct(80));
    lv_slider_set_range(tcs_slider, 0, 15); // 包含0-15段，讓supermoto的TCS=0能正確顯示
    lv_slider_set_value(tcs_slider, g_vehicle_state.tcs_level, LV_ANIM_OFF);
    static int tcs_type = 0;
    lv_obj_set_user_data(tcs_slider, &tcs_type);
    lv_obj_add_event_cb(tcs_slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(tcs_slider, slider_event_cb, LV_EVENT_RELEASED, NULL);

    // TCS段數顯示標籤
    tcs_level_label = lv_label_create(cont);
    lv_label_set_text_fmt(tcs_level_label, "段數: %d", g_vehicle_state.tcs_level);
    lv_obj_set_style_text_font(tcs_level_label, get_chinese_font(), 0);
    lv_obj_align_to(tcs_level_label, tcs_slider, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // ABS 下拉選單
    lv_obj_t *abs_label = lv_label_create(cont);
    lv_label_set_text(abs_label, "防鎖死煞車 (ABS)");
    lv_obj_set_style_text_font(abs_label, get_chinese_font(), 0);

    abs_dropdown = lv_dropdown_create(cont);
    // 選項順序對應 ABS level: 0=Offroad, 1=全開, 2=後輪關閉, 3=前後輪關閉
    lv_dropdown_set_options(abs_dropdown, "Offroad\n全開\n後輪關閉\n前後輪關閉");
    lv_obj_set_width(abs_dropdown, lv_pct(80));

    // 直接使用當前 ABS level (0-3 直接映射)
    lv_dropdown_set_selected(abs_dropdown, g_vehicle_state.abs_level);
    lv_obj_add_event_cb(abs_dropdown, abs_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 應用主題
    apply_theme_to_container(cont);
}

static void create_display_settings_page(void)
{
    current_page_level = 1; // 進入子頁面

    // 清除主列表
    lv_obj_clean(main_list);

    lv_obj_t *cont = lv_obj_create(main_list);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(cont, 20, 0);

    // 禁用拖動
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_CLICKABLE);

    // 字體大小調整
    lv_obj_t *font_label = lv_label_create(cont);
    lv_label_set_text(font_label, "字體大小");
    lv_obj_set_style_text_font(font_label, get_chinese_font(), 0);

    lv_obj_t *font_slider = lv_slider_create(cont);
    lv_obj_set_width(font_slider, lv_pct(80));
    lv_slider_set_range(font_slider, 12, 24);
    lv_slider_set_value(font_slider, g_vehicle_state.font_size, LV_ANIM_OFF);
    lv_obj_add_event_cb(font_slider, font_size_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 字體大小顯示標籤
    font_size_label = lv_label_create(cont);
    lv_label_set_text_fmt(font_size_label, "%dpx", g_vehicle_state.font_size);
    lv_obj_set_style_text_font(font_size_label, get_chinese_font(), 0);
    lv_obj_align_to(font_size_label, font_slider, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 亮度調整
    lv_obj_t *bright_label = lv_label_create(cont);
    lv_label_set_text(bright_label, "螢幕亮度 / Brightness");
    lv_obj_set_style_text_font(bright_label, get_chinese_font(), 0);

    lv_obj_t *bright_arc = lv_arc_create(cont);
    lv_arc_set_range(bright_arc, 0, 100);
    lv_arc_set_value(bright_arc, g_vehicle_state.brightness);
    lv_obj_set_size(bright_arc, 100, 100);
    lv_obj_add_event_cb(bright_arc, arc_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 亮度百分比標籤
    brightness_label = lv_label_create(cont);
    lv_label_set_text_fmt(brightness_label, "%d%%", g_vehicle_state.brightness);
    lv_obj_set_style_text_font(brightness_label, get_chinese_font(), 0);
    lv_obj_set_style_text_align(brightness_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(brightness_label, bright_arc, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // 應用主題
    apply_theme_to_container(cont);
}

static void create_vehicle_info_page(void)
{
    current_page_level = 1; // 進入子頁面

    // 清除主列表
    lv_obj_clean(main_list);

    lv_obj_t *cont = lv_obj_create(main_list);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(cont, 20, 0);

    // 禁用拖動
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *info_label = lv_label_create(cont);
    const char *abs_names[] = {"全開", "前後輪全關", "後輪關閉", "Offroad"};
    int abs_idx = 0;
    if (g_vehicle_state.abs_level <= 2)
        abs_idx = 0; // 全開
    else if (g_vehicle_state.abs_level >= 7)
        abs_idx = 1; // 前後輪全關
    else if (g_vehicle_state.abs_level >= 5)
        abs_idx = 2; // 後輪關閉
    else
        abs_idx = 3; // Offroad

    lv_label_set_text_fmt(info_label, "目前狀態:\nRPM: %d\n時速: %d km/h\n檔位: %d\n模式: %s\nTCS: %d段\nABS: %s",
                          g_vehicle_state.current_rpm, g_vehicle_state.current_speed, g_vehicle_state.current_gear,
                          g_vehicle_state.riding_mode == 0   ? "race"
                          : g_vehicle_state.riding_mode == 1 ? "rain"
                          : g_vehicle_state.riding_mode == 2 ? "normal"
                          : g_vehicle_state.riding_mode == 3 ? "offroad"
                          : g_vehicle_state.riding_mode == 4 ? "supermoto"
                                                             : "自定義",
                          g_vehicle_state.tcs_level, abs_names[abs_idx]);
    lv_obj_set_style_text_font(info_label, get_chinese_font(), 0);

    // 應用主題
    apply_theme_to_container(cont);
}

static void abs_dropdown_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *dropdown = lv_event_get_target_obj(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t sel = lv_dropdown_get_selected(dropdown);

        // 自定義模式：直接更新 (直接映射 0-3)
        if (g_vehicle_state.riding_mode == 5)
        {
            g_vehicle_state.abs_level = sel;
        }
        else
        {
            // 非自定義模式：彈出警告
            show_mode_change_warning_with_data(dropdown, sel, 2); // 2表示ABS
        }
    }
}

static void back_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (current_page_level == 1)
        {
            // 在子頁面，返回主選單
            current_page_level = 0;
            create_settings_screen();
        }
        else
        {
            // 在主選單，返回主儀表
            return_to_main_dashboard();
        }
    }
}

static void show_mode_change_warning(void)
{
    // 創建一個居中的dialog panel，避免全屏覆蓋
    lv_obj_t *dialog = lv_obj_create(lv_screen_active());
    lv_obj_set_size(dialog, 400, 200);
    lv_obj_align(dialog, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(dialog, lv_color_make(50, 50, 50), 0);
    lv_obj_set_style_border_color(dialog, lv_color_white(), 0);
    lv_obj_set_style_border_width(dialog, 2, 0);
    lv_obj_set_style_radius(dialog, 10, 0);

    // 添加標題
    lv_obj_t *title_label = lv_label_create(dialog);
    lv_label_set_text(title_label, "模式保護");
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_font(title_label, get_chinese_font(), 0);
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);

    // 添加文本
    lv_obj_t *text_label = lv_label_create(dialog);
    lv_label_set_text(text_label, "當前模式有預設值，調整將切換為自定義模式。是否繼續？");
    lv_obj_align(text_label, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_text_font(text_label, get_chinese_font(), 0);
    lv_obj_set_style_text_color(text_label, lv_color_white(), 0);
    lv_label_set_long_mode(text_label, LV_LABEL_LONG_WRAP);

    // 創建按鈕容器
    lv_obj_t *btn_cont = lv_obj_create(dialog);
    lv_obj_set_size(btn_cont, 380, 50);
    lv_obj_align(btn_cont, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_opa(btn_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(btn_cont, LV_OPA_TRANSP, 0);

    // 添加按鈕
    lv_obj_t *btn_no = lv_button_create(btn_cont);
    lv_obj_set_size(btn_no, 120, 40);
    lv_obj_align(btn_no, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_t *btn_no_label = lv_label_create(btn_no);
    lv_label_set_text(btn_no_label, "不同意");
    lv_obj_center(btn_no_label);
    lv_obj_set_style_text_font(btn_no_label, get_chinese_font(), 0);

    lv_obj_t *btn_yes = lv_button_create(btn_cont);
    lv_obj_set_size(btn_yes, 120, 40);
    lv_obj_align(btn_yes, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_obj_t *btn_yes_label = lv_label_create(btn_yes);
    lv_label_set_text(btn_yes_label, "同意切換");
    lv_obj_center(btn_yes_label);
    lv_obj_set_style_text_font(btn_yes_label, get_chinese_font(), 0);

    // 註: show_mode_change_warning 是舊版函式，暫時保留但不建議使用
    // 請使用 show_mode_change_warning_with_data 代替
}

static void show_mode_change_warning_with_data(lv_obj_t *control, int new_value, int setting_type)
{
    // 分配數據結構
    ModeChangeData_t *data = lv_malloc(sizeof(ModeChangeData_t));
    if (!data)
        return;

    // 創建模態對話框
    lv_obj_t *msgbox = lv_msgbox_create(NULL);
    lv_msgbox_add_title(msgbox, "模式保護");

    const char *setting_name = get_setting_name(setting_type);
    char message[256];
    snprintf(message, sizeof(message),
             "當前模式有預設值，調整%s將切換為自定義模式。\n"
             "新值: %d\n是否繼續？",
             setting_name, new_value);
    lv_msgbox_add_text(msgbox, message);

    lv_obj_t *btn_no = lv_msgbox_add_footer_button(msgbox, "不同意");
    lv_obj_t *btn_yes = lv_msgbox_add_footer_button(msgbox, "同意切換");
    lv_obj_set_style_text_font(msgbox, get_chinese_font(), 0);

    // 禁用背景控件以確保模態行為
    lv_obj_add_state(settings_screen, LV_STATE_DISABLED);

    // 填充數据結構
    data->dialog = msgbox;
    data->control = control;
    data->new_value = new_value;
    data->setting_type = setting_type;

    // 設置按鈕事件回調 - 使用新的獨立回呼
    lv_obj_add_event_cb(btn_no, msgbox_cancel_cb, LV_EVENT_CLICKED, data);
    lv_obj_add_event_cb(btn_yes, msgbox_ok_cb, LV_EVENT_CLICKED, data);
}

static void font_size_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *slider = lv_event_get_target_obj(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        int value = lv_slider_get_value(slider);
        g_vehicle_state.font_size = value;
        // 更新字體大小顯示
        if (font_size_label)
        {
            lv_label_set_text_fmt(font_size_label, "%dpx", value);
        }
        // 這裡可以添加字體大小調整邏輯
        // 實際應用中可能需要重新載入UI或調整字體
    }
}

// ABS 選項映射函式實現
static uint16_t calculate_abs_dropdown_selection(uint16_t abs_level)
{
    return abs_level; // 直接映射 0-3
}

static uint16_t calculate_abs_level_from_selection(uint16_t selection)
{
    return selection; // 直接映射 0-3
}

static const char *get_setting_name(int setting_type)
{
    switch (setting_type)
    {
    case 0:
        return "循跡控制 (TCS)";
    case 1:
        return "螢幕亮度";
    case 2:
        return "防鎖死煞車 (ABS)";
    default:
        return "設定";
    }
}

// UI 更新輔助函式 - 參考 lv_example_slider 風格
static void update_tcs_ui(int tcs_value)
{
    // 同時更新滑桿和標籤
    if (tcs_slider)
    {
        lv_slider_set_value(tcs_slider, tcs_value, LV_ANIM_ON);
    }
    if (tcs_level_label)
    {
        lv_label_set_text_fmt(tcs_level_label, "段數: %d", tcs_value);
    }
}

static void sync_mode_ui_controls(void)
{
    // 更新騎乘模式
    if (riding_mode_roller)
    {
        lv_roller_set_selected(riding_mode_roller, g_vehicle_state.riding_mode, LV_ANIM_ON);
    }

    // 更新 TCS
    update_tcs_ui(g_vehicle_state.tcs_level);

    // 更新 ABS
    if (abs_dropdown)
    {
        lv_dropdown_set_selected(abs_dropdown, g_vehicle_state.abs_level);
    }
}

// 對話框取消回呼
static void msgbox_cancel_cb(lv_event_t *e)
{
    ModeChangeData_t *data = lv_event_get_user_data(e);

    // 恢復原值
    sync_mode_ui_controls();

    // 關閉並清理
    lv_msgbox_close(data->dialog);
    lv_free(data);

    // 重新啟用設定屏幕
    lv_obj_clear_state(settings_screen, LV_STATE_DISABLED);
}

// 對話框確定回呼
static void msgbox_ok_cb(lv_event_t *e)
{
    ModeChangeData_t *data = lv_event_get_user_data(e);

    // 切換到自定義模式
    g_vehicle_state.riding_mode = 5;

    // 應用新值
    switch (data->setting_type)
    {
    case 0:
        g_vehicle_state.tcs_level = data->new_value;
        break;
    case 1:
        g_vehicle_state.brightness = data->new_value;
        break;
    case 2:
        g_vehicle_state.abs_level = data->new_value;
        break;
    }

    // 更新 UI
    sync_mode_ui_controls();

    // 關閉並清理
    lv_msgbox_close(data->dialog);
    lv_free(data);

    // 重新啟用設定屏幕
    lv_obj_clear_state(settings_screen, LV_STATE_DISABLED);
}

// 應用主題到容器及其子元件
static void apply_theme_to_container(lv_obj_t *cont)
{
    if (!cont)
        return;

    lv_color_t bg_color, text_color, widget_bg;

    if (is_dark_theme)
    {
        // 深色主題
        bg_color = lv_color_make(30, 30, 30);
        text_color = lv_color_white();
        widget_bg = lv_color_make(50, 50, 50);
    }
    else
    {
        // 淺色主題
        bg_color = lv_color_make(240, 240, 240);
        text_color = lv_color_black();
        widget_bg = lv_color_make(220, 220, 220);
    }

    // 設定容器背景和文字顏色
    lv_obj_set_style_bg_color(cont, bg_color, 0);
    lv_obj_set_style_text_color(cont, text_color, 0);

    // 遍歷所有子元件
    uint32_t child_cnt = lv_obj_get_child_count(cont);
    for (uint32_t i = 0; i < child_cnt; i++)
    {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!child)
            continue;

        // 設定文字顏色
        lv_obj_set_style_text_color(child, text_color, 0);

        // 根據元件類型設定樣式
        // 滑桿 (slider)
        if (lv_obj_check_type(child, &lv_slider_class))
        {
            lv_obj_set_style_bg_color(child, widget_bg, LV_PART_MAIN);
            lv_obj_set_style_bg_color(child, is_dark_theme ? lv_color_make(0, 120, 215) : lv_color_make(0, 100, 200), LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(child, is_dark_theme ? lv_color_make(100, 100, 100) : lv_color_make(150, 150, 150), LV_PART_KNOB);
        }
        // 下拉選單 (dropdown)
        else if (lv_obj_check_type(child, &lv_dropdown_class))
        {
            lv_obj_set_style_bg_color(child, widget_bg, 0);
            lv_obj_set_style_text_color(child, text_color, 0);
        }
        // 滾輪 (roller)
        else if (lv_obj_check_type(child, &lv_roller_class))
        {
            lv_obj_set_style_bg_color(child, widget_bg, 0);
            lv_obj_set_style_text_color(child, text_color, 0);
            // 選中項：設定背景色和文字色以確保可讀性
            if (is_dark_theme)
            {
                lv_obj_set_style_bg_color(child, lv_color_make(0, 120, 215), LV_PART_SELECTED);
                lv_obj_set_style_text_color(child, lv_color_white(), LV_PART_SELECTED);
            }
            else
            {
                lv_obj_set_style_bg_color(child, lv_color_make(0, 100, 200), LV_PART_SELECTED);
                lv_obj_set_style_text_color(child, lv_color_white(), LV_PART_SELECTED);
            }
        }
        // 圓弧 (arc)
        else if (lv_obj_check_type(child, &lv_arc_class))
        {
            lv_obj_set_style_arc_color(child, widget_bg, LV_PART_MAIN);
            lv_obj_set_style_arc_color(child, is_dark_theme ? lv_color_make(0, 120, 215) : lv_color_make(0, 100, 200), LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(child, is_dark_theme ? lv_color_make(100, 100, 100) : lv_color_make(150, 150, 150), LV_PART_KNOB);
        }
    }
}

// 應用主題到設定選單（公開函數）
void apply_settings_menu_theme(void)
{
    if (!settings_screen)
        return;

    // 設定背景和文字顏色
    if (is_dark_theme)
    {
        // 深色主題
        lv_obj_set_style_bg_color(settings_screen, lv_color_black(), 0);
        lv_obj_set_style_text_color(settings_screen, lv_color_white(), LV_PART_MAIN);

        // 更新 main_list
        if (main_list)
        {
            lv_obj_set_style_bg_color(main_list, lv_color_make(20, 20, 20), 0);
            lv_obj_set_style_text_color(main_list, lv_color_white(), LV_PART_MAIN);

            // 更新 list 中的所有按鈕
            uint32_t child_cnt = lv_obj_get_child_count(main_list);
            for (uint32_t i = 0; i < child_cnt; i++)
            {
                lv_obj_t *btn = lv_obj_get_child(main_list, i);
                if (btn)
                {
                    lv_obj_set_style_bg_color(btn, lv_color_make(40, 40, 40), 0);
                    lv_obj_set_style_bg_color(btn, lv_color_make(60, 60, 60), LV_STATE_PRESSED);
                    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
                }
            }
        }
    }
    else
    {
        // 淺色主題
        lv_obj_set_style_bg_color(settings_screen, lv_color_white(), 0);
        lv_obj_set_style_text_color(settings_screen, lv_color_black(), LV_PART_MAIN);

        // 更新 main_list
        if (main_list)
        {
            lv_obj_set_style_bg_color(main_list, lv_color_make(240, 240, 240), 0);
            lv_obj_set_style_text_color(main_list, lv_color_black(), LV_PART_MAIN);

            // 更新 list 中的所有按鈕
            uint32_t child_cnt = lv_obj_get_child_count(main_list);
            for (uint32_t i = 0; i < child_cnt; i++)
            {
                lv_obj_t *btn = lv_obj_get_child(main_list, i);
                if (btn)
                {
                    lv_obj_set_style_bg_color(btn, lv_color_make(220, 220, 220), 0);
                    lv_obj_set_style_bg_color(btn, lv_color_make(200, 200, 200), LV_STATE_PRESSED);
                    lv_obj_set_style_text_color(btn, lv_color_black(), 0);
                }
            }
        }
    }
}
