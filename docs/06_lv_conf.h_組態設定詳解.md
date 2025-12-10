# lv_conf.h 組態設定詳解

## 檔案資訊
- **檔案位置**: `/home/sshuser/LVGL/lv_conf.h`
- **總行數**: 1454 行
- **功能**: LVGL 圖形庫組態檔
- **版本**: v9.4.0-dev

---

## 1. 組態檔功能說明

### 1.1 核心功能
`lv_conf.h` 是 LVGL 的**中央組態檔**，控制：

1. ✅ **記憶體管理** - Heap 大小、記憶體池設定
2. ✅ **顯示設定** - 色彩深度、刷新率、DPI
3. ✅ **功能開關** - 啟用/停用 LVGL 功能模組
4. ✅ **字體系統** - FreeType、內建字體設定
5. ✅ **作業系統整合** - FreeRTOS、POSIX 執行緒
6. ✅ **效能調校** - 繪圖緩衝區、動畫設定

### 1.2 是否為原創設定
**🔄 基於官方模板修改 (約 10% 修改)**：
- 📦 使用 `lv_conf_template.h` 作為基礎
- 🆕 調整記憶體大小 (1MB heap)
- 🆕 啟用 FreeType 字體渲染
- 🆕 調整刷新率與 DPI
- 🆕 設定作業系統模式 (可選 FreeRTOS)

---

## 2. 重點設定解析

### 2.1 色彩深度設定 ⭐

**位置**: 第 28 行

```c
/** Color depth: 1 (I1), 8 (L8), 16 (RGB565), 24 (RGB888), 32 (XRGB8888) */
#define LV_COLOR_DEPTH 32
```

#### 📌 色彩深度說明

| 設定值 | 格式 | 每像素位元數 | 顏色數量 | 適用場景 |
|--------|------|-------------|---------|---------|
| 1 | I1 | 1 bit | 2 色 (黑白) | 電子紙顯示器 |
| 8 | L8 | 8 bit | 256 色 | 單色 LCD |
| 16 | RGB565 | 16 bit | 65536 色 | 嵌入式 TFT |
| 24 | RGB888 | 24 bit | 16.7M 色 | 高階顯示器 |
| **32** | **XRGB8888** | **32 bit** | **16.7M 色 + Alpha** | **PC 模擬器 (本專案)** |

**本專案選擇 32-bit 的原因**:
- ✅ SDL2 預設支援 ARGB8888
- ✅ 支援透明度 (Alpha 通道)
- ✅ 最佳視覺效果
- ✅ PC 記憶體充足，無需考慮資源限制

---

### 2.2 記憶體管理設定 ⭐⭐

**位置**: 第 75-87 行

```c
#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN
    /** Size of memory available for `lv_malloc()` in bytes (>= 2kB) */
    #define LV_MEM_SIZE (1 * 1024 * 1024)  // 1MB

    /** Size of the memory expand for `lv_malloc()` in bytes */
    #define LV_MEM_POOL_EXPAND_SIZE 0

    /** Set an address for the memory pool instead of allocating it as a normal array.
     *  Can be in external SRAM too. */
    #define LV_MEM_ADR 0     /**< 0: unused*/
```

#### 📌 記憶體配置說明

**LV_MEM_SIZE = 1MB**:
- 原預設值: 64KB
- 本專案調整為: **1MB (1048576 bytes)**
- 原因:
  1. PC 環境記憶體充足
  2. 支援複雜的 UI (主儀表板 + 設定選單)
  3. 支援 FreeType 動態字體渲染
  4. 支援多個 Widget 同時存在

**記憶體使用預估**:
```
主儀表板:     約 200KB
設定選單:     約 150KB
FreeType 字體: 約 300KB
LVGL 內部:    約 100KB
緩衝區:       約 200KB
------------------------
總計:         約 950KB
```

**LV_MEM_POOL_EXPAND_SIZE = 0**:
- 不使用動態擴展
- 固定 1MB heap，簡化記憶體管理

**LV_MEM_ADR = 0**:
- 使用 LVGL 內建的記憶體分配器
- 不指定外部 SRAM 位址 (嵌入式系統才需要)

---

### 2.3 顯示刷新設定

**位置**: 第 95-99 行

```c
/** Default display refresh, input device read and animation step period. */
#define LV_DEF_REFR_PERIOD  33      /**< [ms] */

/** Default Dots Per Inch. Used to initialize default sizes such as widgets sized,
 *  style paddings. (Not so important, you can adjust it to modify default sizes and spaces.) */
#define LV_DPI_DEF 130              /**< [px/inch] */
```

