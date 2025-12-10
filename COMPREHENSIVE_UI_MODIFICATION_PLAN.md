# 綜合 UI 修改計畫文檔

## 專案概述
本文檔詳細規劃了摩托車儀表板系統的重大修改，包括 UI 改進、主題修正、儀表布局重構等。

## 騎乘模式與儀表布局對應表

| 騎乘模式 | 儀表布局類型 | 布局特色 |
|---------|------------|---------|
| **Race** | Race 賽道布局 | 橫條轉速 + 圈速計時 + 大號速度/檔位 |
| **Rain** | Normal 雙環布局 | 左圓環轉速 + 右圓環速度 |
| **Normal** | Normal 雙環布局 | 左圓環轉速 + 右圓環速度 |
| **Offroad** | Offroad 越野布局 | 垂直轉速條 + 超大檔位/速度 |
| **Supermoto** | Race 賽道布局 | 橫條轉速 + 圈速計時 + 大號速度/檔位 |
| **自定義** | 用戶可選 | 三種布局任選，預設 Normal |

---

## 任務一：移除語言切換選項

### 1.1 目標
- 移除顯示設定中的語言選項
- 系統統一使用中文顯示
- 清理相關的語言切換程式碼

### 1.2 影響的檔案
- `src/ui_settings_menu.c` - 移除語言下拉選單
- `src/ui_state_manager.h` - 移除 language 欄位（選擇性）
- `src/ui_state_manager.c` - 移除語言初始化

### 1.3 實作步驟
1. 在 `create_display_settings_page()` 函數中移除語言選項的創建程式碼
2. 移除 `lang_event_cb()` 事件回呼函數
3. 清理相關的全域變數和註釋
4. 保留 `get_chinese_font()` 函數，因為其他地方仍在使用

### 1.4 測試要點
- 顯示設定頁面只顯示：亮度調整、字體大小調整
- 所有文字保持中文顯示
- 無任何語言切換相關的 UI 元素

---

## 任務二：修正字體放大縮小邏輯並套用到全部 UI

### 2.1 目標
- 建立統一的字體大小管理系統
- 讓字體大小調整套用到所有 UI 元件
- 提供三種字體大小：小、中、大

### 2.2 當前問題分析
- 字體大小調整只影響部分元件
- 缺乏全域字體更新機制
- 主儀表板可能未響應字體大小變化

### 2.3 設計方案

#### 2.3.1 字體大小定義
```c
// 在 ui_state_manager.h 中定義
typedef enum {
    FONT_SIZE_SMALL = 0,   // 小字體 (12-14pt)
    FONT_SIZE_MEDIUM = 1,  // 中字體 (16-18pt)
    FONT_SIZE_LARGE = 2    // 大字體 (20-24pt)
} FontSize_t;
```

#### 2.3.2 字體獲取函數
在 `ui_settings_menu.c` 或獨立的 `ui_font_manager.c` 中：
```c
const lv_font_t* get_font_by_size(FontSize_t size, bool is_label);
// 返回對應大小的 LVGL 字體指標
// is_label: true 為一般文字，false 為數字/大標題
```

#### 2.3.3 全域更新函數
```c
void update_all_ui_fonts(void);
// 遍歷所有 UI 元件，更新字體
// 需要觸發主儀表板、設定頁面的重繪
```

### 2.4 影響的檔案
- `src/ui_state_manager.h` - 新增 FontSize_t 枚舉
- `src/ui_settings_menu.c` - 修改字體調整邏輯
- `src/ui_main_dashboard.c` - 新增字體更新響應
- 可能需要新增：`src/ui_font_manager.c/h`

### 2.5 實作步驟
1. 定義字體大小枚舉和映射表
2. 修改 `g_vehicle_state.font_size` 為枚舉類型
3. 實作 `get_font_by_size()` 函數
4. 實作 `update_all_ui_fonts()` 函數
5. 在設定頁面的字體調整回呼中調用全域更新
6. 為主儀表板的所有文字標籤添加字體更新邏輯

### 2.6 UI 元件清單（需要更新字體）
- 主儀表板：
  - 速度數字標籤
  - 轉速數字標籤
  - 檔位顯示
  - TCS/ABS 狀態標籤
