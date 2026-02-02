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

#endif // SCREEN_SETTINGS_H