#### 📌 刷新率設定

**LV_DEF_REFR_PERIOD = 33ms**:
- 刷新週期: 33ms
- **幀率 (FPS)**: 1000ms / 33ms ≈ **30 FPS**
- 適合一般 UI 顯示，流暢度足夠

**可選配置**:
| 設定值 | FPS | 適用場景 |
|--------|-----|---------|
| 50 ms | 20 FPS | 低效能裝置 |
| **33 ms** | **30 FPS** | **一般 UI (本專案)** |
| 16 ms | 60 FPS | 遊戲、動畫 |
| 8 ms | 120 FPS | 高階顯示器 |

**LV_DPI_DEF = 130**:
- 每英寸點數 (Dots Per Inch)
- 影響 Widget 的預設大小
- 130 DPI 適合 PC 顯示器

---

### 2.4 作業系統設定 ⭐⭐

**位置**: 第 104-110 行

```c
/*=================
 * OPERATING SYSTEM
 *=================*/
/** Select operating system to use. Possible options:
 * - LV_OS_NONE
 * - LV_OS_PTHREAD
 * - LV_OS_FREERTOS
 * - LV_OS_CMSIS_RTOS2
 * - LV_OS_RTTHREAD
 * - LV_OS_WINDOWS
 * - LV_OS_CUSTOM
 */
#define LV_USE_OS   LV_OS_NONE  // 預設: 無作業系統
```

#### 📌 作業系統模式選擇

| 模式 | 說明 | 本專案使用 |
|------|------|----------|
| **LV_OS_NONE** | **單執行緒** | **✅ 預設模式** |
| LV_OS_PTHREAD | POSIX 執行緒 | ❌ |
| LV_OS_FREERTOS | FreeRTOS | ✅ 可選 (USE_FREERTOS=ON) |
| LV_OS_CMSIS_RTOS2 | ARM CMSIS | ❌ |
| LV_OS_RTTHREAD | RT-Thread | ❌ |
| LV_OS_WINDOWS | Windows 執行緒 | ❌ |

**切換 FreeRTOS 模式**:
```c
// 方法 1: 修改 lv_conf.h
#define LV_USE_OS  LV_OS_FREERTOS

// 方法 2: CMakeLists.txt
option(USE_FREERTOS "Enable FreeRTOS" ON)

// 方法 3: CMake 命令列
cmake -B build -DUSE_FREERTOS=ON ..
```

---

### 2.5 字體系統設定 ⭐⭐⭐

**位置**: 第 305-390 行 (約)

```c
/*=================
 * FONT USAGE
 *=================*/

/** Enable FreeType font rendering */
#define LV_USE_FREETYPE 1

#if LV_USE_FREETYPE
    /* Memory used by FreeType to cache characters in kilobytes */
    #define LV_FREETYPE_CACHE_SIZE 64

    /* Maximum number of opened FT_Face objects managed by this cache instance */
    #define LV_FREETYPE_CACHE_FT_FACES 8

    /* Maximum number of opened FT_Size objects managed by this cache instance */
    #define LV_FREETYPE_CACHE_FT_SIZES 8

    /* Use lvgl port */
    #define LV_FREETYPE_USE_LVGL_PORT 0
#endif
```

#### 📌 FreeType 設定

**LV_USE_FREETYPE = 1**:
- ✅ **必須啟用** (本專案需求)
- 用途: 載入 TrueType/OpenType 字體 (.ttf, .ttc)
- 支援: Noto Sans CJK (中文字體)

**LV_FREETYPE_CACHE_SIZE = 64KB**:
- FreeType 字元快取大小
- 64KB 足夠快取常用字元
- 減少重複渲染，提升效能

**LV_FREETYPE_CACHE_FT_FACES = 8**:
- 最多同時開啟 8 個字體檔案
- 本專案僅使用 1 個 (Noto Sans CJK)

**LV_FREETYPE_USE_LVGL_PORT = 0**:
- 不使用 LVGL 移植版本的 FreeType
- 直接使用系統安裝的 FreeType 函式庫

---

### 2.6 內建字體設定

**位置**: 第 400-450 行 (約)

