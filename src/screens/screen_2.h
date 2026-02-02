/**
 * @file screen_2.h
 * @brief Screen 2 interface
 */

#ifndef SCREEN_2_H
#define SCREEN_2_H

#include <lvgl/lvgl.h>

/**
 * @brief Create Screen 2
 * @return Created screen object
 */
lv_obj_t *screen_2_create(void);

/**
 * @brief Called when screen becomes active
 */
void screen_2_on_show(void);

/**
 * @brief Called when screen is about to be hidden
 */
void screen_2_on_hide(void);

/**
 * @brief Handle encoder rotation
 * @param delta Rotation delta (positive = clockwise)
 */
void screen_2_encoder_rotate(int delta);

#endif // SCREEN_2_H
