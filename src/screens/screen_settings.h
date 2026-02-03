/**
 * @file screen_settings.h
 * @brief Settings screen interface
 */

#ifndef SCREEN_SETTINGS_H
#define SCREEN_SETTINGS_H

#include <lvgl/lvgl.h>

/**
 * @brief Create Settings screen
 * @return Created screen object
 */
lv_obj_t *screen_settings_create(void);

/**
 * @brief Update settings screen content
 */
void screen_settings_update(void);

/**
 * @brief Called when screen becomes active
 */
void screen_settings_on_show(void);

/**
 * @brief Called when screen is about to be hidden
 */
void screen_settings_on_hide(void);

/**
 * @brief Handle encoder rotation
 * @param delta Rotation delta
 */
void screen_settings_encoder_rotate(int delta);

/**
 * @brief Handle menu item selection
 */
void screen_settings_select(void);

#endif // SCREEN_SETTINGS_H
