/**
 * @file test_strategy.c
 * @brief Display test strategy implementation
 *
 * Provides interactive demo scenes to test display driver functionality:
 * - Synchronous/asynchronous fills
 * - DMA operations
 * - Orientation switching
 * - Backlight control with gamma correction
 * - Drawing primitives (lines, rectangles)
 *
 * Controls:
 * - Encoder rotation: Select scene
 * - Encoder button: Run selected scene (long press to exit to idle)
 */

#include "test_strategy.h"
#include "../../drivers/display/ili9341.h"
#include "../managers/display_manager.h"
#include "idle_strategy.h"
#include <math.h>
#include <pico/time.h>
#include <stdio.h>

// Demo scenes to exercise driver paths
static int scene_index = 0;
static uint32_t button_press_time = 0;

static const uint16_t colors[] = {0xF800, 0x07E0, 0x001F};
static const char *color_names[] = {"RED", "GREEN", "BLUE"};

typedef void (*scene_fn_t)(ili9341_t *dev);

/**
 * @brief Wait for DMA operation to complete
 */
static void wait_dma(ili9341_t *dev) {
  while (!ili9341_dma_is_idle(dev)) {
    tight_loop_contents();
  }
}

/**
 * @brief Scene: Synchronous full screen fill with diagonal line
 */
static void scene_full_sync(ili9341_t *dev) {
  static int idx = 0;
  idx = (idx + 1) % 3;
  printf("[Test] Sync fill %s\n", color_names[idx]);
  ili9341_fill_screen(dev, colors[idx]);
  ili9341_draw_line(dev, 0, 0, ili9341_get_width(dev) - 1,
                    ili9341_get_height(dev) - 1, 0xFFFF);
}

/**
 * @brief Scene: Asynchronous full screen fill (DMA)
 */
static void scene_full_async(ili9341_t *dev) {
  static int idx = 0;
  idx = (idx + 1) % 3;
  printf("[Test] Async fill %s\n", color_names[idx]);
  if (ili9341_fill_screen_async(dev, colors[idx])) {
    wait_dma(dev);
  }
}

/**
 * @brief Scene: Four colored quadrants with diagonal lines
 */
static void scene_quadrants(ili9341_t *dev) {
  uint16_t w = ili9341_get_width(dev);
  uint16_t h = ili9341_get_height(dev);
  uint16_t hw = w / 2;
  uint16_t hh = h / 2;
  printf("[Test] Quadrants sync\n");
  ili9341_fill_rect(dev, 0, 0, hw, hh, 0xF800);
  ili9341_fill_rect(dev, hw, 0, w - hw, hh, 0x07E0);
  ili9341_fill_rect(dev, 0, hh, hw, h - hh, 0x001F);
  ili9341_fill_rect(dev, hw, hh, w - hw, h - hh, 0xFFFF);
  ili9341_draw_line(dev, 0, 0, w - 1, h - 1, 0xFFE0);
  ili9341_draw_line(dev, w - 1, 0, 0, h - 1, 0xFFE0);
}

/**
 * @brief Scene: Vertical color stripes (async DMA)
 */
static void scene_stripes_async(ili9341_t *dev) {
  uint16_t w = ili9341_get_width(dev);
  uint16_t h = ili9341_get_height(dev);
  uint16_t stripe = w / 6;
  printf("[Test] Vertical stripes async\n");
  for (int i = 0; i < 6; i++) {
    uint16_t x = i * stripe;
    uint16_t c = colors[i % 3];
    if (ili9341_fill_rect_async(dev, x, 0, (i == 5) ? (w - x) : stripe, h, c)) {
      wait_dma(dev);
    }
  }
}

/**
 * @brief Scene: Toggle display orientation (landscape <-> portrait)
 */
static void scene_orientation_toggle(ili9341_t *dev) {
  ili9341_orientation_t cur = ili9341_get_orientation(dev);
  ili9341_orientation_t next =
      (cur == ILI9341_LANDSCAPE) ? ILI9341_PORTRAIT : ILI9341_LANDSCAPE;
  printf("[Test] Orientation %s\n",
         next == ILI9341_LANDSCAPE ? "LANDSCAPE" : "PORTRAIT");
  ili9341_set_orientation(dev, next);
  ili9341_fill_screen(dev, 0x0000);
  ili9341_draw_line(dev, 0, 0, ili9341_get_width(dev) - 1,
                    ili9341_get_height(dev) - 1, 0x07E0);
}

