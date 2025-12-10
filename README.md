# 🚀 期末專題：摩托車儀表模擬器
# Final Project: Motorcycle Dashboard Simulator

[![LVGL](https://img.shields.io/badge/LVGL-v9.4.0--dev-blue)](https://github.com/lvgl/lvgl)
[![CMake](https://img.shields.io/badge/CMake-3.12+-green)](https://cmake.org/)
[![SDL2](https://img.shields.io/badge/SDL2-2.0+-red)](https://www.libsdl.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow)](LICENSE)

## 📋 專題概述

本專題基於 **LVGL (Light and Versatile Graphics Library)** 開發的摩托車儀表模擬器，實現完整的車載資訊顯示系統。專題結合嵌入式系統概念與現代圖形介面技術，展示摩托車儀表板的各項功能。

### 🎯 主要功能

- **即時儀表顯示**：速度、轉速、油位、溫度等關鍵指標
- **動態UI動畫**：流暢的指針動畫和狀態轉換
- **設定選單系統**：可自定義的參數調整介面
- **中文字體支援**：完整的中文介面顯示
- **實時狀態模擬**：物理引擎驅動的動態數據
- **跨平台支援**：Windows、Linux、macOS

### 🏗️ 技術架構

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   LVGL v9.4     │    │     SDL2        │    │   FreeType      │
│  Graphics Core  │◄──►│ Display Backend │◄──►│  Font Engine    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         ▲                       ▲                       ▲
         │                       │                       │
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ State Manager   │    │   UI Dashboard  │    │ Settings Menu   │
│ 物理引擎        │    │ 儀表介面        │    │ 設定系統        │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 🚀 快速開始

### 環境需求

- **作業系統**：Linux Ubuntu 18.04+ / Windows 10+ / macOS 10.15+
- **編譯器**：GCC 7.0+ / Clang 6.0+ / MSVC 2017+
- **建置工具**：CMake 3.12+
- **依賴套件**：
  - SDL2 2.0+
  - FreeType 2.8+
  - CMake 3.12+

### 安裝依賴套件

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libfreetype6-dev
```

#### Windows (vcpkg)
```bash
vcpkg install sdl2 freetype
```

#### macOS (Homebrew)
```bash
brew install sdl2 freetype cmake
```

### 建置專題

```bash
# 1. 建立建置目錄
mkdir build && cd build

# 2. 配置 CMake
cmake ..

# 3. 編譯專題
make -j$(nproc)

# 4. 執行模擬器
./bin/main
```

### VS Code 開發環境

1. 開啟專題：雙擊 `simulator.code-workspace`
2. 安裝推薦擴充套件
3. 按 F5 或使用除錯面板啟動

## 📁 專題結構

```
LVGL/
├── src/                    # 核心源碼
│   ├── main.c             # 主程式入口
│   ├── hal/               # 硬體抽象層
│   │   ├── hal.c
│   │   └── hal.h
│   ├── ui_*.c/.h          # UI 模組
│   └── freertos/          # FreeRTOS 移植
├── docs/                   # 技術文檔 (中文)
│   ├── 00_文檔導覽.md
│   ├── 01_專案總覽與設計方法.md
│   ├── 02_main.c_詳解.md
│   └── ...
├── CMakeLists.txt         # 建置配置
├── lv_conf.h             # LVGL 配置
├── .gitignore            # Git 忽略規則
└── README.md             # 本文件
```

## 🎮 使用說明

### 基本操作

- **滑鼠**：點擊按鈕和控制項
- **鍵盤**：ESC 退出程式
- **視窗**：800x600 解析度

### 功能模組

1. **主儀表板**：顯示所有關鍵指標
2. **設定選單**：調整系統參數
3. **狀態管理**：實時數據更新

## 📚 技術文檔

詳細的技術文檔位於 `docs/` 目錄：

- [📋 文檔導覽](docs/00_文檔導覽.md)
- [🏗️ 專案總覽](docs/01_專案總覽與設計方法.md)
- [⚙️ 程式碼詳解](docs/02_main.c_詳解.md)
- [🔧 建置設定](docs/04_CMakeLists建置設定詳解.md)
- [📖 完整功能說明](docs/05_完整功能說明與程式改動對照.md)

## 🛠️ 開發與除錯

### 建置選項

```bash
# 除錯模式 (預設)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 發行模式
cmake -DCMAKE_BUILD_TYPE=Release ..

# 啟用 AddressSanitizer
cmake -DENABLE_ASAN=ON ..
```

### 程式碼風格

- 遵循 C99 標準
- 使用 4 空格縮排
- 函數註釋使用 Doxygen 格式
- 變數命名使用 snake_case

## 🤝 貢獻指南

1. Fork 此專題
2. 建立功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交變更 (`git commit -m 'Add some AmazingFeature'`)
4. 推送分支 (`git push origin feature/AmazingFeature`)
5. 開啟 Pull Request

## 📄 授權條款

本專題採用 MIT 授權條款 - 詳見 [LICENSE](LICENSE) 文件

## 🙏 致謝

- [LVGL](https://github.com/lvgl/lvgl) - 輕量級圖形庫
- [SDL2](https://www.libsdl.org/) - 跨平台多媒體庫
- [FreeType](https://www.freetype.org/) - 字體渲染引擎

---

**🏫 國立台東大學 嵌入式系統軟體技術 期末專題**

**📅 開發期間：2025年**

**👨‍💻 開發者：學生專題作品**

to build using the latest version of clang from homebrew, do the following:

1. `brew install llvm`

2. cmd+shift+p and run `Cmake: select a kit`, then `[Scan for kits]`

3. then cmd+shift+p and run `Cmake: select a kit`, select the version of clang you just installed from homebrew (it should say `Using compilers C=/opt/homebrew/opt/llvm/bin/clang ...`)

4. reconfigure by running cmd+shift+p `Cmake: Configure`

5. build using [step 4 above](#visual-studio-code)

### FreeRTOS configuration
To correctly configure the project, the RTOS (Real-Time Operating System) requires a significant amount of heap memory, especially when debugging an SDL (Simple DirectMedia Layer) window application. In this project, the heap memory has been experimentally set to **512 MB**.

```c
#define configTOTAL_HEAP_SIZE ( ( size_t ) ( 512 * 1024 * 1024 ) )  // 512 MB Heap
```
This configuration ensures that the SDL window is displayed in a timely manner. If this value is reduced, it may cause significant delays in the SDL window's appearance. If the allocated heap memory is too small, the window may fail to appear altogether.
Therefore, it is crucial to allocate sufficient heap memory to ensure smooth execution and debugging experience.

### Enable FreeRTOS
To enable the rtos part of this project select in lv_conf.h `#define LV_USE_OS   LV_OS_NONE` to `#define LV_USE_OS  LV_OS_FREERTOS`
Additionaly you have to enable the compilation of all FreeRTOS Files by turning on the `option(USE_FREERTOS "Enable FreeRTOS" OFF)` in the CMakeLists.txt file or
by enabling the same flag from the command line when bootstrapping `cmake`:

```bash
cmake -B build -DUSE_FREERTOS=ON
```

### CMake

This project uses CMake under the hood which can be used without Visula Studio Code too. Just type these in a Terminal when you are in the project's root folder:

```bash
mkdir build
cd build
cmake ..
make -j
```

## Run demos and examples

By default, the widgets demo (`lv_demo_widgets()`) will run. If you want to run a different demo or example from the LVGL library,
simply replace the demo function call in the code with another one—such as `lv_demo_benchmark()` or `lv_example_label_1()`.

```c
int main(int argc, char **argv)
{
  /* ... */
  /* Run the default demo */
  /* To try a different demo or example, replace this with one of: */
  /* - lv_demo_benchmark(); */
  /* - lv_demo_stress(); */
  /* - lv_example_label_1(); */
  /* - etc. */
  lv_demo_widgets();

  while(1) {
      /* ... */
  }
  return 0;
}
```

## Optional library

There are also FreeType and FFmpeg support. You can install these according to the followings:

### Linux

```bash
# FreeType support
wget https://kumisystems.dl.sourceforge.net/project/freetype/freetype2/2.13.2/freetype-2.13.2.tar.xz
tar -xf freetype-2.13.2.tar.xz
cd freetype-2.13.2
make
make install
```

```bash
# FFmpeg support
git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg
cd ffmpeg
git checkout release/6.0
./configure --disable-all --disable-autodetect --disable-podpages --disable-asm --enable-avcodec --enable-avformat --enable-decoders --enable-encoders --enable-demuxers --enable-parsers --enable-protocol='file' --enable-swscale --enable-zlib
make
sudo make install
```
### (RT)OS support
Works with any OS like pthred, Windows, FreeRTOS, etc. It has build in support for FreeRTOS.

## Test
This project is configured for [VSCode](https://code.visualstudio.com) and is tested on:
- Ubuntu Linux
- Windows WSL (Ubuntu Linux)

It requires a working version of GCC, GDB and make in your path.

To allow debugging inside VSCode you will also require a GDB [extension](https://marketplace.visualstudio.com/items?itemName=webfreak.debug) or other suitable debugger. All the requirements, build and debug settings have been pre-configured in the [.workspace](simulator.code-workspace) file.

The project can use **SDL** but it can be easily relaced by any other built-in LVGL dirvers.