- 設定頁面：
  - 所有選單項目標籤
  - 按鈕文字
  - 對話框文字
  - 滑桿標籤

### 2.7 測試要點
- 調整字體大小後，所有畫面的文字都應立即更新
- 三種字體大小都清晰可讀
- 不同字體大小下，UI 布局不會錯亂

---

## 任務三：修正選單元件日夜切換問題

### 3.1 目標
- 修正設定選單在夜間模式下仍顯示白色背景的問題
- 確保所有 UI 元件都響應主題切換

### 3.2 當前問題分析
- 主頁面的日夜切換可能只影響了部分元件
- 設定選單的背景色、文字色沒有跟隨主題變化
- 對話框（msgbox）可能也有類似問題

### 3.3 設計方案

#### 3.3.1 主題顏色定義
```c
// 在 ui_state_manager.h 或 ui_theme.h 中
typedef struct {
    lv_color_t bg_color;        // 背景色
    lv_color_t text_color;      // 文字色
    lv_color_t panel_color;     // 面板色
    lv_color_t border_color;    // 邊框色
    lv_color_t accent_color;    // 強調色
} ThemeColors_t;

extern ThemeColors_t current_theme_colors;
```

#### 3.3.2 主題切換函數
```c
void apply_theme_to_all_ui(bool is_dark);
// 套用主題到所有 UI 元件
// 包括：主儀表板、設定頁面、對話框
```

### 3.4 影響的檔案
- `src/ui_settings_menu.c` - 添加主題響應
- `src/ui_main_dashboard.c` - 確認主題切換邏輯
- 可能需要新增：`src/ui_theme.c/h` - 主題管理

### 3.5 實作步驟
1. 檢查 `is_dark_theme` 全域變數的使用情況
2. 在 `create_settings_screen()` 中添加主題初始化
3. 為設定頁面的所有容器添加動態樣式
4. 實作 `apply_theme_to_settings_menu()` 函數
5. 在主題切換回呼中調用設定頁面的主題更新
6. 確保對話框（msgbox）也響應主題

### 3.6 需要添加主題樣式的元件
- `settings_screen` - 主容器
- `main_list` - 選單列表
- 所有 `cont` 容器
- 所有標籤（label）的文字顏色
- 滑桿、下拉選單、roller 的背景和文字
- 對話框背景和按鈕

### 3.7 測試要點
- 切換日夜模式後，設定頁面應立即變色
- 夜間模式：深色背景、淺色文字
- 日間模式：淺色背景、深色文字
- 所有互動元件（按鈕、滑桿等）都應清晰可見

---

## 任務四：將預設模式從 race 改為 normal

### 4.1 目標
- 系統啟動時預設為 normal 模式
- 載入 normal 模式的預設 TCS 和 ABS 值

### 4.2 影響的檔案
- `src/ui_state_manager.c` - `init_state_manager()` 函數

### 4.3 實作步驟
```c
void init_state_manager(void)
{
    // ...
    g_vehicle_state.riding_mode = 2; // normal (原為 0 race)
    // 使用 normal 模式的預設值初始化
    g_vehicle_state.tcs_level = riding_mode_defaults[2].tcs_level;  // normal: TCS 7
    g_vehicle_state.abs_level = riding_mode_defaults[2].abs_level;  // normal: ABS 1 (全開)
    // ...
}
```

### 4.4 測試要點
- 第一次啟動，騎乘模式顯示 "normal"
- TCS 顯示 7
- ABS 顯示 "全開"

---

## 任務五：改為經典雙環儀表布局（Normal 模式）

### 5.1 目標
- 將現有的轉速表和速度表改為經典的雙環設計
- 左邊圓環顯示轉速（RPM）
- 右邊圓環顯示車速（km/h）
- 此布局作為 normal 模式的專屬儀表

### 5.2 設計規格

#### 5.2.1 布局結構
```
+----------------------------------+
|                                  |
|   +---------+     +---------+    |
|   |   RPM   |     |  Speed  |    |
|   |  /---\  |     |  /---\  |    |
|   | |  8  | |     | | 120 | |    |
|   |  \---/  |     |  \---/  |    |
|   | x1000rpm|     |  km/h   |    |
|   +---------+     +---------+    |
|                                  |
|        Gear: 3    TCS: 7         |
|        ABS: 全開               |
+----------------------------------+
```

