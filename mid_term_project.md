# 專案計畫書：高階智慧機車儀表 (LVGL 模擬器) - v2.0

## 0. 專案環境與架構補充

- **開發環境**：Linux + VS Code，已整合 CMake/Makefile，支援 GCC 編譯與 C/C++ 調試。
- **目錄結構**：
    - `src/`：主要程式碼（建議模組化拆分 UI、邏輯、HAL）。
    - `lvgl/`：LVGL 原始碼與範例，可直接參考/複製官方 `examples`。
    - `FreeRTOS/`：可選用 RTOS 任務管理（如需多任務 UI/模擬）。
    - `core/`、`config/`：可放置全域設定、共用結構。
    - `build/`、`bin/`：編譯輸出與執行檔。
- **建議檔案規劃**：
    - `src/ui_main_dashboard.c/h`
    - `src/ui_settings_menu.c/h`
    - `src/ui_state_manager.c/h`
    - `src/main.c`（僅負責初始化與主迴圈）

---

## 1. 專案概述

### 1.1 專案名稱
LVGL Smart Motorcycle Dashboard Simulator (高階智慧機車儀表模擬器)

### 1.2 專案願景
本專案旨在建立一個功能完整、視覺精美、互動流暢的高階智慧機車儀表板模擬器。專案將以歐洲頂級車款（如 BMW, Ducati, KTM）的 TFT 儀表為美學藍本，強調**科技感、運動化與高度的資訊整合**。

### 1.3 核心目標
* **技術掌握**：深入掌握 LVGL 的核心組件、佈局（Grid/Flex）、樣式 (Style) 及動畫 (Animation) 系統，並實作複雜的狀態管理。
* **開發策略**：以 LVGL 官方 `examples` 為起點進行功能迭代與客製化，驗證「範例驅動開發 (Example-Driven Development)」的高效率。
* **成果展示**：產出一個視覺效果出色、互動邏輯嚴謹的個人作品集 (Portfolio)，可作為期中專案或未來求職亮點。

