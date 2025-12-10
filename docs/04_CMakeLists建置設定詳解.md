# CMakeLists.txt 與專案建置設定詳解

## 檔案資訊
- **檔案位置**: `/home/sshuser/LVGL/CMakeLists.txt`
- **總行數**: 177 行
- **功能**: CMake 建置系統設定檔
- **編譯工具**: GCC (Linux), MSVC (Windows 可選)

---

## 1. 組態檔功能說明

### 1.1 核心功能
這是專案的**建置核心設定檔**，負責：

1. ✅ **編譯器設定** - C99/C++17 標準
2. ✅ **相依性管理** - SDL2、FreeType、FreeRTOS
3. ✅ **原始檔管理** - 自動偵測並編譯所有模組
4. ✅ **條件編譯** - 支援 FreeRTOS 可選功能
5. ✅ **最佳化選項** - Debug/Release 模式切換
6. ✅ **跨平台支援** - Linux/Windows/macOS

### 1.2 是否為原創設定
**🔄 基於官方範例修改 (約 70% 修改)**：
- 🆕 新增自定義原始檔 (ui_*.c)
- 🆕 新增 FreeType 字體支援
- 🆕 調整編譯器警告選項
- 🆕 新增除錯符號與最佳化選項
- 📦 保留基本 LVGL 整合架構

---

## 2. CMakeLists.txt 結構解析

### 2.1 整體架構
```
CMakeLists.txt
├── 專案基本設定 (第 1-5 行)
├── FreeRTOS 設定 (第 7-34 行)
├── LVGL 選項設定 (第 39-44 行)
├── 編譯標準設定 (第 47-49 行)
├── SDL2 整合 (第 55 行)
├── LVGL 子專案 (第 64-65 行)
├── 原始檔清單 (第 67-81 行)
├── 可執行檔建立 (第 125-127 行)
└── 最佳化選項 (第 141-177 行)
```

---

## 3. 重點設定解析

### 3.1 專案基本設定

**位置**: 第 1-5 行

```cmake
cmake_minimum_required(VERSION 3.12.4)
project(lvgl C CXX)

# Set the correct FreeRTOS port for your system (e.g., Posix for WSL)
set(FREERTOS_PORT GCC_POSIX CACHE STRING "Port for FreeRTOS on Posix environment")
```

#### 📌 說明

**CMake 版本要求**: 3.12.4+
- 支援現代 CMake 功能
- 支援 target-based 設定

**專案語言**: C + C++
- C: LVGL 和主要邏輯程式碼
- C++: FreeRTOS 部分支援檔案

**FreeRTOS 移植設定**:
```cmake
set(FREERTOS_PORT GCC_POSIX CACHE STRING ...)
```
- `GCC_POSIX`: 使用 POSIX 執行緒模擬 FreeRTOS
- 適用於 Linux/WSL 環境

---

### 3.2 FreeRTOS 條件編譯 ⭐

**位置**: 第 7-34 行

```cmake
option(USE_FREERTOS "Enable FreeRTOS" OFF) # 預設關閉

if(USE_FREERTOS)
    message(STATUS "FreeRTOS is enabled")

    # FreeRTOS 組態介面
    add_library(freertos_config INTERFACE)
    target_include_directories(freertos_config SYSTEM
        INTERFACE ${PROJECT_SOURCE_DIR}/config)
    target_compile_definitions(freertos_config
        INTERFACE projCOVERAGE_TEST=0)

    # 新增 FreeRTOS 子專案
    add_subdirectory(FreeRTOS)

    # FreeRTOS 標頭檔路徑
    include_directories(${PROJECT_SOURCE_DIR}/FreeRTOS/include)
    include_directories(${PROJECT_SOURCE_DIR}/FreeRTOS/portable/ThirdParty/GCC/Posix)
    include_directories(${PROJECT_SOURCE_DIR}/config)

    # 收集 FreeRTOS 原始檔
    file(GLOB FREERTOS_SOURCES
        "${PROJECT_SOURCE_DIR}/FreeRTOS/*.c"
        "${PROJECT_SOURCE_DIR}/FreeRTOS/portable/MemMang/heap_4.c"
        "${PROJECT_SOURCE_DIR}/FreeRTOS/portable/ThirdParty/GCC/Posix/*.c"
    )
else()
    message(STATUS "FreeRTOS is disabled")
    set(FREERTOS_SOURCES "")
endif()
```