/**
 * @brief Scene: Backlight brightness ramp with gamma correction
 */
static void scene_backlight_ramp(ili9341_t *dev) {
  static int percent = 100;
  // Gamma-corrected brightness for perceptually linear fading
  double p = percent / 100.0;
  double gamma_level = pow(p, 2.2);
  uint8_t level = (uint8_t)(gamma_level * 255.0);

  // Ensure minimum backlight level for visibility
  if (level < 2) {
    level = 2;
  }

  ili9341_set_backlight(dev, level);
  printf("[Test] Backlight %d%% (PWM=%u, gamma=%.3f)\n", percent, level,
         gamma_level);

  percent -= 10;
  if (percent < 0) {
    percent = 100;
  }
}

/**
 * @brief Scene: Toggle pixel transfer mode (8-bit/16-bit SPI)
 */
static void scene_toggle_spi_mode(ili9341_t *dev) {
  bool current = ili9341_get_pixel_transfer_mode(dev);
  bool next = !current;
  ili9341_set_pixel_transfer_mode(dev, next);
  printf("[Test] SPI mode: %s\n", next ? "16-bit" : "8-bit");

  // Test fill with new mode
  ili9341_fill_screen(dev, 0x07E0); // Green
  sleep_ms(500);
  ili9341_fill_screen(dev, 0x001F); // Blue
}

/** Array of scene functions */
static scene_fn_t scenes[] = {scene_full_sync,          scene_full_async,
                              scene_quadrants,          scene_stripes_async,
                              scene_orientation_toggle, scene_backlight_ramp,
                              scene_toggle_spi_mode};

/** Scene names for logging */
static const char *scene_names[] = {"Sync fill",     "Async fill",  "Quadrants",
                                    "Stripes async", "Orientation", "Backlight",
                                    "SPI mode"};

/**
 * @brief Execute currently selected scene
 */
static void run_scene(void) {
  ili9341_t *dev = display_manager_get_device();
  if (!dev)
    return;

  int count = (int)(sizeof(scenes) / sizeof(scenes[0]));
  int idx = scene_index % count;
  if (idx < 0)
    idx += count;

  printf("[Test] Run scene %d: %s\n", idx, scene_names[idx]);
  scenes[idx](dev);
}

static void test_on_enter(void) {
  printf("[Test] Display test mode activated\n");
  scene_index = 0;
  run_scene();
}

static void test_on_exit(void) {
  printf("[Test] Display test mode deactivated\n");
}

static void test_on_event(const event_t *event) {
  switch (event->type) {
  case EV_ENCODER_ROTATE: {
    int count = (int)(sizeof(scenes) / sizeof(scenes[0]));
    if (event->payload > 0) {
      scene_index = (scene_index + 1) % count;
    } else {
      scene_index = (scene_index - 1 + count) % count;
    }
    printf("[Test] Selected scene %d: %s\n", scene_index,
           scene_names[scene_index]);
    break;
  }
  case EV_ENCODER_BUTTON:
    if (event->payload == 1) {
      // Button pressed - record time for long press detection
      button_press_time = to_ms_since_boot(get_absolute_time());
      run_scene();
    } else if (event->payload == 0 && button_press_time > 0) {
      // Button released - check if it was long press (>1 second)
      uint32_t press_duration =
          to_ms_since_boot(get_absolute_time()) - button_press_time;
      if (press_duration > 1000) {
        printf("[Test] Long press detected - returning to idle\n");
        strategy_switch(&idle_strategy);
      }
      button_press_time = 0;
    }
    break;
  case EV_TIMER_TICK:
    // No periodic action in test mode
    break;
  default:
    break;
  }
}

const strategy_t test_strategy = {
    .name = "Test",
    .on_enter = test_on_enter,
    .on_exit = test_on_exit,
    .on_event = test_on_event,
};