```c
/** Enable built-in fonts */
#define LV_FONT_MONTSERRAT_8     0
#define LV_FONT_MONTSERRAT_10    0
#define LV_FONT_MONTSERRAT_12    1  // 啟用 12px
#define LV_FONT_MONTSERRAT_14    0
#define LV_FONT_MONTSERRAT_16    1  // 啟用 16px
#define LV_FONT_MONTSERRAT_18    0
#define LV_FONT_MONTSERRAT_20    0
#define LV_FONT_MONTSERRAT_22    0
#define LV_FONT_MONTSERRAT_24    1  // 啟用 24px
#define LV_FONT_MONTSERRAT_26    0
#define LV_FONT_MONTSERRAT_28    0
#define LV_FONT_MONTSERRAT_30    0
#define LV_FONT_MONTSERRAT_32    0
#define LV_FONT_MONTSERRAT_34    0
#define LV_FONT_MONTSERRAT_36    0
#define LV_FONT_MONTSERRAT_38    0
#define LV_FONT_MONTSERRAT_40    0
#define LV_FONT_MONTSERRAT_42    0
#define LV_FONT_MONTSERRAT_44    0
#define LV_FONT_MONTSERRAT_46    0
#define LV_FONT_MONTSERRAT_48    1  // 啟用 48px

/** Default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_16
```

#### 📌 內建字體使用

**本專案啟用的字體**:
| 字體 | 大小 | 用途 |
|------|------|------|
| lv_font_montserrat_12 | 12px | 小型標籤、提示文字 |
| lv_font_montserrat_16 | 16px | 預設字體、一般文字 |
| lv_font_montserrat_24 | 24px | 標題、按鈕文字 |
| lv_font_montserrat_48 | 48px | 大型數值顯示 (Race 布局時速) |

**LV_FONT_DEFAULT**:
```c
#define LV_FONT_DEFAULT &lv_font_montserrat_16
```
- 預設字體: Montserrat 16px
- 當 Widget 未指定字體時使用
- 作為 FreeType 字體的 fallback

---

### 2.7 Widget 啟用設定

**位置**: 第 500-700 行 (約)

```c
/*==================
 * WIDGETS
 *==================*/

#define LV_USE_ANIMIMG      1   // 動畫圖片
#define LV_USE_ARC          1   // 圓弧/旋鈕 (亮度調整)
#define LV_USE_BAR          1   // 進度條
#define LV_USE_BUTTON       1   // 按鈕
#define LV_USE_BUTTONMATRIX 1   // 按鈕矩陣
#define LV_USE_CANVAS       0   // 畫布 (不使用)
#define LV_USE_CHECKBOX     0   // 核取方塊 (不使用)
#define LV_USE_DROPDOWN     1   // 下拉選單 (ABS 選擇)
#define LV_USE_IMAGE        1   // 圖片
#define LV_USE_IMAGEBUTTON  0   // 圖片按鈕 (不使用)
#define LV_USE_KEYBOARD     0   // 虛擬鍵盤 (不使用)
#define LV_USE_LABEL        1   // 文字標籤
#define LV_USE_LED          0   // LED 指示燈 (不使用)
#define LV_USE_LINE         1   // 線條
#define LV_USE_LIST         1   // 列表 (設定選單)
#define LV_USE_MENU         0   // 選單 (不使用，自訂實作)
#define LV_USE_MSGBOX       1   // 訊息框 (模式變更警告)
#define LV_USE_ROLLER       1   // 滾輪選擇器 (騎乘模式)
#define LV_USE_SCALE        1   // 刻度/儀表 (轉速表)
#define LV_USE_SLIDER       1   // 滑動條 (TCS/油門/剎車)
#define LV_USE_SPAN         0   // 文字片段 (不使用)
#define LV_USE_SPINBOX      0   // 數字輸入框 (不使用)
#define LV_USE_SPINNER      0   // 載入動畫 (不使用)
#define LV_USE_SWITCH       0   // 開關 (不使用)
#define LV_USE_TEXTAREA     0   // 文字區域 (不使用)
#define LV_USE_TABLE        0   // 表格 (不使用)
#define LV_USE_TABVIEW      0   // 分頁檢視 (不使用)
```

#### 📌 本專案使用的 Widget

