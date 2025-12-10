# 摩托車儀表盤程式碼優化計畫

## 文檔概述

本文檔為期中作業的完整優化計畫，著重於使用 LVGL example 風格實作，保持程式碼簡潔易懂，適合初學者理解和維護。

---

## 一、當前系統分析

### 1.1 現有功能模組
- **主儀表板** (`ui_main_dashboard.c`): 顯示轉速、時速、檔位、TCS/ABS 狀態
- **設定選單** (`ui_settings_menu.c`): 提供騎乘模式、TCS/ABS、顯示設定調整
- **狀態管理** (`ui_state_manager.c`): 管理車輛狀態、驗證數值範圍

### 1.2 已識別的問題
1. **TCS 滑桿同步問題**: 模式切換時，滑桿位置未正確更新
2. **ABS 數值邏輯**: 目前使用 0-3 範圍，需對應到正確的功能說明
3. **事件觸發時機**: 已改為 `LV_EVENT_RELEASED`，但實作可以更簡化
4. **程式碼可讀性**: 部分邏輯可以更清晰

---

## 二、騎乘模式規格確認

### 2.1 各模式預設值

| 模式 | TCS 等級 | ABS 設定 | 說明 |
|------|----------|----------|------|
| Race | 1 | 前後輪關閉 (3) | 賽道模式，最低循跡控制 |
| Rain | 15 | 全開 (1) | 雨天模式，最高安全輔助 |
| Normal | 7 | 全開 (1) | 一般街道騎乘 |
| Offroad | 3 | Offroad ABS (0) | 越野模式，特殊 ABS 設定 |
| Supermoto | 0 | 後輪關閉 (2) | 滑胎模式，允許後輪滑動 |
| 自定義 | (保持) | (保持) | 沿用上次設定值 |

### 2.2 ABS 選項定義

```c
// ABS 等級對應 (0-3)
typedef enum {
    ABS_OFFROAD = 0,      // Offroad 專用 ABS
    ABS_FULL = 1,         // 全開 (前後輪啟動)
    ABS_REAR_OFF = 2,     // 後輪關閉
    ABS_BOTH_OFF = 3      // 前後輪關閉
} abs_level_t;
```

---

## 三、優化計畫

### 3.1 核心修復項目

#### 修復項目 1: TCS 滑桿同步
**問題**: 模式切換時，TCS 滑桿位置未更新
**原因**: 滑桿值和顯示標籤分開更新
**解決方案**: 參考 LVGL example 的做法，統一更新函式

```c
// 參考 lv_example_slider_1() 風格
static void update_tcs_ui(int tcs_value)
{
    // 同時更新滑桿和標籤
    if (tcs_slider) {
        lv_slider_set_value(tcs_slider, tcs_value, LV_ANIM_ON);
    }
    if (tcs_level_label) {
        lv_label_set_text_fmt(tcs_level_label, "段數: %d", tcs_value);
    }
}
```

**修改位置**: `ui_settings_menu.c` 的 `riding_mode_event_cb()`

---

#### 修復項目 2: 簡化 ABS 映射邏輯
**問題**: ABS 映射函式過於複雜
**解決方案**: 使用直接映射，參考 example 的簡單風格

```c
// 簡化版本 - 直接對應
static uint16_t calculate_abs_dropdown_selection(uint16_t abs_level)
{
    return abs_level; // 直接映射 0-3
}

static uint16_t calculate_abs_level_from_selection(uint16_t selection)
{
    return selection; // 直接映射 0-3
}
```

**修改位置**: `ui_settings_menu.c` 的映射函式區段

---

#### 修復項目 3: 統一更新函式
**問題**: UI 更新分散在多處
**解決方案**: 建立統一的更新函式，參考 LVGL example 的模式

```c
// 參考 lv_example_get_started 的風格
static void sync_mode_ui_controls(void)
{
    // 更新 TCS
    update_tcs_ui(g_vehicle_state.tcs_level);

    // 更新 ABS
    if (abs_dropdown) {
        lv_dropdown_set_selected(abs_dropdown, g_vehicle_state.abs_level);
    }

    // 更新模式顯示
    if (roller) {
        lv_roller_set_selected(roller, g_vehicle_state.riding_mode, LV_ANIM_OFF);
    }
}
```

