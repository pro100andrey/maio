/**
 * @file display_manager.h
 * @brief LVGL integration layer - bridge between LVGL and ILI9341 driver
 *
 * Manages:
 * - LVGL display initialization
 * - Flush callback (LVGL -> ILI9341)
 * - DMA ISR handling
 * - Static draw buffers
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../../drivers/display/ili9341.h"
#include <lvgl.h>
#include <stdbool.h>

/**
 * @brief Initialize display manager and LVGL
 * @param ili9341_dev Initialized ILI9341 device
 */
void display_manager_init(ili9341_t *ili9341_dev);

/**
 * @brief Get LVGL display object
 * @return Pointer to LVGL display
 */
lv_display_t *display_manager_get_display(void);

/**
 * @brief DMA completion handler (call from DMA ISR)
 */
void display_manager_dma_handler(void);

#endif // DISPLAY_MANAGER_H
