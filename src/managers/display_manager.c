/**
 * @file display_manager.c
 * @brief LVGL integration implementation
 */

#include "display_manager.h"
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <stdio.h>
#include <string.h>

// Draw buffer configuration (2 buffers, 10-20 lines each)
#define DRAW_BUF_HEIGHT 20
#define DRAW_BUF_SIZE (ILI9341_WIDTH * DRAW_BUF_HEIGHT)

/** Static LVGL draw buffers */
static uint16_t draw_buf1[DRAW_BUF_SIZE];
static uint16_t draw_buf2[DRAW_BUF_SIZE];

/** LVGL display object */
static lv_display_t *lvgl_display = NULL;

/** Reference to ILI9341 device */
static ili9341_t *ili_dev = NULL;

/**
 * @brief LVGL flush callback - sends pixels to display via DMA
 */
static void flush_cb(lv_display_t *disp, const lv_area_t *area,
                     uint8_t *px_map) {
  if (!ili_dev) {
    lv_display_flush_ready(disp);
    return;
  }

  // Set drawing window
  ili9341_set_window(ili_dev, area->x1, area->y1, area->x2, area->y2);

  // Calculate pixel count
  int32_t w = area->x2 - area->x1 + 1;
  int32_t h = area->y2 - area->y1 + 1;
  size_t pixel_count = w * h;

  // Start DMA transfer
  if (!ili9341_send_pixels_dma(ili_dev, (const uint16_t *)px_map,
                               pixel_count)) {
    printf("[DisplayMgr] Warning: DMA busy, flush skipped\n");
    lv_display_flush_ready(disp);
  }

  // Note: lv_display_flush_ready() will be called from DMA ISR
}

/**
 * @brief DMA interrupt handler
 */
static void dma_irq_handler(void) {
  if (ili_dev) {
    int ch = ili9341_get_dma_channel(ili_dev);
    if (ch >= 0 && dma_hw->ints0 & (1u << ch)) {
      // Clear interrupt
      dma_hw->ints0 = 1u << ch;

      // End CS transaction
      gpio_put(ili_dev->config.pin_cs, 1);

      // Mark DMA idle
      ili_dev->dma_busy = false;

      // Notify LVGL that flush is complete
      if (lvgl_display) {
        lv_display_flush_ready(lvgl_display);
      }
    }
  }
}

void display_manager_init(ili9341_t *ili9341_dev) {
  ili_dev = ili9341_dev;

  // Initialize LVGL
  lv_init();

  // Create LVGL display
  lvgl_display = lv_display_create(ILI9341_WIDTH, ILI9341_HEIGHT);
  lv_display_set_flush_cb(lvgl_display, flush_cb);

  // Set draw buffers (double buffering)
  lv_display_set_buffers(lvgl_display, draw_buf1, draw_buf2, sizeof(draw_buf1),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Register DMA interrupt handler
  int dma_ch = ili9341_get_dma_channel(ili_dev);
  if (dma_ch >= 0) {
    dma_channel_set_irq0_enabled(dma_ch, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
  }

  printf("[DisplayMgr] LVGL initialized: %dx%d, %d buffers of %d bytes\n",
         ILI9341_WIDTH, ILI9341_HEIGHT, 2, sizeof(draw_buf1));
}

lv_display_t *display_manager_get_display(void) { return lvgl_display; }

void display_manager_dma_handler(void) {
  // Handler logic is already in dma_irq_handler
}