**修改位置**: 新增輔助函式區段

---

### 3.2 程式碼結構優化

#### 優化項目 1: 事件回呼簡化
**目標**: 讓事件處理更像 LVGL example 的風格

```c
// 參考 lv_example_slider 的簡潔風格
static void slider_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *slider = lv_event_get_target_obj(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        int value = lv_slider_get_value(slider);

        // 自定義模式：直接更新
        if (g_vehicle_state.riding_mode == 5) {
            g_vehicle_state.tcs_level = value;
            update_tcs_ui(value);
            return;
        }

        // 非自定義模式：暫時顯示但不保存
        if (tcs_level_label) {
            lv_label_set_text_fmt(tcs_level_label, "段數: %d", value);
        }
    }
    else if (code == LV_EVENT_RELEASED) {
        // 手放開後確認是否切換模式
        if (g_vehicle_state.riding_mode != 5) {
            int value = lv_slider_get_value(slider);
            show_mode_change_warning_with_data(slider, value, 0);
        }
    }
}
```

---

#### 優化項目 2: 對話框處理簡化
**目標**: 使用更簡潔的 msgbox 實作

```c
// 參考 lv_example_msgbox 的簡潔風格
static void show_mode_change_warning_with_data(lv_obj_t *control, int new_value, int setting_type)
{
    // 建立對話框
    lv_obj_t *msgbox = lv_msgbox_create(NULL);
    lv_msgbox_add_title(msgbox, "模式保護");

    // 簡單的訊息
    static char msg_buf[128];
    const char *setting_names[] = {"TCS", "亮度", "ABS"};
    snprintf(msg_buf, sizeof(msg_buf),
             "調整 %s 將切換為自定義模式\n新值: %d\n是否繼續？",
             setting_names[setting_type], new_value);
    lv_msgbox_add_text(msgbox, msg_buf);

    // 按鈕
    lv_obj_t *btn_no = lv_msgbox_add_footer_button(msgbox, "取消");
    lv_obj_t *btn_yes = lv_msgbox_add_footer_button(msgbox, "確定");

    // 儲存資料
    ModeChangeData_t *data = lv_malloc(sizeof(ModeChangeData_t));
    data->dialog = msgbox;
    data->control = control;
    data->new_value = new_value;
    data->setting_type = setting_type;

    // 設定事件
    lv_obj_add_event_cb(btn_no, msgbox_cancel_cb, LV_EVENT_CLICKED, data);
    lv_obj_add_event_cb(btn_yes, msgbox_ok_cb, LV_EVENT_CLICKED, data);
}

// 取消按鈕回呼
static void msgbox_cancel_cb(lv_event_t *e)
{
    ModeChangeData_t *data = lv_event_get_user_data(e);

    // 恢復原值
    sync_mode_ui_controls();

    // 關閉並清理
    lv_msgbox_close(data->dialog);
    lv_free(data);
}

// 確定按鈕回呼
static void msgbox_ok_cb(lv_event_t *e)
{
    ModeChangeData_t *data = lv_event_get_user_data(e);

    // 切換到自定義模式
    g_vehicle_state.riding_mode = 5;

    // 應用新值
    switch (data->setting_type) {
        case 0: g_vehicle_state.tcs_level = data->new_value; break;
        case 1: g_vehicle_state.brightness = data->new_value; break;
        case 2: g_vehicle_state.abs_level = data->new_value; break;
    }

    // 更新 UI
    sync_mode_ui_controls();

    // 關閉並清理
    lv_msgbox_close(data->dialog);
    lv_free(data);
}
```

---

### 3.3 狀態管理優化

#### 優化項目 1: 預設值管理
**目標**: 更清晰的預設值結構

```c
// 在 ui_state_manager.h 中定義清晰的預設值
typedef struct {
    int tcs_level;
    int abs_level;
} RidingModeDefaults_t;

// 預設值表格 - 對應 ABS 0-3 範圍
static const RidingModeDefaults_t MODE_DEFAULTS[6] = {
    {1,  3},  // Race:      TCS 1,  ABS 前後輪關閉
    {15, 1},  // Rain:      TCS 15, ABS 全開
    {7,  1},  // Normal:    TCS 7,  ABS 全開
    {3,  0},  // Offroad:   TCS 3,  ABS Offroad
    {0,  2},  // Supermoto: TCS 0,  ABS 後輪關閉
    {7,  1}   // 自定義預設值 (實際會被用戶設定覆蓋)
};
```

