#ifndef UI_STATE_MANAGER_H
#define UI_STATE_MANAGER_H

#include "../lvgl/lvgl.h"

// 儀表板布局類型
typedef enum
{
    DASHBOARD_LAYOUT_NORMAL = 0,  // 雙環布局（Normal, Rain, Supermoto）
    DASHBOARD_LAYOUT_RACE = 1,    // 賽道布局（Race）
    DASHBOARD_LAYOUT_OFFROAD = 2, // 越野布局（Offroad）
} DashboardLayout_t;

// 車輛狀態結構體
typedef struct
{
    int current_rpm;
    int target_rpm;
    int current_speed;
    int current_gear;
    int riding_mode;                    // 0:Race, 1:Rain, 2:Normal, 3:Offroad, 4:Supermoto, 5:自訂
    int tcs_level;                      // 0-15
    int abs_level;                      // 0:Offroad, 1:全開, 2:後輪關閉, 3:前後輪關閉
    int brightness;                     // 0-100
    int language;                       // 0:中文, 1:English
    int font_size;                      // 字體大小
    DashboardLayout_t dashboard_layout; // 當前儀表板布局
} VehicleState_t;

// 騎乘模式預設值結構體
typedef struct
{
    int tcs_level;
    int abs_level;
} RidingModeDefaults_t;

// 各模式的預設值 (不可修改)
// ABS: 0=Offroad, 1=全開, 2=後輪關閉, 3=前後輪關閉
static const RidingModeDefaults_t riding_mode_defaults[6] = {
    {1, 3},  // race: TCS 1, ABS 前後輪關閉
    {15, 1}, // rain: TCS 15, ABS 全開
    {7, 1},  // normal: TCS 7, ABS 全開
    {3, 0},  // offroad: TCS 3, Offroad ABS
    {0, 2},  // supermoto: TCS 0, ABS 後輪關閉
    {7, 1}   // 自訂 (預設值，實際會被用戶覆蓋)
};

// 全域狀態實例
extern VehicleState_t g_vehicle_state;

// 主題狀態
extern bool is_dark_theme;

// 初始化函式
void init_state_manager(void);

// 更新狀態函式
void update_vehicle_state(void);

// 控制輸入函式
void set_throttle_input(int throttle); // 0-100
void set_brake_input(int brake);       // 0-100
void shift_gear(int direction);        // -1:降檔, 1:升檔

// 獲取狀態函式
int get_current_rpm(void);
int get_current_speed(void);
int get_current_gear(void);

// 獲取控制輸入狀態函式
int get_throttle_input(void);
int get_brake_input(void);

// 儀表板布局管理
void set_dashboard_layout(DashboardLayout_t layout);
DashboardLayout_t get_dashboard_layout(void);

#endif // UI_STATE_MANAGER_H
