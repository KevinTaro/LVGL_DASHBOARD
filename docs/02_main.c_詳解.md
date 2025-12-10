# main.c - 主程式進入點詳解

## 檔案資訊
- **檔案位置**: `/home/sshuser/LVGL/src/main.c`
- **總行數**: 349 行
- **功能**: 主程式進入點，負責 LVGL 初始化、中文字體載入、主迴圈管理

---

## 1. 程式功能說明

### 1.1 核心功能
`main.c` 是整個專案的**啟動核心**，負責以下關鍵任務：

1. ✅ **LVGL 圖形庫初始化**
2. ✅ **SDL2 硬體抽象層 (HAL) 初始化**
3. ✅ **中文字體系統設定**
4. ✅ **狀態管理器初始化**
5. ✅ **主儀表板介面建立**
6. ✅ **主事件迴圈 (Event Loop) 管理**

### 1.2 是否為原創程式
**部分原創，部分基於範例**：
- ✅ **原創部分 (約 60%)**:
  - 中文字體載入系統 (第 326-347 行)
  - 主程式初始化流程設計 (第 70-119 行)
  - 主儀表板整合呼叫

- 🔄 **修改部分 (約 30%)**:
  - LVGL 初始化流程 (基於官方範例)
  - SDL 視窗參數調整

- 📦 **官方範例 (約 10%)**:
  - 基礎的 LVGL 範例函式 (已註解，僅供參考)

---

## 2. 程式碼結構解析

### 2.1 整體架構
```
main.c
├── 標頭檔引入 (第 9-28 行)
├── 全域變數宣告 (第 43-48 行)
├── 函式宣告 (第 59-65 行)
├── main() 主函式 (第 70-120 行)
└── 輔助函式實作 (第 125-349 行)
    ├── 中文字體系統
    └── LVGL 範例函式 (已註解)
```

---

## 3. 重點程式碼解說

### 3.1 主函式 main() - 程式進入點

**位置**: 第 70-120 行

```c
int main(int argc, char **argv)
{
    (void)argc; /*Unused*/
    (void)argv; /*Unused*/

    /*Initialize LVGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    sdl_hal_init(800, 600);

    /* 初始化中文字體 */
    init_chinese_font();

    /* Initialize state manager */
    init_state_manager();

    /* Run the default demo */
    // ... 已註解的範例程式碼 ...

    // 建立主儀表
    create_main_dashboard(lv_screen_active());

    while (1)
    {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        uint32_t sleep_time_ms = lv_timer_handler();
        if (sleep_time_ms == LV_NO_TIMER_READY)
        {
            sleep_time_ms = LV_DEF_REFR_PERIOD;
        }
#ifdef _MSC_VER
        Sleep(sleep_time_ms);
#else
        usleep(sleep_time_ms * 1000);
#endif
    }

    return 0;
}
```

#### 📌 程式邏輯說明

**步驟 1: LVGL 初始化** (第 77 行)
```c
lv_init();
```
- 初始化 LVGL 圖形庫的核心系統
- 設定記憶體管理、繪圖引擎、事件系統

**步驟 2: SDL2 HAL 初始化** (第 80 行)
```c
sdl_hal_init(800, 600);
```
- 建立 800x600 的 SDL 視窗
- 初始化滑鼠/鍵盤輸入
- 設定顯示驅動與刷新計時器

**步驟 3: 中文字體初始化** (第 83 行)
```c
init_chinese_font();
```
- 載入系統中的 Noto Sans CJK 字體
- 設定 fallback 字體以支援符號

**步驟 4: 狀態管理器初始化** (第 86 行)
```c
init_state_manager();
```
- 初始化車輛狀態結構
- 啟動資料模擬計時器
- 重置所有控制輸入

**步驟 5: 建立主儀表板** (第 95 行)
```c
create_main_dashboard(lv_screen_active());
```
- 在當前活動畫面建立主儀表板 UI
- 綁定所有事件回呼函式

**步驟 6: 主事件迴圈** (第 97-115 行)
```c
while (1)
{
    uint32_t sleep_time_ms = lv_timer_handler();
    if (sleep_time_ms == LV_NO_TIMER_READY)
    {
        sleep_time_ms = LV_DEF_REFR_PERIOD;
    }
    usleep(sleep_time_ms * 1000);
}
```
- **無限迴圈**: 持續處理 LVGL 事件
- **lv_timer_handler()**:
  - 刷新螢幕顯示
  - 處理輸入事件
  - 執行計時器回呼
  - 返回建議的休眠時間
- **usleep()**: 休眠以節省 CPU 資源

---

### 3.2 中文字體初始化系統 ⭐

**位置**: 第 326-347 行

