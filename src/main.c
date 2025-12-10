/**
 * @file main.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

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

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/* 全域中文字體 */
static lv_font_t *g_chinese_font = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * 初始化中文字體
 */
static void init_chinese_font(void);

/**
 * 獲取中文字體
 */
lv_font_t *get_chinese_font(void);

#if LV_USE_OS != LV_OS_FREERTOS

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
    /* To try a different demo or example, replace this with one of: */
    /*lv_demo_benchmark();*/
    /*lv_example_freetype_1();*/
    /* - lv_demo_stress(); */
    /* - lv_example_label_1(); */
    /* - etc. */
    // lv_example_get_started_1();
    // lv_example_get_started_2();
    // lv_example_get_started_4();

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

#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/
#if LV_USE_FREETYPE

#if LV_FREETYPE_USE_LVGL_PORT
#define PATH_PREFIX "A:"
#else
#define PATH_PREFIX "./"
#endif

/**
 * Basic example to create a "Hello world" label
 */
void lv_example_get_started_1(void)
{
    /*Create a font*/
    lv_font_t *font = lv_freetype_font_create(PATH_PREFIX "lvgl/examples/libs/freetype/NotoSerifCJK-Regular.ttc",
                                              LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                              24,
                                              LV_FREETYPE_FONT_STYLE_NORMAL);

    if (!font)
    {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }

    /*Create style with the new font*/
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, font);
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);

    /*Change the active screen's background color*/
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_add_style(label, &style, 0);
    lv_label_set_text(label, "Hello 你好\nNoto 中文字型,日文,한글");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_source_han_sans_sc_16_cjk, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

#endif

/*
#if LV_BUILD_EXAMPLES && LV_USE_BUTTON

static const char *number_labels[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "#", "0", "*"};

static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED)
    {
        static uint8_t cnt = 0;
        uint32_t btn_id = (uint32_t)lv_obj_get_user_data(btn);

        lv_obj_set_style_bg_color(btn, lv_color_hex(0x808080), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0096ff), LV_STATE_PRESSED);
    }
}

// Create a button with a label and react on click event.

void lv_example_get_started_2(void)
{
    const int BUTTON_SIZE = 70;
    const int SPACING = 10;
    const int START_X = 40;
    const int START_Y = 40;

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            int index;
            index = row * 3 + col;
            lv_obj_t *btn = lv_button_create(lv_screen_active()); //Add a button the current screen
            int x = START_X + col * (BUTTON_SIZE + SPACING);
            int y = START_Y + row * (BUTTON_SIZE + SPACING);
            lv_obj_set_pos(btn, x, y);                      //Set its position
            lv_obj_set_size(btn, BUTTON_SIZE, BUTTON_SIZE); //Set its size

            lv_obj_set_style_radius(btn, 10, 0); //Make it round
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x0096FF), LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x808080), LV_STATE_PRESSED);

            lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL); //Assign a callback to the button
            lv_obj_set_user_data(btn, (void *)(uintptr_t)index);

            lv_obj_t *label = lv_label_create(btn);         //Add a label to the button
            lv_label_set_text(label, number_labels[index]); //Set the labels text
            lv_obj_center(label);

            lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
            lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        }
    }
}
#endif
*/

#if LV_BUILD_EXAMPLES && LV_USE_BUTTON

static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED)
    {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

/**
 * Create a button with a label and react on click event.
 */
void lv_example_get_started_2(void)
{
    lv_obj_t *btn = lv_button_create(lv_screen_active());       /*Add a button the current screen*/
    lv_obj_set_pos(btn, 120, 360);                              /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                              /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
    lv_label_set_text(label, "Button");     /*Set the labels text*/
    lv_obj_center(label);
}

#endif

#if LV_BUILD_EXAMPLES && LV_USE_SLIDER

static lv_obj_t *label1;
static lv_obj_t *label2;

static void slider1_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target_obj(e);
    char buf[32];
    lv_snprintf(buf, sizeof(buf), "%d", (int)lv_slider_get_value(slider));
    lv_label_set_text(label1, buf);
}

static void slider2_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target_obj(e);
    char buf[32];
    lv_snprintf(buf, sizeof(buf), "%d", (int)lv_slider_get_value(slider));
    lv_label_set_text(label2, buf);
}

/**
 * Create a slider and write its value on a label.
 */
void lv_example_get_started_4(void)
{
    /*Create a slider in the center of the display*/
    lv_obj_t *slider = lv_slider_create(lv_screen_active());
    lv_obj_set_width(slider, 200);                                               /*Set the width*/
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 50);                               /*Align to the top center with offset*/
    lv_obj_add_event_cb(slider, slider1_event_cb, LV_EVENT_VALUE_CHANGED, NULL); /*Assign an event function*/

    /*Create a label above the slider*/
    label1 = lv_label_create(lv_screen_active());
    lv_label_set_text(label1, "0");
    lv_obj_align_to(label1, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/

    /* Create a second slider below the first one*/
    lv_obj_t *slider2 = lv_slider_create(lv_screen_active());
    lv_obj_set_width(slider2, 200);                                               /*Set the width*/
    lv_obj_align_to(slider2, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 50);             /*Align below the first slider*/
    lv_obj_add_event_cb(slider2, slider2_event_cb, LV_EVENT_VALUE_CHANGED, NULL); /*Assign an event function*/

    /*Create a label above the second slider*/
    label2 = lv_label_create(lv_screen_active());
    lv_label_set_text(label2, "0");
    lv_obj_align_to(label2, slider2, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the second slider*/
}

#endif

/**
 * 初始化中文字體
 */
static void init_chinese_font(void)
{
    /* 使用系統安裝的 Noto Sans CJK 字體 */
    g_chinese_font = lv_freetype_font_create("/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
                                             LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                             16,
                                             LV_FREETYPE_FONT_STYLE_NORMAL);

    if (!g_chinese_font)
    {
        LV_LOG_ERROR("Failed to create Chinese font with FreeType");
        /* 回退到內建字體 */
        g_chinese_font = (lv_font_t *)&lv_font_source_han_sans_sc_16_cjk;
    }
    else
    {
        LV_LOG_INFO("Chinese font loaded successfully");
        /* 設置 fallback 字體以支持符號和特殊字符 */
        if (g_chinese_font->fallback == NULL)
        {
            g_chinese_font->fallback = LV_FONT_DEFAULT;
        }
    }
}

/**
 * 獲取中文字體
 */
lv_font_t *get_chinese_font(void)
{
    return g_chinese_font;
}