#### 📌 啟用方式

**方法 1: CMake 命令列參數**
```bash
cmake -B build -DUSE_FREERTOS=ON
cmake --build build
```

**方法 2: 修改 CMakeLists.txt**
```cmake
option(USE_FREERTOS "Enable FreeRTOS" ON)  # 改為 ON
```

**方法 3: lv_conf.h 設定**
```c
#define LV_USE_OS  LV_OS_FREERTOS  // 或 LV_OS_NONE
```

#### 📌 條件編譯效果

| USE_FREERTOS | 編譯的主檔案 | 執行模式 |
|--------------|-------------|---------|
| OFF (預設) | `src/main.c` | 單執行緒 + LVGL Timer |
| ON | `src/freertos_main.c` | 多執行緒 + FreeRTOS Tasks |

---

### 3.3 LVGL 功能選項

**位置**: 第 39-44 行

```cmake
# Define options for LVGL with default values (OFF)
option(LV_USE_DRAW_SDL "Use SDL draw unit" OFF)
option(LV_USE_LIBPNG "Use libpng to decode PNG" OFF)
option(LV_USE_LIBJPEG_TURBO "Use libjpeg turbo to decode JPEG" OFF)
option(LV_USE_FFMPEG "Use libffmpeg to display video using lv_ffmpeg" OFF)
option(LV_USE_FREETYPE "Use freetype library" ON)  # 預設開啟！
```

#### 📌 選項說明

| 選項 | 預設 | 功能 | 本專案使用 |
|------|------|------|----------|
| LV_USE_DRAW_SDL | OFF | SDL 硬體加速繪圖 | ❌ 不需要 |
| LV_USE_LIBPNG | OFF | PNG 圖片解碼 | ❌ 本專案無圖片 |
| LV_USE_LIBJPEG_TURBO | OFF | JPEG 圖片解碼 | ❌ 本專案無圖片 |
| LV_USE_FFMPEG | OFF | 影片播放支援 | ❌ 不需要 |
| **LV_USE_FREETYPE** | **ON** | **TrueType 字體渲染** | ✅ **必須開啟** |

**FreeType 為何必須？**
```c
// 在 main.c 中需要載入 .ttc 字體檔
g_chinese_font = lv_freetype_font_create(
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
    ...
);
```

---

### 3.4 編譯標準設定

**位置**: 第 47-49 行

```cmake
# Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

#### 📌 標準說明

**C99 標準**:
- 支援 `//` 註解
- 支援 inline 函式
- 支援變長陣列 (VLA) - 雖然本專案未使用

**C++17 標準**:
- 用於 FreeRTOS 的部分實作
- 本專案主要邏輯使用 C，不依賴 C++ 功能

---

### 3.5 原始檔清單 ⭐

**位置**: 第 67-81 行

```cmake
set(MAIN_SOURCES
    src/mouse_cursor_icon.c
    src/hal/hal.c
    src/ui_main_dashboard.c
    src/ui_state_manager.c
    src/ui_settings_menu.c
    src/ui_dashboard_layouts.c
)

set(MAIN_LIBS
    lvgl
    lvgl::examples
    lvgl::demos
    lvgl::thorvg
    ${SDL2_LIBRARIES}
)

# 根據 FreeRTOS 選項決定主檔案
if(USE_FREERTOS)
    list(APPEND MAIN_SOURCES src/freertos_main.c src/freertos/freertos_posix_port.c ${FREERTOS_SOURCES})
    list(APPEND MAIN_LIBS freertos_config freertos_kernel)
else()
    list(APPEND MAIN_SOURCES src/main.c)
endif()
```

#### 📌 原始檔分類

**必編譯檔案** (MAIN_SOURCES):
```
src/mouse_cursor_icon.c      ← 游標圖示資料
src/hal/hal.c                 ← SDL2 硬體抽象層
src/ui_main_dashboard.c       ← 主儀表板 UI
src/ui_state_manager.c        ← 狀態管理系統
src/ui_settings_menu.c        ← 設定選單 UI
src/ui_dashboard_layouts.c    ← 儀表板布局系統
```