```c
/**
 * 初始化中文字體
 */
static void init_chinese_font(void)
{
    /* 使用系統安裝的 Noto Sans CJK 字體 */
    g_chinese_font = lv_freetype_font_create(
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
        16,
        LV_FREETYPE_FONT_STYLE_NORMAL
    );

    if (!g_chinese_font)
    {
        LV_LOG_ERROR("Failed to create Chinese font with FreeType");
        /* 回退到內建字體 */
        g_chinese_font = (lv_font_t *)&lv_font_source_han_sans_sc_16_cjk;
    }
    else
    {
        LV_LOG_INFO("Chinese font loaded successfully");
        /* 設置 fallback 字體以支援符號和特殊字符 */
        if (g_chinese_font->fallback == NULL)
        {
            g_chinese_font->fallback = LV_FONT_DEFAULT;
        }
    }
}
```

#### 📌 技術解析

**FreeType 字體載入**
```c
lv_freetype_font_create(
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",  // 字體檔案路徑
    LV_FREETYPE_FONT_RENDER_MODE_BITMAP,                       // 渲染模式：點陣圖
    16,                                                         // 字體大小：16px
    LV_FREETYPE_FONT_STYLE_NORMAL                              // 字體樣式：正常
);
```

**關鍵參數說明**：
1. **字體路徑**: `/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc`
   - 這是 Linux 系統中 Noto Sans CJK 字體的標準位置
   - 支援中文、日文、韓文字符

2. **渲染模式**: `LV_FREETYPE_FONT_RENDER_MODE_BITMAP`
   - 點陣圖渲染，速度較快
   - 適合嵌入式系統或低效能裝置

3. **字體大小**: `16`
   - 16 像素高度，適合儀表板資訊顯示

**錯誤處理機制**
```c
if (!g_chinese_font)
{
    LV_LOG_ERROR("Failed to create Chinese font with FreeType");
    g_chinese_font = (lv_font_t *)&lv_font_source_han_sans_sc_16_cjk;
}
```
- 如果 FreeType 載入失敗，回退到內建的思源黑體
- 確保程式不會因字體問題而崩潰

**Fallback 字體設定**
```c
if (g_chinese_font->fallback == NULL)
{
    g_chinese_font->fallback = LV_FONT_DEFAULT;
}
```
- 設定預設字體作為備用
- 當中文字體缺少某些符號時，自動使用預設字體

---

### 3.3 全域變數與宏定義

**位置**: 第 9-48 行

```c
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* needed for usleep() */
#endif

#include <stdlib.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif
#include "../lvgl/lvgl.h"
#include "../lvgl/examples/lv_examples.h"
#include "../lvgl/demos/lv_demos.h"
#include <SDL2/SDL.h>

#include "hal/hal.h"
#include "ui_main_dashboard.h"
#include "ui_state_manager.h"

/* 全域中文字體 */
static lv_font_t *g_chinese_font = NULL;
```

#### 📌 重點說明

**標頭檔引入**：
- `lvgl.h`: LVGL 核心標頭檔
- `SDL2/SDL.h`: SDL2 顯示後端
- `hal/hal.h`: 硬體抽象層
- `ui_main_dashboard.h`: 主儀表板介面
- `ui_state_manager.h`: 狀態管理系統

**全域變數**：
```c
static lv_font_t *g_chinese_font = NULL;
```
- 儲存中文字體的指標
- 使用 `static` 限制作用域在本檔案內
- 透過 `get_chinese_font()` 函式對外提供存取

---

### 3.4 字體存取函式

**位置**: 第 349 行

```c
/**
 * 獲取中文字體
 */
lv_font_t *get_chinese_font(void)
{
    return g_chinese_font;
}
```

#### 📌 設計模式說明

這是一個典型的 **Getter 函式**：
- 提供對私有全域變數的唯讀存取
- 其他模組（如 ui_main_dashboard.c）可以透過此函式取得字體
- 遵循封裝原則，隱藏實作細節

---

## 4. FreeRTOS 模式說明

### 4.1 條件編譯

**位置**: 第 67-68 行

```c
#if LV_USE_OS != LV_OS_FREERTOS

int main(int argc, char **argv)
{
    // ... 一般模式的 main 函式 ...
}

#endif
```

#### 📌 說明
- 當 `LV_USE_OS` 設定為 `LV_OS_FREERTOS` 時，此 `main()` 不會被編譯
- 改為使用 `freertos_main.c` 中的 FreeRTOS 入口函式
- 這是透過 `lv_conf.h` 中的設定控制：
  ```c
  #define LV_USE_OS   LV_OS_NONE  // 或 LV_OS_FREERTOS
  ```

---

## 5. 已註解的範例程式碼

### 5.1 FreeType 範例 (第 140-165 行)

```c
#if LV_USE_FREETYPE

void lv_example_get_started_1(void)
{
    /*Create a font*/
    lv_font_t *font = lv_freetype_font_create(
        PATH_PREFIX "lvgl/examples/libs/freetype/NotoSerifCJK-Regular.ttc",
        LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
        24,
        LV_FREETYPE_FONT_STYLE_NORMAL
    );

    if (!font)
    {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_add_style(label, &style, 0);
    lv_label_set_text(label, "Hello 你好\nNoto 中文字型,日文,한글");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

#endif
```

#### 📌 參考價值
- 這是官方的 FreeType 字體載入範例
- 展示如何建立自訂樣式並套用到標籤
- 我們的 `init_chinese_font()` 即參考此範例設計