---

#### 優化項目 2: 模式切換邏輯
**目標**: 簡化模式切換的處理

```c
// 參考 example 的簡潔風格
static void riding_mode_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_VALUE_CHANGED) return;

    lv_obj_t *roller = lv_event_get_target_obj(e);
    uint16_t sel = lv_roller_get_selected(roller);

    // 更新模式
    g_vehicle_state.riding_mode = sel;

    // 非自定義模式：套用預設值
    if (sel != 5) {
        g_vehicle_state.tcs_level = MODE_DEFAULTS[sel].tcs_level;
        g_vehicle_state.abs_level = MODE_DEFAULTS[sel].abs_level;
    }
    // 自定義模式：保持當前值

    // 更新所有 UI
    sync_mode_ui_controls();
}
```

---

## 四、實作步驟

### 步驟 1: 準備工作
1. 備份當前程式碼
2. 確認 LVGL example 參考來源
3. 準備測試環境

### 步驟 2: 修改 `ui_state_manager.h`
- 更新 ABS 範圍註解為 0-3
- 定義清晰的預設值結構
- 添加 enum 定義 (可選)

### 步驟 3: 修改 `ui_settings_menu.c`
按順序執行以下修改：

1. **新增輔助函式**
   - `update_tcs_ui()` - TCS UI 更新
   - `sync_mode_ui_controls()` - 統一 UI 同步
   - `msgbox_cancel_cb()` - 對話框取消處理
   - `msgbox_ok_cb()` - 對話框確定處理

2. **簡化映射函式**
   - `calculate_abs_dropdown_selection()` - 直接映射
   - `calculate_abs_level_from_selection()` - 直接映射

3. **重構事件回呼**
   - `riding_mode_event_cb()` - 使用新的輔助函式
   - `slider_event_cb()` - 簡化事件處理
   - `abs_dropdown_event_cb()` - 簡化事件處理

4. **重構對話框**
   - `show_mode_change_warning_with_data()` - 使用新的回呼分離
   - 移除舊的 `mode_change_warning_cb()` - 替換為兩個新函式

### 步驟 4: 修改 `ui_main_dashboard.c`
- 更新 ABS 顯示邏輯以對應 0-3 範圍
- 使用新的 ABS 名稱對應

### 步驟 5: 測試驗證
1. 編譯程式
2. 測試每個模式切換
3. 測試 TCS/ABS 調整
4. 測試對話框互動
5. 驗證數值範圍

---

## 五、程式碼風格指南

### 5.1 遵循 LVGL Example 風格

```c
// ✓ 好的範例 - 簡潔清晰
static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target_obj(e);
        // 處理邏輯
    }
}

// ✗ 避免 - 過於複雜
static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target_obj(e);

    if (code != LV_EVENT_CLICKED)
        return;

    // 多層嵌套...
}
```

### 5.2 命名規範

```c
// 函式命名：動詞開頭
update_tcs_ui()          // ✓
sync_mode_ui_controls()  // ✓
tcs_ui_update()          // ✗

// 變數命名：名詞
tcs_slider               // ✓
abs_dropdown             // ✓
slider_tcs               // ✗

// 常數：大寫
MODE_DEFAULTS            // ✓
ABS_FULL                 // ✓
mode_defaults            // ✗
```

### 5.3 註解風格

```c
// 簡短註解 - 說明「為什麼」而非「是什麼」
g_vehicle_state.riding_mode = 5;  // 切換到自定義模式

// 區段註解 - 標示功能區塊
/*
 * TCS 滑桿事件處理
 * 自定義模式：直接更新
 * 其他模式：顯示確認對話框
 */
static void slider_event_cb(lv_event_t *e)
{
    // ...
}
```

---

## 六、測試計畫

### 6.1 單元測試項目