**條件編譯檔案**:
```
USE_FREERTOS=OFF:
  src/main.c                  ← 單執行緒主程式

USE_FREERTOS=ON:
  src/freertos_main.c         ← FreeRTOS 主程式
  src/freertos/...            ← FreeRTOS POSIX 移植
  FreeRTOS/*.c                ← FreeRTOS 核心原始碼
```

**連結函式庫** (MAIN_LIBS):
```
lvgl              ← LVGL 核心
lvgl::examples    ← LVGL 官方範例
lvgl::demos       ← LVGL 官方 Demo
SDL2              ← SDL2 顯示後端
FreeType          ← 字體渲染 (條件加入)
pthread           ← POSIX 執行緒 (Linux)
```

---

### 3.6 可執行檔建立

**位置**: 第 125-143 行

```cmake
add_executable(main ${MAIN_SOURCES})
target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(main ${MAIN_LIBS})

# On Windows, GUI applications do not show a console by default,
# which hides log output. This ensures a console is available for logging.
if (WIN32)
    if (MSVC)
        target_link_options(main PRIVATE "/SUBSYSTEM:CONSOLE")
    else()
        target_link_options(main PRIVATE "-mconsole")
    endif()
endif()
```

#### 📌 設定說明

**可執行檔名稱**: `main`
- 輸出路徑: `bin/main`

**編譯定義**:
```cmake
target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)
```
- 告訴 LVGL 使用簡化的組態檔引入方式
- 對應 `lv_conf.h` 放在專案根目錄

**Windows 主控台支援**:
- 確保在 Windows 也能看到 `printf()` 輸出
- 方便除錯

---

### 3.7 Debug 模式最佳化選項 ⭐⭐

**位置**: 第 145-177 行

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode enabled")

    target_compile_options(lvgl PRIVATE
        -pedantic-errors
        -Wall
        -Wclobbered
        -Wdeprecated
        -Wdouble-promotion
        -Wempty-body
        -Wextra
        -Wformat-security
        -Wmaybe-uninitialized
        -Wpointer-arith
        -Wredundant-decls
        -Wshadow
        -Wtype-limits
        -Wundef
        -Wuninitialized
        -Wunreachable-code
        -Wfloat-conversion
        -Wstrict-aliasing
        -g3                    # 最高等級除錯符號
        -O0                    # 無最佳化，方便除錯
    )

    # 記憶體檢查工具 (Linux)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_options(lvgl PRIVATE
            -fsanitize=address         # 記憶體位址檢查
            -fsanitize=leak            # 記憶體洩漏檢查
            -fstack-protector-strong   # 堆疊溢位保護
        )
        target_link_options(lvgl PRIVATE
            -fsanitize=address
            -fsanitize=leak
        )
    endif()
```

#### 📌 編譯選項說明

**警告選項** (找出潛在 Bug):
| 選項 | 功能 |
|------|------|
| `-Wall -Wextra` | 開啟所有常見警告 |
| `-Wshadow` | 偵測變數名稱遮蔽 |
| `-Wuninitialized` | 偵測未初始化變數 |
| `-Wunreachable-code` | 偵測永不執行的程式碼 |
| `-Wfloat-conversion` | 偵測浮點數轉換精度損失 |

**除錯選項**:
| 選項 | 功能 |
|------|------|
| `-g3` | 產生最詳細的除錯符號 (包含宏定義) |
| `-O0` | 關閉所有最佳化，保持程式碼與原始碼一致 |

**記憶體檢查工具** (僅 Linux):
| 選項 | 功能 | 偵測內容 |
|------|------|---------|
| `-fsanitize=address` | AddressSanitizer | 記憶體越界、Use-after-free |
| `-fsanitize=leak` | LeakSanitizer | 記憶體洩漏 |
| `-fstack-protector-strong` | Stack Protector | 堆疊緩衝區溢位 |

---

## 4. 建置流程

### 4.1 完整建置步驟

#### 方法 1: 使用 CMake (推薦)

```bash
# 步驟 1: 建立 build 目錄
mkdir -p build
cd build