| Widget | 功能 | 本專案用途 |
|--------|------|----------|
| **Arc** | 圓弧/旋鈕 | 螢幕亮度調整 |
| **Bar** | 進度條 | Race 布局 RPM 顯示 |
| **Button** | 按鈕 | 升檔/降檔/返回/設定 |
| **Dropdown** | 下拉選單 | ABS 模式選擇 |
| **Label** | 文字標籤 | 時速/檔位/RPM 顯示 |
| **Line** | 線條 | 轉速表指針 |
| **List** | 列表 | 設定選單主列表 |
| **Msgbox** | 訊息框 | 模式變更警告 |
| **Roller** | 滾輪選擇器 | 騎乘模式選擇 |
| **Scale** | 刻度/儀表 | 轉速表、時速表 |
| **Slider** | 滑動條 | TCS/油門/剎車調整 |

---

## 3. 與其他檔案的關聯

### 3.1 lv_conf.h → CMakeLists.txt

**CMakeLists.txt 中的設定**:
```cmake
# 告訴編譯器 lv_conf.h 放在專案根目錄
target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)
```

**LVGL 內部會自動引入**:
```c
// lvgl/lvgl.h 內部
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_conf.h"  // 簡單引入，從專案根目錄尋找
#else
#include "../lv_conf.h"  // 相對路徑引入
#endif
```

---

### 3.2 lv_conf.h → main.c

**main.c 中使用組態設定**:
```c
// main.c, 第 83 行
init_chinese_font();  // 此函式依賴 LV_USE_FREETYPE = 1

// main.c, 第 329 行
g_chinese_font = lv_freetype_font_create(...);  // FreeType 必須啟用

// main.c, 第 107 行
uint32_t sleep_time_ms = lv_timer_handler();
if (sleep_time_ms == LV_NO_TIMER_READY)
{
    sleep_time_ms = LV_DEF_REFR_PERIOD;  // 使用組態設定的刷新週期
}
```

---

## 4. 常見問題與除錯

### 4.1 記憶體不足錯誤

**錯誤訊息**:
```
[ERROR] Out of memory while allocating XXX bytes
```

**解決方法**:
```c
// lv_conf.h, 第 77 行
#define LV_MEM_SIZE (2 * 1024 * 1024)  // 增加到 2MB
```

---

### 4.2 FreeType 功能無法使用

**錯誤訊息**:
```
undefined reference to `lv_freetype_font_create'
```

**解決方法**:
```c
// 1. 檢查 lv_conf.h
#define LV_USE_FREETYPE 1  // 確保為 1

// 2. 檢查 CMakeLists.txt
option(LV_USE_FREETYPE "Use freetype library" ON)  // 確保為 ON

// 3. 重新編譯
cmake -B build && cmake --build build
```

---

### 4.3 Widget 功能無法使用

**錯誤訊息**:
```
undefined reference to `lv_slider_create'
```

**解決方法**:
```c
// lv_conf.h, 找到對應的 Widget 設定
#define LV_USE_SLIDER 1  // 確保為 1

// 重新編譯
cmake --build build
```

---

## 5. 效能調校建議

### 5.1 提升 FPS

```c
// 降低刷新週期
#define LV_DEF_REFR_PERIOD  16  // 60 FPS

// 啟用雙緩衝 (如果支援)
#define LV_USE_DRAW_SW_COMPLEX 1
```

---

### 5.2 降低記憶體使用

```c
// 減少記憶體池
#define LV_MEM_SIZE (512 * 1024)  // 512KB

// 關閉不需要的 Widget
#define LV_USE_CANVAS 0
#define LV_USE_CHART 0
```

---

### 5.3 停用不需要的功能

```c
// 停用 Demo
#define LV_BUILD_EXAMPLES 0

// 停用動畫 (提升效能，但失去流暢感)
#define LV_USE_ANIM 0
```

---

## 6. 報告建議

### 6.1 投影片架構

**第 1 張**: lv_conf.h 組態檔概述
- 說明組態檔的作用

**第 2 張**: 記憶體設定
- 展示 1MB heap 設定
- 說明記憶體使用預估

**第 3 張**: FreeType 字體整合
- 說明為何需要 FreeType
- 展示中文字體載入流程

**第 4 張**: Widget 選擇
- 列出本專案使用的 Widget
- 說明為何選擇這些 Widget

---

## 7. 參考資料

- [LVGL 組態文件](https://docs.lvgl.io/master/overview/config.html)
- [lv_conf_template.h](https://github.com/lvgl/lvgl/blob/master/lv_conf_template.h)
- [LVGL Widget 列表](https://docs.lvgl.io/master/widgets/index.html)

---

**檔案版本**: v1.0
**最後更新**: 2025-11-11