### 5.2 按鈕範例 (第 247-262 行)

```c
void lv_example_get_started_2(void)
{
    lv_obj_t *btn = lv_button_create(lv_screen_active());
    lv_obj_set_pos(btn, 120, 360);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);
}
```

#### 📌 參考價值
- 展示基本的按鈕建立與事件綁定
- 這些技巧在設定選單中大量使用

---

## 6. 編譯與執行流程

### 6.1 編譯設定 (CMakeLists.txt)

```cmake
set(MAIN_SOURCES
    src/mouse_cursor_icon.c
    src/hal/hal.c
    src/ui_main_dashboard.c
    src/ui_state_manager.c
    src/ui_settings_menu.c
    src/ui_dashboard_layouts.c
)

if(USE_FREERTOS)
    list(APPEND MAIN_SOURCES src/freertos_main.c ...)
else()
    list(APPEND MAIN_SOURCES src/main.c)
endif()
```

- 根據 `USE_FREERTOS` 選項決定編譯 `main.c` 或 `freertos_main.c`

### 6.2 執行流程圖

```
開始
  │
  ▼
lv_init()  ←─ LVGL 核心初始化
  │
  ▼
sdl_hal_init(800, 600)  ←─ SDL2 視窗建立
  │
  ▼
init_chinese_font()  ←─ 載入中文字體
  │
  ▼
init_state_manager()  ←─ 初始化狀態管理
  │
  ▼
create_main_dashboard()  ←─ 建立主儀表板
  │
  ▼
┌─────────────────────┐
│   主事件迴圈 (無限)   │
│ ┌─────────────────┐ │
│ │lv_timer_handler()│ │
│ └─────────────────┘ │
│         │           │
│         ▼           │
│   usleep(ms)        │
│         │           │
│         └───────────┘
└─────────────────────┘
```

---

## 7. 常見問題與除錯

### 7.1 中文字體無法顯示

**問題**: 中文顯示為方框 (tofu)

**解決方法**:
1. 確認系統已安裝 Noto Sans CJK 字體：
   ```bash
   sudo apt install fonts-noto-cjk
   ```

2. 檢查字體路徑是否正確：
   ```bash
   ls /usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc
   ```

3. 查看日誌輸出：
   ```
   [INFO] Chinese font loaded successfully
   ```

### 7.2 視窗無法開啟

**問題**: 執行時無視窗顯示

**解決方法**:
1. 確認 SDL2 已正確安裝：
   ```bash
   sdl2-config --version
   ```

2. 檢查 DISPLAY 環境變數（Linux）：
   ```bash
   echo $DISPLAY
   ```

3. 嘗試強制軟體渲染：
   ```bash
   SDL_VIDEODRIVER=x11 ./bin/main
   ```

---

## 8. 與其他模組的關聯

### 8.1 呼叫關係圖

```
main.c
  │
  ├──> lv_init()                (LVGL 核心)
  ├──> sdl_hal_init()           (hal/hal.c)
  ├──> init_chinese_font()      (本檔案)
  ├──> init_state_manager()     (ui_state_manager.c)
  ├──> create_main_dashboard()  (ui_main_dashboard.c)
  └──> lv_timer_handler()       (LVGL 核心)
```

### 8.2 資料流向

```
main.c
  │
  ├── 字體指標 ──> ui_main_dashboard.c (透過 get_chinese_font())
  │                      │
  │                      └──> ui_settings_menu.c
  │
  └── 狀態結構 ──> ui_state_manager.c (g_vehicle_state)
                         │
                         └──> ui_main_dashboard.c
                         └──> ui_settings_menu.c
```

---

## 9. 報告重點建議

### 9.1 適合報告的內容

✅ **推薦講解**：
1. **主程式初始化流程** (第 70-95 行)
   - 展示程式啟動的完整步驟
   - 說明各模組如何協同工作

2. **中文字體系統** (第 326-347 行)
   - 這是完全原創的功能
   - 展現對 FreeType 和 LVGL 的整合能力

3. **主事件迴圈設計** (第 97-115 行)
   - 說明事件驅動架構
   - 展現對效能優化的考量

### 9.2 投影片建議

**第 1 張**: 標題
- 「main.c - 程式啟動核心」

**第 2 張**: 初始化流程圖
- 用流程圖展示 main() 的執行步驟

**第 3 張**: 中文字體系統
- 展示程式碼片段
- 說明 FreeType 整合方法

**第 4 張**: 事件迴圈
- 說明為何需要無限迴圈
- 解釋休眠機制的效能考量

---

## 10. 參考資料

- [LVGL 初始化文件](https://docs.lvgl.io/master/intro/index.html)
- [SDL2 視窗管理](https://wiki.libsdl.org/SDL2/CategoryVideo)
- [FreeType 文件](https://www.freetype.org/freetype2/docs/)
- [LVGL Font 系統](https://docs.lvgl.io/master/details/main-components/fonts.html)

---

**檔案版本**: v1.0
**最後更新**: 2025-11-11
