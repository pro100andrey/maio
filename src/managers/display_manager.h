/**
 * @file display_manager.h
 * @brief Display manager - hardware abstraction layer
 *
 * Manages:
 * - ILI9341 driver reference
 * - Hardware initialization
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../../drivers/display/ili9341.h"
#include <stdbool.h>

/**
 * @brief Initialize display manager
 * @param ili9341_dev Initialized ILI9341 device
 */
void display_manager_init(ili9341_t *ili9341_dev);

/**
 * @brief Get ILI9341 device reference
 * @return Pointer to ILI9341 device
 */
ili9341_t *display_manager_get_device(void);

#endif // DISPLAY_MANAGER_H