#### 5.2.2 圓環儀表參數
- **轉速表（左側）**：
  - 範圍：0-15000 RPM
  - 刻度：0, 3, 6, 9, 12, 15 (x1000)
  - 紅線區：12000-15000（紅色弧段）
  - 圓環起始角度：135°，結束角度：45°（270° 範圍）
  - 使用 `lv_arc` 或 `lv_meter` 元件

- **速度表（右側）**：
  - 範圍：0-200 km/h
  - 刻度：0, 50, 100, 150, 200
  - 圓環起始角度：135°，結束角度：45°（270° 範圍）
  - 使用 `lv_arc` 或 `lv_meter` 元件

#### 5.2.3 中央資訊顯示
- 檔位：大號字體，置中下方
- TCS/ABS 狀態：小號字體，檔位下方

### 5.3 LVGL 元件選擇
推薦使用 `lv_meter` 元件（符合 LVGL 官方 example）：
- 支援多個刻度和指針
- 可設定顏色漸變
- 有內建動畫效果

參考 LVGL example：`lv_example_meter_1()`, `lv_example_meter_2()`

### 5.4 影響的檔案
- `src/ui_main_dashboard.c` - 重構儀表顯示邏輯
- 可能需要新增：`src/ui_dashboard_layouts.c/h` - 分離不同布局

### 5.5 實作步驟
1. 創建 `create_dual_meter_layout()` 函數
2. 使用 `lv_meter_create()` 創建兩個圓環
3. 配置轉速表刻度和紅線區
4. 配置速度表刻度
5. 添加中央數字顯示（大字體）
6. 添加檔位和 TCS/ABS 狀態顯示
7. 實作數據更新函數 `update_dual_meter_layout()`

### 5.6 程式碼結構範例
```c
typedef struct {
    lv_obj_t *rpm_meter;      // 轉速圓環
    lv_obj_t *rpm_label;      // 轉速數字
    lv_obj_t *speed_meter;    // 速度圓環
    lv_obj_t *speed_label;    // 速度數字
    lv_obj_t *gear_label;     // 檔位顯示
    lv_obj_t *tcs_label;      // TCS 狀態
    lv_obj_t *abs_label;      // ABS 狀態
} DualMeterLayout_t;

static DualMeterLayout_t dual_meter_ui;

void create_dual_meter_layout(lv_obj_t *parent);
void update_dual_meter_layout(void);
```

### 5.7 測試要點
- 兩個圓環正確顯示並居中
- 轉速表在 12000 以上顯示紅色
- 數值更新流暢，無閃爍
- 所有資訊清晰可讀
- 符合 LVGL example 的程式碼風格

---

## 任務六：新增 Race 模式儀表布局

### 6.1 目標
- 為 race 模式設計專屬的賽道風格儀表
- 強調性能數據和快速讀取

### 6.2 設計規格

#### 6.2.1 布局結構
```
+----------------------------------+
| Lap Time: 01:23.456              |
+----------------------------------+
|  RPM [||||||||||||||||------]    |
|     0  3  6  9  12  15 x1000     |
+----------------------------------+
|                                  |
|       Speed         Gear         |
|        180            5          |
|        km/h                      |
|                                  |
|  TCS: 1        ABS: 前後輪關閉   |
+----------------------------------+
```

#### 6.2.2 特色元件

**1. 圈速計時器（Lap Timer）**
- 位置：頂部，全寬
- 格式：MM:SS.mmm（分:秒.毫秒）
- 功能：模擬圈速計時（從 0 開始累加）
- 顏色：綠色文字，深色背景
- 字體：等寬字體（數字）

**2. 橫條式轉速表**
- 位置：頂部下方，全寬
- 類型：水平進度條 `lv_bar`
- 範圍：0-15000 RPM
- 顏色漸變：
  - 0-9000: 綠色
  - 9000-12000: 黃色
  - 12000-15000: 紅色
- 刻度標記：0, 3, 6, 9, 12, 15 (x1000)
- 參考：`lv_example_bar_6()` 帶漸變顏色

