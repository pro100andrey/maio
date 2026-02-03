/**
 * @file screen_theme.h
 * @brief Theme selection screen interface
 */

#ifndef SCREEN_THEME_H
#define SCREEN_THEME_H

#include <lvgl/lvgl.h>

/**
 * @brief Create theme selection screen
 * @return Created screen object
 */
lv_obj_t *screen_theme_create(void);

/**
 * @brief Called when screen becomes active
 */
void screen_theme_on_show(void);

/**
 * @brief Called when screen is about to be hidden
 */
void screen_theme_on_hide(void);

/**
 * @brief Handle encoder rotation
 * @param delta Rotation delta
 */
void screen_theme_encoder_rotate(int delta);

#endif // SCREEN_THEME_H
