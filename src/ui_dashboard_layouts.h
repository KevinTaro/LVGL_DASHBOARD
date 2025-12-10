#ifndef UI_DASHBOARD_LAYOUTS_H
#define UI_DASHBOARD_LAYOUTS_H

#include "../lvgl/lvgl.h"
#include "ui_state_manager.h"

/**
 * @brief 創建指定類型的儀表板布局
 * @param parent 父容器物件
 * @param layout 布局類型 (Normal/Race/Offroad)
 */
void create_dashboard_layout(lv_obj_t *parent, DashboardLayout_t layout);

/**
 * @brief 更新當前儀表板布局的資料顯示
 */
void update_dashboard_layout(void);

/**
 * @brief 切換到新的儀表板布局
 * @param new_layout 新的布局類型
 */
void switch_dashboard_layout(DashboardLayout_t new_layout);

/**
 * @brief 獲取當前儀表板布局類型
 * @return 當前布局類型
 */
DashboardLayout_t get_current_dashboard_layout(void);

#endif // UI_DASHBOARD_LAYOUTS_H