**3. 速度與檔位顯示**
- 位置：中央，大號顯示
- 速度：超大字體（48-60pt），置中左側
- 檔位：超大字體（48-60pt），置中右側
- 單位標註：小字體於下方

**4. TCS/ABS 狀態**
- 位置：底部
- 字體：中等大小
- 簡潔顯示當前設定

### 6.3 LVGL 元件選擇
- 圈速計時器：`lv_label` + `lv_timer`
- 轉速條：`lv_bar` 元件
- 速度/檔位：`lv_label` 大字體
- 刻度標記：多個 `lv_label` 小字體

### 6.4 影響的檔案
- `src/ui_dashboard_layouts.c/h` - 新增 race 布局
- `src/ui_main_dashboard.c` - 添加布局切換邏輯

### 6.5 實作步驟
1. 創建 `create_race_layout()` 函數
2. 實作圈速計時器顯示和更新邏輯
3. 創建橫條式轉速表（`lv_bar`）
4. 配置顏色漸變（使用樣式）
5. 添加刻度標記
6. 創建大號速度和檔位標籤
7. 實作 `update_race_layout()` 函數
8. 添加計時器模擬邏輯（可選）

### 6.6 程式碼結構範例
```c
typedef struct {
    lv_obj_t *lap_timer_label;   // 圈速計時
    lv_obj_t *rpm_bar;           // 轉速橫條
    lv_obj_t *speed_label;       // 速度（大字）
    lv_obj_t *gear_label;        // 檔位（大字）
    lv_obj_t *tcs_label;         // TCS 狀態
    lv_obj_t *abs_label;         // ABS 狀態
    uint32_t lap_time_ms;        // 圈速時間（毫秒）
} RaceLayout_t;

static RaceLayout_t race_ui;

void create_race_layout(lv_obj_t *parent);
void update_race_layout(void);
void reset_lap_timer(void);  // 重置圈速
```

### 6.7 測試要點
- 圈速計時器正確計時和顯示
- 轉速條顏色漸變正確
- 速度和檔位數字清晰、大而易讀
- 紅線區（12000+）轉速條顯示紅色
- 整體風格符合賽車儀表感

---

## 任務七：新增 Offroad 模式儀表布局

### 7.1 目標
- 為 offroad 模式設計越野風格儀表
- 強調檔位和速度，轉速為輔助資訊

### 7.2 設計規格

#### 7.2.1 布局結構
```
+----------------------------------+
|                                  |
| RPM  Gear        Speed           |
| |||    3          85             |
| |||                              |
| |||              km/h            |
| |||                              |
| |||  TCS: 3    ABS: Offroad      |
| |||                              |
| |||                              |
| |||                              |
| 15                               |
| 12                               |
| 9                                |
| 6                                |
| 3                                |
| 0                                |
+----------------------------------+
```

#### 7.2.2 特色元件

**1. 碩大的檔位顯示**
- 位置：左側偏中央
- 字體：超大（72-96pt）
- 樣式：粗體，高對比
- 顏色：白色/黃色（夜間模式可調）

**2. 碩大的速度顯示**
- 位置：右側中央偏上
- 字體：超大（60-72pt）
- 單位：km/h，小字體於下方
- 顏色：主要顏色（白/黑依主題）

**3. 垂直轉速條**
- 位置：畫面最左側
- 方向：由下至上
- 類型：`lv_bar`（垂直方向）
- 寬度：30-50 像素
- 高度：佔據畫面大部分高度
- 範圍：0-15000 RPM
- 刻度標記：0, 3, 6, 9, 12, 15（垂直排列於左側）
- 顏色：
  - 0-12000: 藍/綠色
  - 12000-15000: 紅色

**4. TCS/ABS 狀態**
- 位置：底部或中下
- 字體：中等大小
- 風格：簡潔

### 7.3 LVGL 元件選擇
- 垂直轉速條：`lv_bar` 設定為垂直方向
- 檔位：`lv_label` 超大字體
- 速度：`lv_label` 超大字體
- 刻度：多個 `lv_label`