| 測試項目 | 測試步驟 | 預期結果 |
|---------|---------|---------|
| 模式切換 | 依序切換 6 種模式 | TCS/ABS 值正確更新，滑桿位置同步 |
| TCS 調整 | 在 Race 模式調整 TCS | 彈出對話框，確定後切換到自定義 |
| ABS 調整 | 在 Rain 模式調整 ABS | 彈出對話框，確定後切換到自定義 |
| 對話框取消 | 調整後點取消 | 恢復原值，保持原模式 |
| 自定義模式 | 在自定義模式調整 | 直接更新，不彈出對話框 |

### 6.2 整合測試

1. **完整流程測試**
   - 啟動程式 → 進入設定 → 切換模式 → 調整參數 → 返回主畫面
   - 驗證所有值正確顯示

2. **邊界值測試**
   - TCS: 0, 15
   - ABS: 0, 3
   - 驗證不會超出範圍

3. **UI 響應測試**
   - 快速切換模式
   - 快速調整滑桿
   - 驗證 UI 不會卡頓或錯誤

---

## 七、文件更新

### 7.1 需要更新的註解

1. **`ui_state_manager.h`**
   - 更新 ABS 範圍說明: `1-8` → `0-3`
   - 添加 ABS 各級別說明

2. **`ui_settings_menu.c`**
   - 更新函式功能註解
   - 添加參考的 LVGL example

3. **README 或使用說明**
   - 更新模式預設值表格
   - 添加操作說明

### 7.2 建議新增的文件

1. **USAGE.md** - 使用者指南
   - 各模式說明
   - 操作流程
   - 常見問題

2. **ARCHITECTURE.md** - 架構說明
   - 模組關係圖
   - 資料流向
   - 狀態管理

---

## 八、優化效果預期

### 8.1 程式碼品質
- 減少程式碼行數約 15-20%
- 提高可讀性和維護性
- 更符合 LVGL example 風格

### 8.2 功能完整性
- 修復 TCS 滑桿同步問題
- 簡化 ABS 邏輯
- 改善使用者體驗

### 8.3 學習成果展示
- 展示對 LVGL 的理解
- 展示程式碼重構能力
- 展示系統設計思維

---

## 九、注意事項

### 9.1 保持簡潔原則
- 不追求過度優化
- 保持程式碼可讀性為優先
- 參考 example 的實作方式

### 9.2 避免的陷阱
- 不要使用過於複雜的設計模式
- 不要過度抽象化
- 不要脫離 LVGL example 風格

### 9.3 期中作業重點
- 功能正確完整
- 程式碼清晰易懂
- 展示學習成果
- 文件完整

---

## 十、參考資源

### 10.1 LVGL Examples
```
lvgl/examples/widgets/slider/lv_example_slider_1.c
lvgl/examples/widgets/msgbox/lv_example_msgbox_1.c
lvgl/examples/widgets/dropdown/lv_example_dropdown_1.c
lvgl/examples/get_started/lv_example_get_started_1.c
```

### 10.2 相關文件
- LVGL 官方文檔: https://docs.lvgl.io/
- LVGL API 參考: https://docs.lvgl.io/master/API/index.html
- 範例程式碼: lvgl/examples/

---

## 十一、時程規劃

| 階段 | 預估時間 | 工作內容 |
|-----|---------|---------|
| 準備 | 0.5 小時 | 備份程式碼、確認環境 |
| 修改標頭檔 | 0.5 小時 | 更新 ui_state_manager.h |
| 修改設定選單 | 2 小時 | 重構 ui_settings_menu.c |
| 修改主儀表 | 0.5 小時 | 更新 ui_main_dashboard.c |
| 測試 | 1 小時 | 完整功能測試 |
| 文件 | 0.5 小時 | 更新註解和說明 |
| **總計** | **5 小時** | |

---

## 十二、總結

本優化計畫遵循以下原則：

1. **簡潔優先**: 參考 LVGL example 的簡潔風格
2. **功能完整**: 修復已知問題，保持功能完整性
3. **易於理解**: 適合期中作業展示和說明
4. **可維護性**: 清晰的程式碼結構和註解

執行本計畫後，將得到一個功能完整、程式碼清晰、符合 LVGL 開發風格的摩托車儀表盤系統，適合作為期中作業展示。

---

**文件版本**: 1.0
**建立日期**: 2025-11-11
**作者**: 期中專題開發團隊
**狀態**: 待執行
