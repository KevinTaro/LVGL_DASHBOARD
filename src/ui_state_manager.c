#include "ui_state_manager.h"
#include <stdlib.h>
#include <stdio.h>

// 全域狀態實例
VehicleState_t g_vehicle_state;

// 主題狀態
bool is_dark_theme = false;

// 數據模擬 Timer
static lv_timer_t *data_sim_timer;

// 控制輸入狀態
typedef struct
{
    int throttle_input; // 0-100 (油門輸入)
    int brake_input;    // 0-100 (剎車輸入)
    int gear_input;     // -1:降檔, 0:保持, 1:升檔
} ControlInput_t;

static ControlInput_t g_control_input = {0, 0, 0};

// 錯誤處理函式
static void log_error(const char *message)
{
    fprintf(stderr, "[ERROR] %s\n", message);
}

// 計算最佳轉速的函式
static int calculate_optimal_rpm(int gear, int speed)
{
    // 根據檔位和時速計算最佳轉速
    // 簡化的摩托車動力學模型
    switch (gear)
    {
    case 1:
        // 1檔：0-30 km/h，最佳轉速 3000-6000 RPM
        if (speed < 10)
            return 3000;
        else if (speed < 20)
            return 4000;
        else
            return 5000;
    case 2:
        // 2檔：20-50 km/h，最佳轉速 4000-7000 RPM
        if (speed < 30)
            return 4500;
        else if (speed < 40)
            return 5500;
        else
            return 6500;
    case 3:
        // 3檔：40-70 km/h，最佳轉速 5000-8000 RPM
        if (speed < 50)
            return 5500;
        else if (speed < 60)
            return 6500;
        else
            return 7500;
    case 4:
        // 4檔：60-90 km/h，最佳轉速 6000-9000 RPM
        if (speed < 70)
            return 6500;
        else if (speed < 80)
            return 7500;
        else
            return 8500;
    case 5:
        // 5檔：80-120 km/h，最佳轉速 7000-10000 RPM
        if (speed < 90)
            return 7500;
        else if (speed < 100)
            return 8500;
        else
            return 9500;
    case 6:
        // 6檔：100+ km/h，最佳轉速 8000-11000 RPM
        if (speed < 110)
            return 8500;
        else if (speed < 130)
            return 9500;
        else
            return 10500;
    default:
        return 5000; // 預設值
    }
}

// 根據時速和檔位反算RPM的函式
static int calculate_rpm_from_speed(int gear, int speed)
{
    // 使用相同的gear_ratio公式反算RPM
    float gear_ratio = 1.0f + (gear - 1) * 0.3f;
    int rpm = (int)(speed * (150.0f / gear_ratio));

    // 確保RPM在合理範圍內
    if (rpm < 1000)
        rpm = 1000; // 最低怠速
    if (rpm > 13000)
        rpm = 13000; // 最高轉速限制

    return rpm;
}

static void validate_vehicle_state(void)
{
    // 驗證RPM範圍
    if (g_vehicle_state.current_rpm < 0 || g_vehicle_state.current_rpm > 15000)
    {
        log_error("Invalid RPM value detected, resetting to safe value");
        g_vehicle_state.current_rpm = 0;
        g_vehicle_state.target_rpm = 0;
    }

    // 驗證時速範圍
    if (g_vehicle_state.current_speed < 0 || g_vehicle_state.current_speed > 250)
    {
        log_error("Invalid speed value detected, resetting to safe value");
        g_vehicle_state.current_speed = 0;
    }

    // 驗證檔位範圍
    if (g_vehicle_state.current_gear < 1 || g_vehicle_state.current_gear > 6)
    {
        log_error("Invalid gear value detected, resetting to neutral");
        g_vehicle_state.current_gear = 1;
    }

    // 驗證騎乘模式
    if (g_vehicle_state.riding_mode < 0 || g_vehicle_state.riding_mode > 5)
    {
        log_error("Invalid riding mode detected, resetting to Street");
        g_vehicle_state.riding_mode = 0;
    }

    // 驗證TCS/ABS等級
    if (g_vehicle_state.tcs_level < 0 || g_vehicle_state.tcs_level > 15)
    {
        log_error("Invalid TCS level detected, resetting to 5");
        g_vehicle_state.tcs_level = 5;
    }

    if (g_vehicle_state.abs_level < 0 || g_vehicle_state.abs_level > 3)
    {
        log_error("Invalid ABS level detected, resetting to 1");
        g_vehicle_state.abs_level = 1;
    }

    // 驗證亮度
    if (g_vehicle_state.brightness < 0 || g_vehicle_state.brightness > 100)
    {
        log_error("Invalid brightness value detected, resetting to 80");
        g_vehicle_state.brightness = 80;
    }
}