### 7.4 影響的檔案
- `src/ui_dashboard_layouts.c/h` - 新增 offroad 布局
- `src/ui_main_dashboard.c` - 添加布局切換邏輯

### 7.5 實作步驟
1. 創建 `create_offroad_layout()` 函數
2. 創建垂直轉速條（`lv_bar`，設定 LV_BAR_VERTICAL）
3. 添加轉速刻度標記（左側垂直排列）
4. 創建超大檔位標籤（左側中央）
5. 創建超大速度標籤（右側中央）
6. 添加 TCS/ABS 狀態（底部）
7. 實作 `update_offroad_layout()` 函數

### 7.6 程式碼結構範例
```c
typedef struct {
    lv_obj_t *rpm_bar;           // 垂直轉速條
    lv_obj_t *gear_label;        // 檔位（超大）
    lv_obj_t *speed_label;       // 速度（超大）
    lv_obj_t *speed_unit_label;  // 速度單位
    lv_obj_t *tcs_label;         // TCS 狀態
    lv_obj_t *abs_label;         // ABS 狀態
    lv_obj_t *rpm_scale_labels[6]; // 轉速刻度標記
} OffroadLayout_t;

static OffroadLayout_t offroad_ui;

void create_offroad_layout(lv_obj_t *parent);
void update_offroad_layout(void);
```

### 7.7 垂直轉速條設定範例
```c
rpm_bar = lv_bar_create(parent);
lv_obj_set_size(rpm_bar, 40, 400);  // 寬 40px，高 400px
lv_obj_align(rpm_bar, LV_ALIGN_LEFT_MID, 10, 0);
lv_bar_set_range(rpm_bar, 0, 15000);
lv_bar_set_value(rpm_bar, current_rpm, LV_ANIM_ON);

// 設定為垂直
lv_obj_set_style_transform_angle(rpm_bar, 0, 0);  // 確保垂直
```

### 7.8 測試要點
- 垂直轉速條正確由下至上增長
- 檔位和速度數字極大且清晰
- 刻度標記位於轉速條左側，垂直排列
- 高轉速時轉速條顯示紅色
- 適合越野騎乘時快速讀取資訊

---

## 任務八：自定義模式顯示儀表布局選項

### 8.1 目標
- 在自定義模式的設定頁面中顯示儀表布局選項
- 非自定義模式隱藏此選項
- 允許用戶選擇三種布局之一

### 8.2 設計規格

#### 8.2.1 布局選項
- **Normal 布局**：雙環儀表（預設）
- **Race 布局**：橫條轉速 + 大號速度檔位 + 圈速
- **Offroad 布局**：垂直轉速 + 超大檔位速度

#### 8.2.2 UI 設計
在 `create_riding_mode_page()` 中：
```
當 riding_mode == 5 (自定義) 時顯示：

騎乘模式選擇器
TCS 滑動條
ABS 下拉選單
----------------
儀表布局 [下拉選單]   <-- 新增
  ▸ Normal 雙環
  ▸ Race 賽道
  ▸ Offroad 越野
```

當 riding_mode != 5 時，隱藏儀表布局選項

### 8.3 資料結構修改

#### 8.3.1 新增布局枚舉
```c
// 在 ui_state_manager.h 中
typedef enum {
    DASHBOARD_LAYOUT_NORMAL = 0,   // 雙環儀表
    DASHBOARD_LAYOUT_RACE = 1,     // 賽道布局
    DASHBOARD_LAYOUT_OFFROAD = 2   // 越野布局
} DashboardLayout_t;
```

#### 8.3.2 擴充 VehicleState_t
```c
typedef struct {
    // ...現有欄位
    int dashboard_layout;  // 儀表布局 (DashboardLayout_t)
} VehicleState_t;
```

### 8.4 影響的檔案
- `src/ui_state_manager.h` - 新增枚舉和欄位
- `src/ui_state_manager.c` - 初始化新欄位
- `src/ui_settings_menu.c` - 添加布局選項和顯示/隱藏邏輯
- `src/ui_main_dashboard.c` - 實作布局切換

### 8.5 實作步驟