### 1.4 開發策略：範例驅動開發 (Example-Driven Development)
本專案**不**從零開始。我們將嚴格遵循此策略，以確保開發效率：
1.  **鎖定功能**：確定要實現的 UI 功能（例如：轉速表）。
2.  **定位範例**：在 [lvgl/examples](https://github.com/lvgl/lvgl/tree/master/examples) 目錄中找到最接近的官方範例（例如：`lv_example_meter_4.c`）。
3.  **拆解與修改**：複製範例程式碼，理解其構成，然後對參數、樣式、事件處理進行「魔改」，使其符合我們的儀表板美學與邏輯。

4.  **CMake/Makefile 整合**：每新增模組都在 CMakeLists.txt/Makefile 註冊，確保能順利編譯。
5.  **外部字體/中文化**：將字體檔（.c）放入 `src/fonts/` 或 `lvgl/fonts/`，並在初始化時載入。
6.  **FreeRTOS 整合**（如需）：可將數據模擬、UI刷新分別放入 RTOS 任務，或維持 LVGL timer 架構。


## 2. 專案範圍與階段規劃 (Milestones)

本專案將分為四個主要階段 (Phase) 進行迭代開發。

### 補充：建議開發流程

- **範例驅動開發**：直接複製 `lvgl/examples` 內的 C 檔到 `src/`，並依需求重命名、拆解、客製化。
- **模組化設計**：每個 UI/邏輯功能獨立 .c/.h，減少 main.c 複雜度。
- **狀態管理**：`ui_state_manager.c/h` 集中 VehicleState 結構、所有 timer 與動畫邏輯。
- **資源管理**：圖片、字體、顏色統一放置於 `src/assets/` 或 `lvgl/assets/`，方便維護。
- **測試與除錯**：善用 VS Code 調試功能，並可於 `src/` 新增 `test_*.c` 進行單元測試。

### Phase 1: 核心儀表 MVP (靜態佈局)
* **目標**: 實現主儀表 (Main Dashboard) 的**靜態**顯示，完成 80% 的視覺佈局。
* **任務**:
    1.  **主畫面佈局 (Screen Layout)**:
        * **策略**: 使用 `lv_grid` 進行像素級精確佈局。建立一個主 `lv_obj_t` (作為 Screen)，並使用 `lv_obj_set_grid_dsc_array` 定義儀表的宏觀佈局（例如：頂部狀態區、中部儀表區、底部資訊區）。
        * **參考**: `lv_example_grid_6.c` (展示如何定義複雜的行和列)。
    2.  **[轉速表 (Tachometer)]**:
        * **策略**: 修改 `lv_example_meter_4.c`。移除原有的指針和標籤，建立一個半圓形或 270 度的儀表。
        * **客製化**:
            * 使用 `lv_meter_add_scale` 建立一個 0-13000 RPM 的刻度。
            * 使用 `lv_meter_add_arc` **建立多個圓弧**，用不同顏色標示「怠速區」(藍色)、「巡航區」(白色)、 「紅線區」(紅色)。
            * 使用 `lv_meter_add_needle_line` 增加一個線條指針。
    3.  **[時速與檔位 (Speed & Gear)]**:
        * **策略**: 修改 `lv_example_label_1.c`。
        * **客製化**:
            * 載入一個具科技感的**外部數位字體** (使用 [Online Font Converter](https://lvgl.io/tools/fontconverter))。
            * 建立兩個 `lv_label`，一個用於時速（大字體），一個用於檔位（中字體，如 "N", "1", "2"）。
            * 使用 `lv_obj_align` 或 `lv_grid` 將它們精確放置在轉速表中央。
    4.  **[狀態燈號 (Status Indicators)]**:
        * **策略**: 在頂部 grid 區域，使用 `lv_flex` (`lv_example_flex_1.c`) 建立一個橫向容器。
        * **客製化**: 在容器中放置多個 `lv_label`，使用 LVGL 內建的 Symbols 字體顯示圖示 (例如 `LV_SYMBOL_LEFT`, `LV_SYMBOL_EYE_OPEN` [遠燈], `LV_SYMBOL_WARNING` [引擎])。
        * **參考**: `lv_example_label_2.c` (展示如何使用 Symbols)。

    > **補充建議**：
    > - 先於 `src/ui_main_dashboard.c` 完成靜態佈局，確保能編譯執行。
    > - 字體、Symbol、顏色等資源先行準備，避免後期整合困難。

### Phase 2: 互動選單與狀態管理
* **目標**: 實現可互動的設定選單，並建立儀表的核心狀態機，使 UI 能響應邏輯。
* **任務**:
    1.  **畫面導航 (Screen Navigation)**:
        * **策略**: 學習如何管理多個 `lv_screen`。主儀表是一個 Screen，設定選單是另一個 Screen。
        * **參考**: `lv_example_event_4.c` (展示如何透過事件建立和切換對象)。使用 `lv_scr_load_anim` 實現切換動畫。
    2.  **[選單結構 (Menu Structure)]**:
        * **策略**: 在新的「設定 Screen」中，使用 `lv_example_list_1.c` 建立主設定列表。
        * **客製化**:
            * 建立列表項："騎乘模式", "顯示設定", "車輛資訊"。
            * 為每個列表項添加 `LV_EVENT_CLICKED` 事件。點擊後，導航至對應的子設定頁面（例如 `create_riding_mode_page()`）。
    3.  **[子選單實作 (Sub-menus)]**:
        * **騎乘模式**: 修改 `lv_example_roller_1.c`，選項改為 "Street\nSport\nRace\nRain"。
        * **電控調整**: 修改 `lv_example_slider_1.c`，實現 TCS/ABS 的 1-8 級滑動調整。
        * **亮度調整**: 修改 `lv_example_arc_2.c`，建立一個旋鈕 (Arc) 來調整模擬的螢幕亮度，這比 Slider 更酷。
    4.  **[核心狀態管理 (State Machine)]**:
        * **策略**: 建立一個全域的 `struct VehicleState` 來儲存所有狀態（如 `current_mode`, `tcs_level`, `abs_level`）。
        * **實現**: 當 `roller` (騎乘模式) 觸發 `LV_EVENT_VALUE_CHANGED` 時，在其回呼函式中：
            1.  更新 `VehicleState` 結構體。
            2.  **連動更新**: 根據新的模式（例如 "Rain"），強制更新 TCS/ABS `slider` 的值（例如設為 8 級）。
            3.  **事件廣播**: 使用 `lv_event_send(lv_scr_act(), LV_EVENT_REFRESH, NULL)` 發送一個自訂的刷新事件，通知主儀表更新顯示。

    > **補充建議**：
    > - 設定選單獨立於 `src/ui_settings_menu.c`，並設計 Screen 切換 API。
    > - 狀態機邏輯集中於 `ui_state_manager.c`，UI 只負責顯示。

### Phase 3: 數據模擬與流暢動畫
* **目標**: 讓儀表「動起來」，模擬真實行車數據，並確保所有動畫平滑流暢。
* **任務**:
    1.  **[數據模擬器 (Data Simulator)]**:
        * **策略**: 使用 `lv_timer` 建立一個 50ms 執行的定時器 (參考 `lv_example_timer_1.c`)。
        * **實現 (邏輯強化)**: 在 Timer 回呼函式中模擬真實的物理關聯：
            * 模擬「檔位」與「轉速/時速」的關係。
            * 模擬加速：`current_rpm += 300; current_speed = current_rpm / 150 * current_gear;` (這只是一個範例公式)。
            * 模擬換檔：當轉速 > 10000 且檔位 < 6，`current_gear++` 且 `current_rpm` 回落至 7000。
            * 所有 UI 更新都**禁止**在此 Timer 中直接呼叫 `lv_label_set_text` 或 `lv_meter_set_value`，而是只更新 `VehicleState` 結構體中的目標值。
    2.  **[UI 刷新定時器 (UI Refresh Timer)]**:
        * **策略**: 另外建立一個 20ms (較快) 的 `lv_timer`，專門負責 UI 渲染。
        * **實現**: 此 Timer 讀取 `VehicleState` 中的目標值，並呼叫 `lv_label_set_text_fmt(...)` 和 `lv_meter_set_indicator_value(...)` 來更新顯示。
    3.  **[平滑指針動畫 (Smooth Animation)]**:
        * **策略**: 為了讓轉速表指針移動**極度平滑**，我們不使用 Timer 直接設定值，而是使用 `lv_anim`。
        * **參考**: `lv_example_anim_1.c`。
        * **實現**:
            1.  在「數據模擬器」中，當 `target_rpm` 變更時，**啟動一個動畫**:
                ```c
                lv_anim_t a;
                lv_anim_init(&a);
                lv_anim_set_var(&a, my_tachometer_needle); // 動畫對象
                lv_anim_set_values(&a, lv_meter_get_indicator_value(..., needle), target_rpm); // 起始值, 結束值
                lv_anim_set_time(&a, 200); // 動畫時長 (ms)
                lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_meter_set_indicator_value); // 動畫期間執行的函式
                lv_anim_set_path_cb(&a, lv_anim_path_ease_out); // 動畫曲線，使其更自然
                lv_anim_start(&a);
                ```
    4.  **[數據分頁 (Trip Info)]**:
        * **策略**: 修改 `lv_example_tabview_1.c` 建立 "Trip A", "Trip B", "即時油耗" 三個分頁。
        * **客製化**: 在頁面中，使用 `lv_grid` (`lv_example_grid_2.c`) 來整齊排列數據標題與數值（例如 "平均油耗", "騎乘時間"）。

    > **補充建議**：
    > - 依據 VehicleState 設計 timer 與動畫，確保 UI/邏輯分離。
    > - 可考慮將模擬參數（如加速公式）抽象為 config 檔，方便調整。

### Phase 4: 美學優化與程式碼重構
* **目標**: 進行 UI/UX 拋光，使其具備「高級感」，並重構程式碼以提高可維護性。
* **任務**:
    1.  **[美學微調 (Polishing)]**:
        * **漸層背景**: 使用 `lv_style_set_bg_grad` (參考 `lv_example_style_11.c`) 為儀表背景添加深色漸層，增加科技感。
        * **立體陰影**: 使用 `lv_style_set_shadow_...` (參考 `lv_example_style_10.c`) 為指針、時速文字添加細微陰影，增加立體感。
        * **精美線條**: 使用 `lv_style_set_line_...` 美化 `meter` 的刻度線和 `arc` 的邊緣。
        * **統一調色**: 定義一套全域調色板 (例如 `COLOR_PRIMARY`, `COLOR_ACCENT`, `COLOR_REDLINE`) 並統一應用。
    2.  **[中文化實施]**:
        * **策略**: 嚴格執行「按需加載」策略。
        * **步驟**:
            1.  使用 LVGL 線上工具轉換字體。
            2.  在 `Symbols` 欄位貼上所有用到的中文字：「騎乘模式 顯示 設定 車輛 資訊 街道 運動 賽道 雨天 單次 里程 平均 油耗 公里 時間 亮度 循跡 控制」。
            3.  產生 `.c` 檔並載入專案，建立全域的 `style_chinese_text` 並應用到所有中文 `label`。
    3.  **[程式碼重構 (Refactoring)]**:
        * **風險**: 所有程式碼擠在 `main.c` 會導致災難。
        * **應對**: **模組化**。
            * `ui_main_dashboard.c / .h`: 提供 `create_main_dashboard(lv_obj_t *parent)` 函式。
            * `ui_settings_menu.c / .h`: 提供 `create_settings_screen(void)` 函式。
            * `ui_state_manager.c / .h`: 集中管理 `VehicleState` 結構體和所有數據模擬 `timer`。
        * **目標**: `main.c` 中只保留 LVGL 初始化和 `lv_timer_handler()`，以及呼叫 `create_main_dashboard()`。

    > **補充建議**：
    > - 全域樣式、調色板於 `src/ui_style.c/h` 管理。
    > - 中文字體、資源按需載入，減少記憶體佔用。
    > - 程式碼重構時，善用 VS Code 的「重命名」、「尋找引用」功能，確保不遺漏。

## 5. 專案管理建議

- **README.md**：持續更新專案說明、架構、模組分工。
- **mid_term_project.md**：每階段完成後，補充遇到的問題與解決方案，作為期末報告素材。
- **版本控管**：善用 Git，重要里程碑（如 MVP 完成、互動選單完成）建立 tag，方便回溯。

## 3. 功能模組與 `examples` 核心對照表

| 功能模組 | 子功能 | 核心 LVGL 組件 | 建議修改的範例檔案 | **客製化重點 (How-to)** |
| :--- | :--- | :--- | :--- | :--- |
| **主儀表 (Dashboard)** | 宏觀佈局 | `lv_grid` | [lv_example_grid_6.c](https://github.com/lvgl/lvgl/blob/master/examples/layouts/grid/lv_example_grid_6.c) | 用 `lv_obj_set_grid_dsc_array` 定義儀表的主區域，實現像素級對齊。 |
| | 轉速表 (Tachometer) | `lv_meter` | [lv_example_meter_4.c](https://github.com/lvgl/lvgl/blob/master/examples/widgets/meter/lv_example_meter_4.c) | **使用 `lv_meter_add_arc` 疊加多個不同顏色的圓弧** (藍/白/紅) 來標示紅線區。 |
| | 時速/檔位 | `lv_label` | [lv_example_label_1.c](https://github.com/lvgl/lvgl/blob/master/examples/widgets/label/lv_example_label_1.c) | 使用 `lv_style_set_text_font` 載入**外部數位字體**。用 `lv_label_set_text_fmt` 更新時速。 |
| | 狀態燈號 (ABS, TCS) | `lv_label` (Symbols) | [lv_example_label_2.c](https://github.com/lvgl/lvgl/blob/master/examples/widgets/label/lv_example_label_2.c) | 使用 `lv_style_set_text_color` 改變 Symbol 顏色 (灰 -> 亮綠/亮紅) 來表示狀態。 |
| **設定選單 (Settings)** | 主選單列表 | `lv_list` | [lv_example_list_1.c](https://github.com/lvgl/lvgl/blob/master/examples/widgets/list/lv_example_list_1.c) | 為 `lv_list_add_btn` 綁定 `LV_EVENT_CLICKED` 事件，用於切換頁面。 |
| | 騎乘模式選擇 | `lv_roller` | [lv_example_roller_1.c](https://github.com/lvgl/lvgl/blob/master/examples/widgets/roller/lv_example_roller_1.c) | 監聽 `LV_EVENT_VALUE_CHANGED` 事件，以此觸發全域狀態變更。 |
| | 電控等級 (TCS/ABS) | `lv_slider` | [lv_example_slider_1.c](https://github.com/lvgl/lvgl/blob/master/examples/widgets/slider/lv_example_slider_1.c) | 使用 `LV_SLIDER_MODE_RANGE` 模式，並在 Mode 變更時，**程式化地呼叫 `lv_slider_set_value`**。 |
| | 亮度調整 | `lv_arc` | [lv_example_arc_2.c](https://github.com/lvgl/lvgl/blob/master/examples/widgets/arc/lv_example_arc_2.c) | `arc` 提供的旋鈕效果比 `slider` 更有質感，適合模擬亮度/音量。 |
| **騎乘數據 (Trip Info)** | 數據分頁 (Trip A/B) | `lv_tabview` | [lv_example_tabview_1.c](https://github.com/lvgl/lvgl/blob/master/examples/widgets/tabview/lv_example_tabview_1.c) | 建立 "Trip A", "Trip B" 兩個 Tab，並在 Tab 頁面中使用 `lv_grid` 排版數據。 |
| | 傾角/油耗圖 (加分項) | `lv_chart` | [lv_example_chart_2.c](https://github.com/lvgl/lvgl/blob/master/examples/widgets/chart/lv_example_chart_2.c) | 使用 `lv_chart_set_ext_y_array` 動態更新圖表數據系列。 |
| **核心邏輯** | 數據模擬 | `lv_timer` | [lv_example_timer_1.c](https://github.com/lvgl/lvgl/blob/master/examples/timer/lv_example_timer_1.c) | **建立兩個 Timer**：一個 (50ms) 用於模擬數據邏輯，另一個 (20ms) 用於刷新 UI。 |
| | 平滑動畫 | `lv_anim` | [lv_example_anim_1.c](https://github.com/lvgl/lvgl/blob/master/examples/anim/lv_example_anim_1.c) | **這是流暢感的關鍵**。使用 `lv_anim_set_exec_cb` 和 `lv_anim_path_ease_out`。 |

---

## 4. 關鍵技術方案詳解

### 4.1 數據模擬與狀態管理 (C 偽代碼)
這是專案的核心，必須將 UI 和 邏輯 分離。

```c
// ui_state_manager.h
typedef struct {
    int current_rpm;
    int target_rpm; // 數據模擬器更新此值
    int current_speed;
    int current_gear;
    int riding_mode; // 0:Street, 1:Sport
    int tcs_level;
    // ... 其他狀態
} VehicleState_t;

// 全域狀態實例
extern VehicleState_t g_vehicle_state;

// 初始化函式
void init_state_manager(void);


// ui_state_manager.c
VehicleState_t g_vehicle_state;

// 數據模擬 Timer 回呼 (每 50ms)
static void data_simulator_cb(lv_timer_t *timer) {
    // ... 複雜的物理模擬 ...
    // 範例：模擬加速
    g_vehicle_state.target_rpm += 500;
    if (g_vehicle_state.target_rpm > 13000) {
        g_vehicle_state.target_rpm = 13000;
    }
    // ...
}

void init_state_manager(void) {
    lv_memset(&g_vehicle_state, 0, sizeof(VehicleState_t));
    g_vehicle_state.current_gear = 1;
    g_vehicle_state.tcs_level = 5;
    // ...

    // 建立數據模擬 Timer
    lv_timer_create(data_simulator_cb, 50, NULL);
}

// ui_main_dashboard.c

// 轉速表動畫回呼 (由 lv_anim 系統呼叫)
static void tachometer_anim_cb(void *obj, int32_t value) {
    lv_meter_set_indicator_value(g_tachometer, g_tachometer_needle, value);
    g_vehicle_state.current_rpm = value; // 同步狀態
}

// UI 刷新 Timer 回呼 (每 20ms)
static void ui_refresh_cb(lv_timer_t *timer) {
    // 檢查 RPM 是否需要動畫
    if (g_vehicle_state.current_rpm != g_vehicle_state.target_rpm) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, g_tachometer);
        // 從當前顯示值 動畫到 目標值
        lv_anim_set_values(&a, g_vehicle_state.current_rpm, g_vehicle_state.target_rpm);
        lv_anim_set_time(&a, 150); // 動畫稍快
        lv_anim_set_exec_cb(&a, tachometer_anim_cb);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_start(&a);
    }

    // 直接更新時速 (Label 更新很快，不需要動畫)
    lv_label_set_text_fmt(g_speed_label, "%d", g_vehicle_state.current_speed);
}
