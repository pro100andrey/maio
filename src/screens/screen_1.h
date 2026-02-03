/**
 * @file screen_1.h
 * @brief Screen 1 interface
 */

#ifndef SCREEN_1_H
#define SCREEN_1_H

#include <lvgl/lvgl.h>

/**
 * @brief Create Screen 1
 * @return Created screen object
 */
lv_obj_t *screen_1_create(void);

/**
 * @brief Called when screen becomes active
 */
void screen_1_on_show(void);

/**
 * @brief Called when screen is about to be hidden
 */
void screen_1_on_hide(void);

#endif // SCREEN_1_H