#### 8.5.1 設定頁面修改
1. 在 `create_riding_mode_page()` 中添加布局下拉選單
2. 創建 `dashboard_layout_dropdown` 全域變數
3. 實作 `dashboard_layout_event_cb()` 回呼函數
4. 在 `riding_mode_event_cb()` 中添加顯示/隱藏邏輯：
   ```c
   if (sel == 5) {
       // 自定義模式：顯示布局選項
       lv_obj_clear_flag(dashboard_layout_dropdown, LV_OBJ_FLAG_HIDDEN);
   } else {
       // 非自定義模式：隱藏布局選項
       lv_obj_add_flag(dashboard_layout_dropdown, LV_OBJ_FLAG_HIDDEN);
   }
   ```

#### 8.5.2 主儀表板修改
1. 在 `ui_main_dashboard.c` 中實作 `switch_dashboard_layout()` 函數
2. 根據 `g_vehicle_state.dashboard_layout` 切換顯示的布局
3. 隱藏當前布局，顯示新布局
4. 確保數據更新函數調用正確的布局更新函數

#### 8.5.3 布局切換邏輯
```c
void switch_dashboard_layout(DashboardLayout_t layout) {
    // 隱藏所有布局
    if (dual_meter_ui.rpm_meter) {
        lv_obj_add_flag(dual_meter_container, LV_OBJ_FLAG_HIDDEN);
    }
    if (race_ui.rpm_bar) {
        lv_obj_add_flag(race_container, LV_OBJ_FLAG_HIDDEN);
    }
    if (offroad_ui.rpm_bar) {
        lv_obj_add_flag(offroad_container, LV_OBJ_FLAG_HIDDEN);
    }

    // 顯示選定的布局
    switch (layout) {
        case DASHBOARD_LAYOUT_NORMAL:
            lv_obj_clear_flag(dual_meter_container, LV_OBJ_FLAG_HIDDEN);
            break;
        case DASHBOARD_LAYOUT_RACE:
            lv_obj_clear_flag(race_container, LV_OBJ_FLAG_HIDDEN);
            break;
        case DASHBOARD_LAYOUT_OFFROAD:
            lv_obj_clear_flag(offroad_container, LV_OBJ_FLAG_HIDDEN);
            break;
    }

    g_vehicle_state.dashboard_layout = layout;
}
```

### 8.6 非自定義模式的布局綁定
```c
// 在模式切換時自動設定對應的布局
// 在 riding_mode_event_cb 或 msgbox_ok_cb 中
static const DashboardLayout_t mode_default_layouts[6] = {
    DASHBOARD_LAYOUT_RACE,    // race - 使用賽道布局
    DASHBOARD_LAYOUT_NORMAL,  // rain - 使用雙環布局
    DASHBOARD_LAYOUT_NORMAL,  // normal - 使用雙環布局
    DASHBOARD_LAYOUT_OFFROAD, // offroad - 使用越野布局
    DASHBOARD_LAYOUT_RACE,    // supermoto - 使用賽道布局
    DASHBOARD_LAYOUT_NORMAL   // custom (預設，可由用戶更改)
};

// 切換模式時
if (sel != 5) {  // 非自定義模式
    g_vehicle_state.dashboard_layout = mode_default_layouts[sel];
    switch_dashboard_layout(mode_default_layouts[sel]);
}
```

### 8.7 程式碼結構範例
```c
// 全域變數
static lv_obj_t *dashboard_layout_dropdown = NULL;
static lv_obj_t *dashboard_layout_label = NULL;

// 事件回呼
static void dashboard_layout_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *dropdown = lv_event_get_target_obj(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        uint16_t sel = lv_dropdown_get_selected(dropdown);
        g_vehicle_state.dashboard_layout = sel;
        switch_dashboard_layout((DashboardLayout_t)sel);
    }
}

// 在 create_riding_mode_page() 中
dashboard_layout_label = lv_label_create(cont);
lv_label_set_text(dashboard_layout_label, "儀表布局");
lv_obj_set_style_text_font(dashboard_layout_label, get_chinese_font(), 0);

dashboard_layout_dropdown = lv_dropdown_create(cont);
lv_dropdown_set_options(dashboard_layout_dropdown, "Normal 雙環\nRace 賽道\nOffroad 越野");
lv_obj_set_width(dashboard_layout_dropdown, lv_pct(80));
lv_dropdown_set_selected(dashboard_layout_dropdown, g_vehicle_state.dashboard_layout);
lv_obj_add_event_cb(dashboard_layout_dropdown, dashboard_layout_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

// 根據當前模式決定是否顯示
if (g_vehicle_state.riding_mode != 5) {
    lv_obj_add_flag(dashboard_layout_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(dashboard_layout_dropdown, LV_OBJ_FLAG_HIDDEN);
}
```