# 步驟 2: 產生 Makefile (Debug 模式)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 步驟 3: 編譯
cmake --build . -j$(nproc)

# 步驟 4: 執行
cd ..
./bin/main
```

#### 方法 2: 啟用 FreeRTOS

```bash
mkdir -p build
cd build

# 啟用 FreeRTOS
cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_FREERTOS=ON ..

cmake --build . -j$(nproc)
cd ..
./bin/main
```

#### 方法 3: Release 模式 (最佳化效能)

```bash
mkdir -p build
cd build

# Release 模式：開啟 -O3 最佳化
cmake -DCMAKE_BUILD_TYPE=Release ..

cmake --build . -j$(nproc)
cd ..
./bin/main
```

---

### 4.2 建置流程圖

```
開始
  │
  ▼
清理舊建置檔
rm -rf build
  │
  ▼
建立 build 目錄
mkdir build && cd build
  │
  ▼
CMake 產生 Makefile
cmake -DCMAKE_BUILD_TYPE=Debug ..
  │
  ├─> 檢查相依性 (SDL2, FreeType)
  ├─> 設定編譯選項
  ├─> 產生編譯規則
  └─> 輸出: Makefile
  │
  ▼
執行編譯
make -j8
  │
  ├─> 編譯 LVGL 函式庫
  ├─> 編譯專案原始檔
  ├─> 連結所有 .o 檔案
  └─> 輸出: bin/main
  │
  ▼
執行程式
./bin/main
  │
  └─> SDL2 視窗顯示
```

---

## 5. VS Code 整合

### 5.1 tasks.json 設定

**位置**: `.vscode/tasks.json`

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake: Configure",
            "type": "shell",
            "command": "cmake",
            "args": [
                "-B", "build",
                "-DCMAKE_BUILD_TYPE=Debug"
            ],
            "group": {
                "kind": "build",
                "isDefault": false
            }
        },
        {
            "label": "CMake: Build",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build", "build",
                "-j8"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "dependsOn": ["CMake: Configure"]
        }
    ]
}
```

### 5.2 launch.json 設定

**位置**: `.vscode/launch.json`

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug LVGL demo with gdb",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/bin/main",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "preLaunchTask": "CMake: Build"
        }
    ]
}
```

---

## 6. 常見問題

### 6.1 SDL2 找不到

**錯誤訊息**:
```
CMake Error: Could NOT find SDL2 (missing: SDL2_LIBRARIES SDL2_INCLUDE_DIRS)
```

**解決方法**:
```bash
# Debian/Ubuntu
sudo apt install libsdl2-dev

# macOS
brew install sdl2

# 驗證安裝
sdl2-config --version
```

---

### 6.2 FreeType 找不到

**錯誤訊息**:
```
CMake Error: Could NOT find Freetype (missing: FREETYPE_LIBRARY FREETYPE_INCLUDE_DIRS)
```

**解決方法**:
```bash
# Debian/Ubuntu
sudo apt install libfreetype6-dev

# macOS
brew install freetype

# 驗證安裝
pkg-config --modversion freetype2
```

---

### 6.3 中文字體無法載入

**錯誤訊息**:
```
[ERROR] Failed to create Chinese font with FreeType
```

**解決方法**:
```bash
# 安裝 Noto Sans CJK 字體
sudo apt install fonts-noto-cjk

# 確認字體檔案存在
ls /usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc
```

---

## 7. 報告建議

### 7.1 投影片架構

**第 1 張**: CMake 建置系統概述
- 說明 CMake 的用途與優勢

**第 2 張**: 專案相依性
- 展示 SDL2、LVGL、FreeType 的關係圖

**第 3 張**: 條件編譯機制
- 展示 FreeRTOS 開關的實作方式

**第 4 張**: 建置流程圖
- 從 CMake 到執行檔的完整流程

---

## 8. 參考資料

- [CMake 官方文件](https://cmake.org/documentation/)
- [Modern CMake 教學](https://cliutils.gitlab.io/modern-cmake/)
- [SDL2 CMake 整合](https://wiki.libsdl.org/SDL2/Installation)

---

**檔案版本**: v1.0
**最後更新**: 2025-11-11
