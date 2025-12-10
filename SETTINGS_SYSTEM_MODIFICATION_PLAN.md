# 摩托車儀表盤設定系統修改計劃

## 文檔概述

本文檔詳細分析了當前摩托車儀表盤設定系統的邏輯缺陷，並提供完整的修改方案。重點解決模式切換連動、彈出視窗邏輯以及控制項同步等問題。

## 當前系統分析

### 現有組件
- **騎乘模式選擇器** (roller)
- **TCS滑動條** (slider)
- **ABS下拉選單** (dropdown)
- **模式保護對話框** (msgbox)

### 現有事件處理
- `riding_mode_event_cb`: 處理模式切換
- `slider_event_cb`: 處理TCS滑動條調整
- `abs_dropdown_event_cb`: 處理ABS下拉選單調整
- `mode_change_warning_cb`: 處理對話框按鈕點擊

## 問題分析與修改方案

### 1. 模式切換連動邏輯檢查

#### 當前實現
```c
static void riding_mode_event_cb(lv_event_t *e)
{
    // 設置新的騎乘模式
    g_vehicle_state.riding_mode = sel;

    // 根據模式設置預設值
    switch (sel) {
        case 0: // race
            g_vehicle_state.tcs_level = 1;
            g_vehicle_state.abs_level = 8;
            break;
        // ... 其他模式
    }

    // 更新UI控件
    if (tcs_level_label) {
        lv_label_set_text_fmt(tcs_level_label, "段數: %d", g_vehicle_state.tcs_level);
    }
    if (tcs_slider) {
        lv_slider_set_value(tcs_slider, g_vehicle_state.tcs_level, LV_ANIM_ON);
    }
    if (abs_dropdown) {
        // 計算對應的下拉選項
        lv_dropdown_set_selected(abs_dropdown, abs_selected);
    }
}
```

#### 問題點
1. **TCS與ABS未正確同步**: 模式切換時，TCS與ABS的值未正確更新到對應的控件。
2. **亮度、字體大小、語言控件不需要同步**: 這些控件應保持獨立，不受模式切換影響。

#### 修改方案
```c
static void riding_mode_event_cb(lv_event_t *e)
{
    uint16_t sel = lv_roller_get_selected(roller);
    g_vehicle_state.riding_mode = sel;

    // 根據模式設置TCS與ABS參數
    switch (sel) {
        case 0: // race
            g_vehicle_state.tcs_level = 1;
            g_vehicle_state.abs_level = 8;
            break;
        case 1: // rain
            g_vehicle_state.tcs_level = 15;
            g_vehicle_state.abs_level = 1;
            break;
        // ... 其他模式
        case 5: // 自定義
            // 自定義模式保持當前設定
            break;
    }

    // 同步更新TCS與ABS控件
    if (tcs_slider) {
        lv_slider_set_value(tcs_slider, g_vehicle_state.tcs_level, LV_ANIM_ON);
    }
    if (tcs_level_label) {
        lv_label_set_text_fmt(tcs_level_label, "段數: %d", g_vehicle_state.tcs_level);
    }
    if (abs_dropdown) {
        uint16_t abs_selected = calculate_abs_dropdown_selection(g_vehicle_state.abs_level);
        lv_dropdown_set_selected(abs_dropdown, abs_selected);
    }
}
```

### 2. ABS選項重新編排

#### 問題點
ABS的回傳值與選項的數量和排序不一致，導致邏輯混亂。

#### 修改方案
重新定義ABS選項的數量與排序，確保與回傳值一致。

```c
static uint16_t calculate_abs_dropdown_selection(uint16_t abs_level)
{
    switch (abs_level) {
        case 0: return 0; // ABS關閉
        case 1: return 1; // ABS低
        case 2: return 2; // ABS中
        case 3: return 3; // ABS高
        default: return 0; // 預設為關閉
    }
}

static uint16_t calculate_abs_level_from_selection(uint16_t selection)
{
    switch (selection) {
        case 0: return 0; // ABS關閉
        case 1: return 1; // ABS低
        case 2: return 2; // ABS中
        case 3: return 3; // ABS高
        default: return 0; // 預設為關閉
    }
}
```

### 3. 測試案例更新

#### 測試場景1: 模式切換同步
1. 從race模式切換到rain模式。
2. 檢查TCS與ABS控件是否正確更新到rain模式的預設值。

#### 測試場景2: ABS選項檢查
1. 測試ABS下拉選單的每個選項，確保回傳值與選項一致。
2. 測試選項切換後，ABS值是否正確更新。

## 總結

本次修改重點解決了以下問題：
1. **TCS與ABS的同步邏輯**: 確保模式切換時正確更新。
2. **ABS選項的邏輯一致性**: 重新編排選項與回傳值的對應關係。
3. **亮度、字體大小、語言控件的獨立性**: 排除不必要的同步行為。

這些修改將進一步提升系統的穩定性與用戶體驗。