### 8.8 測試要點
- 自定義模式下，顯示儀表布局選項
- 非自定義模式下，隱藏儀表布局選項
- 切換布局後，主儀表板立即更新
- **race 模式**自動使用 **race 布局**（橫條轉速+圈速）
- **supermoto 模式**自動使用 **race 布局**（橫條轉速+圈速）
- **offroad 模式**自動使用 **offroad 布局**（垂直轉速+超大顯示）
- **normal 模式**自動使用 **normal 布局**（雙環儀表）
- **rain 模式**自動使用 **normal 布局**（雙環儀表）
- 自定義模式記住用戶選擇的布局

---

## 整體實作順序建議

### 階段一：基礎清理和修正（1-4 天）
1. 任務一：移除語言切換（簡單）
2. 任務四：改變預設模式（簡單）
3. 任務三：修正日夜切換（中等難度）

### 階段二：字體系統重構（2-3 天）
4. 任務二：字體放大縮小邏輯（複雜）

### 階段三：儀表布局開發（5-7 天）
5. 任務五：Normal 雙環布局（中高難度）
6. 任務六：Race 賽道布局（中高難度）
7. 任務七：Offroad 越野布局（中高難度）

### 階段四：整合和優化（2-3 天）
8. 任務八：自定義模式布局選項（中等難度）
9. 整體測試和調整
10. 程式碼優化和註釋補充

---

## 測試計畫

### 功能測試
1. 語言選項已移除，系統使用中文
2. 字體調整影響所有 UI 元件
3. 日夜模式切換作用於所有頁面
4. 預設啟動為 normal 模式
5. 三種儀表布局都能正常顯示和更新
6. 自定義模式顯示布局選項
7. 非自定義模式隱藏布局選項且自動綁定布局

### 效能測試
1. 布局切換流暢，無延遲
2. 數據更新無閃爍
3. 記憶體使用合理

### 相容性測試
1. 不同解析度下顯示正常
2. 字體大小調整後布局不錯亂

---

## 風險評估

### 高風險項目
1. **字體系統重構**：可能影響所有現有 UI
   - 緩解：先在測試分支實作，充分測試後合併
2. **儀表布局重構**：程式碼量大，複雜度高
   - 緩解：分階段實作，每個布局獨立測試

### 中風險項目
1. **主題切換修正**：需要追蹤所有 UI 元件
   - 緩解：建立元件清單，逐一檢查
2. **布局切換邏輯**：需要妥善管理多個布局
   - 緩解：使用清晰的資料結構和函數封裝

### 低風險項目
1. 移除語言選項
2. 改變預設模式

---

## 程式碼風格指引

### 遵循 LVGL 官方範例
1. 使用 LVGL 內建元件，避免過度客製化
2. 參考 `examples/widgets/` 中的程式碼
3. 使用標準的事件回呼模式
4. 適當使用動畫效果（LV_ANIM_ON）

### 命名規範
- 函數：`動詞_名詞_名詞` (如 `create_dual_meter_layout`)
- 變數：`描述性名稱` (如 `rpm_meter`, `speed_label`)
- 枚舉：`大寫_下劃線` (如 `DASHBOARD_LAYOUT_NORMAL`)

### 註釋要求
- 每個主要函數添加功能說明
- 複雜邏輯添加行內註釋
- 保持中英文註釋混用（重要處使用中文）

---

## 結論

本文檔詳細規劃了 8 個重大修改任務，涵蓋 UI 清理、主題修正、字體系統、儀表布局重構等。實作時應按階段進行，確保每個階段充分測試後再進入下一階段。預計總開發時間：12-17 天。

所有修改都應遵循 LVGL 官方範例的程式碼風格，保持簡潔、易讀、易維護的原則。
