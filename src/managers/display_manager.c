/**
 * @file display_manager.c
 * @brief Display manager implementation with LVGL integration
 */

#include "display_manager.h"
#include <stdio.h>
#include <string.h>

/** Reference to ILI9341 device */
static ili9341_t *ili_dev = NULL;

/** LVGL display object */
static lv_display_t *lv_disp = NULL;

/** LVGL draw buffers */
#define DRAW_BUF_SIZE (320 * 10) // 10 lines buffer
static lv_color_t draw_buf_1[DRAW_BUF_SIZE];
static lv_color_t draw_buf_2[DRAW_BUF_SIZE];

/**
 * @brief LVGL flush callback - sends framebuffer to display
 */
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area,
                          uint8_t *px_map) {
  ili9341_t *dev = display_manager_get_device();
  if (!dev) {
    lv_display_flush_ready(disp);
    return;
  }

  // Calculate dimensions
  int32_t w = area->x2 - area->x1 + 1;
  int32_t h = area->y2 - area->y1 + 1;

  // Set window
  ili9341_set_window(dev, area->x1, area->y1, area->x2, area->y2);

  // Send pixels (DMA will be used automatically if buffer > 2KB)
  ili9341_send_pixels(dev, (const uint16_t *)px_map, w * h);

  // Wait for DMA to complete
  while (!ili9341_dma_is_idle(dev)) {
    tight_loop_contents();
  }

  // Tell LVGL we're done
  lv_display_flush_ready(disp);
}

void display_manager_init(ili9341_t *ili9341_dev) {
  ili_dev = ili9341_dev;
  printf("[DisplayMgr] Display manager initialized\n");
}

void display_manager_init_lvgl(void) {
  if (!ili_dev) {
    printf("[DisplayMgr] Error: Display not initialized\n");
    return;
  }

  printf("[DisplayMgr] Initializing LVGL...\n");

  // Initialize LVGL
  lv_init();

  // Create display
  lv_disp = lv_display_create(ili9341_get_width(ili_dev),
                              ili9341_get_height(ili_dev));

  // Set draw buffers
  lv_display_set_buffers(lv_disp, draw_buf_1, draw_buf_2, sizeof(draw_buf_1),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Set color format to SWAPPED for ILI9341 SPI byte order
  lv_display_set_color_format(lv_disp, LV_COLOR_FORMAT_RGB565_SWAPPED);

  // Set flush callback
  lv_display_set_flush_cb(lv_disp, lvgl_flush_cb);

  printf("[DisplayMgr] LVGL initialized: %dx%d, buffers: %d bytes\n",
         ili9341_get_width(ili_dev), ili9341_get_height(ili_dev),
         (int)sizeof(draw_buf_1));
}

ili9341_t *display_manager_get_device(void) { return ili_dev; }

lv_display_t *display_manager_get_lvgl_display(void) { return lv_disp; }