// 數據模擬回呼函式
static void data_simulator_cb(lv_timer_t *timer)
{
    // 應用控制輸入
    if (g_control_input.throttle_input > 0)
    {
        // 油門控制：根據輸入增加RPM
        int rpm_increase = g_control_input.throttle_input * 25; // 每100ms最多增加500 RPM
        g_vehicle_state.target_rpm += rpm_increase;
    }
    else if (g_control_input.brake_input > 0)
    {
        // 剎車控制：根據輸入減少RPM
        int rpm_decrease = g_control_input.brake_input * 30; // 每100ms最多減少3000 RPM
        g_vehicle_state.target_rpm -= rpm_decrease;
        if (g_vehicle_state.target_rpm < 0)
            g_vehicle_state.target_rpm = 0;
    }
    else
    {
        // 無輸入時緩慢減速
        g_vehicle_state.target_rpm -= 100;
        if (g_vehicle_state.target_rpm < 0)
            g_vehicle_state.target_rpm = 0;
    }

    // 處理換檔輸入
    if (g_control_input.gear_input != 0)
    {
        if (g_control_input.gear_input > 0 && g_vehicle_state.current_gear < 6)
        {
            // 升檔：保持當前時速，計算對應的新RPM
            g_vehicle_state.current_gear++;
            // 根據新檔位和當前時速反算RPM
            g_vehicle_state.target_rpm = calculate_rpm_from_speed(g_vehicle_state.current_gear, g_vehicle_state.current_speed);
        }
        else if (g_control_input.gear_input < 0 && g_vehicle_state.current_gear > 1)
        {
            // 降檔：保持當前時速，計算對應的新RPM
            g_vehicle_state.current_gear--;
            // 根據新檔位和當前時速反算RPM
            g_vehicle_state.target_rpm = calculate_rpm_from_speed(g_vehicle_state.current_gear, g_vehicle_state.current_speed);
        }
        g_control_input.gear_input = 0; // 重置輸入
    }

    // 根據 RPM 和檔位計算時速 (雙向計算)
    float gear_ratio = 1.0f + (g_vehicle_state.current_gear - 1) * 0.3f;
    g_vehicle_state.current_speed = (int)(g_vehicle_state.target_rpm / (150.0f / gear_ratio));

    // 限制最大值
    if (g_vehicle_state.target_rpm > 13000)
    {
        g_vehicle_state.target_rpm = 13000;
    }
    if (g_vehicle_state.current_speed > 200)
    {
        g_vehicle_state.current_speed = 200;
    }

    // 驗證狀態完整性
    validate_vehicle_state();
}

// 初始化狀態管理器
void init_state_manager(void)
{
    // 初始化狀態
    g_vehicle_state.current_rpm = 0;
    g_vehicle_state.target_rpm = 0;
    g_vehicle_state.current_speed = 0;
    g_vehicle_state.current_gear = 1;
    g_vehicle_state.riding_mode = 2; // normal (預設模式)
    // 使用 normal 模式的預設值初始化 TCS 和 ABS
    g_vehicle_state.tcs_level = riding_mode_defaults[2].tcs_level; // normal: TCS 7
    g_vehicle_state.abs_level = riding_mode_defaults[2].abs_level; // normal: ABS 1 (全開)
    g_vehicle_state.brightness = 100;
    g_vehicle_state.language = 0;                               // 預設中文
    g_vehicle_state.font_size = 16;                             // 預設字體大小
    g_vehicle_state.dashboard_layout = DASHBOARD_LAYOUT_NORMAL; // 預設雙環布局

    // 重置控制輸入
    g_control_input.throttle_input = 0;
    g_control_input.brake_input = 0;
    g_control_input.gear_input = 0;

    // 建立數據模擬 Timer (100ms - 調整為較慢以減少負擔)
    data_sim_timer = lv_timer_create(data_simulator_cb, 100, NULL);
    if (data_sim_timer == NULL)
    {
        log_error("Failed to create data simulation timer");
        return;
    }

    printf("[INFO] State manager initialized successfully\n");
}

// 更新狀態 (可由外部呼叫)
void update_vehicle_state(void)
{
    // 這裡可以添加額外的狀態更新邏輯
}

// 控制輸入函式實現
void set_throttle_input(int throttle)
{
    if (throttle < 0)
    {
        log_error("Throttle input below minimum (0), clamping to 0");
        throttle = 0;
    }
    if (throttle > 100)
    {
        log_error("Throttle input above maximum (100), clamping to 100");
        throttle = 100;
    }
    g_control_input.throttle_input = throttle;
}

void set_brake_input(int brake)
{
    if (brake < 0)
    {
        log_error("Brake input below minimum (0), clamping to 0");
        brake = 0;
    }
    if (brake > 100)
    {
        log_error("Brake input above maximum (100), clamping to 100");
        brake = 100;
    }
    g_control_input.brake_input = brake;
}

void shift_gear(int direction)
{
    if (direction != -1 && direction != 1)
    {
        log_error("Invalid gear shift direction, must be -1 or 1");
        return;
    }

    // 檢查是否可以換檔
    if (direction > 0 && g_vehicle_state.current_gear >= 6)
    {
        log_error("Cannot shift up, already in highest gear");
        return;
    }
    if (direction < 0 && g_vehicle_state.current_gear <= 1)
    {
        log_error("Cannot shift down, already in lowest gear");
        return;
    }

    g_control_input.gear_input = direction;
}

// 獲取狀態函式
int get_current_rpm(void)
{
    return g_vehicle_state.current_rpm;
}

int get_current_speed(void)
{
    return g_vehicle_state.current_speed;
}

int get_current_gear(void)
{
    return g_vehicle_state.current_gear;
}

// 獲取控制輸入狀態函式
int get_throttle_input(void)
{
    return g_control_input.throttle_input;
}

int get_brake_input(void)
{
    return g_control_input.brake_input;
}

// 儀表板布局管理函式
void set_dashboard_layout(DashboardLayout_t layout)
{
    if (layout < DASHBOARD_LAYOUT_NORMAL || layout > DASHBOARD_LAYOUT_OFFROAD)
    {
        log_error("Invalid dashboard layout, ignoring");
        return;
    }
    g_vehicle_state.dashboard_layout = layout;
    printf("[INFO] Dashboard layout changed to %d\n", layout);
}

DashboardLayout_t get_dashboard_layout(void)
{
    return g_vehicle_state.dashboard_layout;
}
