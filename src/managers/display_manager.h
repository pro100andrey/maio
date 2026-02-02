/**
 * @file display_manager.h
 * @brief Display manager - hardware abstraction layer
 *
 * Manages:
 * - ILI9341 driver reference
 * - LVGL integration
 * - Hardware initialization
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../../drivers/display/ili9341.h"
#include <lvgl/lvgl.h>
#include <stdbool.h>

/**
 * @brief Initialize display manager
 * @param ili9341_dev Initialized ILI9341 device
 */
void display_manager_init(ili9341_t *ili9341_dev);

/**
 * @brief Initialize LVGL integration
 * Must be called after display_manager_init
 */
void display_manager_init_lvgl(void);

/**
 * @brief Get ILI9341 device reference
 * @return Pointer to ILI9341 device
 */
ili9341_t *display_manager_get_device(void);

/**
 * @brief Get LVGL display object
 * @return Pointer to LVGL display or NULL if not initialized
 */
lv_display_t *display_manager_get_lvgl_display(void);

#endif // DISPLAY_MANAGER_H
