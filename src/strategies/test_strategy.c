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
 * @brief Scene: Synchronous full screen fill (REMOVED - use LVGL)
 */
static void scene_full_sync(ili9341_t *dev) {
  printf("[Test] scene_full_sync removed - use LVGL for drawing\\n");
}

/**
 * @brief Scene: Asynchronous full screen fill (DMA)
 * Shows DMA launch time vs total time (with wait)
 */
static void scene_full_async(ili9341_t *dev) {
  static int idx = 0;
  idx = (idx + 1) % 3;
  printf("[Test] Async fill %s\n", color_names[idx]);

  uint32_t start = time_us_32();
  if (ili9341_fill_screen_async(dev, colors[idx])) {
    uint32_t launch_time = time_us_32() - start;
    wait_dma(dev);
    uint32_t total_time = time_us_32() - start;
    printf("[Test] Launch: %u us, Total: %u us (CPU free: %.1f ms)\n",
           launch_time, total_time, (total_time - launch_time) / 1000.0);
  }
}

/**
 * @brief Scene: Four colored quadrants (REMOVED - use LVGL)
 */
static void scene_quadrants(ili9341_t *dev) {
  printf("[Test] scene_quadrants removed - use LVGL for drawing\n");
}

/**
 * @brief Scene: Vertical color stripes (async DMA)
 * Properly overlaps computation with DMA transfers
 */
static void scene_stripes_async(ili9341_t *dev) {
  uint16_t w = ili9341_get_width(dev);
  uint16_t h = ili9341_get_height(dev);
  uint16_t stripe = w / 6;
  printf("[Test] Vertical stripes async\n");

  uint32_t start = time_us_32();
  for (int i = 0; i < 6; i++) {
    // Wait for previous DMA to finish before starting next
    if (i > 0) {
      wait_dma(dev);
    }

    uint16_t x = i * stripe;
    uint16_t c = colors[i % 3];
    ili9341_fill_rect_async(dev, x, 0, (i == 5) ? (w - x) : stripe, h, c);

    // CPU is free here! Could do other work...
    // For demo, just print to show overlap
    if (i < 5) {
      printf("[Test]   Stripe %d launched, CPU doing work while DMA runs...\n",
             i);
    }
  }
  // Wait for last stripe to complete
  wait_dma(dev);
  uint32_t elapsed = time_us_32() - start;
  printf("[Test] Total time with async overlap: %u us (%.2f ms)\n", elapsed,
         elapsed / 1000.0);
}

/**
 * @brief Scene: Toggle display orientation (REMOVED - use LVGL)
 */
static void scene_orientation_toggle(ili9341_t *dev) {
  ili9341_orientation_t cur = ili9341_get_orientation(dev);
  ili9341_orientation_t next =
      (cur == ILI9341_LANDSCAPE) ? ILI9341_PORTRAIT : ILI9341_LANDSCAPE;
  printf("[Test] Orientation %s\n",
         next == ILI9341_LANDSCAPE ? "LANDSCAPE" : "PORTRAIT");
  ili9341_set_orientation(dev, next);
  printf("[Test] Fill screen removed - use LVGL\n");
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
 * @brief Scene: Toggle pixel transfer mode (REMOVED - use LVGL)
 */
static void scene_toggle_spi_mode(ili9341_t *dev) {
  bool current = ili9341_get_pixel_transfer_mode(dev);
  bool next = !current;
  ili9341_set_pixel_transfer_mode(dev, next);
  printf("[Test] SPI mode: %s\n", next ? "16-bit" : "8-bit");
  printf("[Test] Fill screen test removed - use LVGL\n");
}

/**
 * @brief Scene: Window caching test (REMOVED - no caching in LVGL driver)
 */
static void scene_window_caching(ili9341_t *dev) {
  printf("[Test] Window caching removed - not needed for LVGL\n");
}

/**
 * @brief Scene: DMA threshold test (REMOVED - internal to driver)
 */
static void scene_dma_threshold(ili9341_t *dev) {
  printf("[Test] DMA threshold test removed - LVGL handles this\n");
}

/**
 * @brief Scene: Mixed operations (REMOVED - use LVGL)
 */
static void scene_mixed_ops(ili9341_t *dev) {
  printf("[Test] Mixed operations removed - use LVGL for drawing\n");
}

/**
 * @brief Scene: Performance benchmark
 * Measures timing for various operations
 */
static void scene_benchmark(ili9341_t *dev) {
  printf("[Test] Performance benchmark (DMA only)\n");

  uint32_t start, elapsed;

  // Full screen fill (async DMA) - measure launch vs total
  start = time_us_32();
  ili9341_fill_screen_async(dev, 0xFFFF);
  uint32_t launch = time_us_32() - start;
  wait_dma(dev);
  elapsed = time_us_32() - start;
  printf("  Full screen async: launch=%u us, total=%u us (%.2f ms)\n", launch,
         elapsed, elapsed / 1000.0);
  printf("  CPU free time: %.2f ms (%.0f%%)\n", (elapsed - launch) / 1000.0,
         (float)(elapsed - launch) / elapsed * 100);

  printf("[Test] Primitive drawing tests removed - use LVGL\n");
}

/**
 * @brief Scene: Custom pixel buffer test
 * Tests ili9341_send_pixels() with gradient pattern
 */
static void scene_pixel_buffer(ili9341_t *dev) {
  printf("[Test] Custom pixel buffer - gradient\n");

  uint16_t w = 240, h = 100;
  static uint16_t gradient_buffer[240 * 100];

  // Generate horizontal RGB gradient in RGB565 format
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      uint8_t r = (x * 31) / w;
      uint8_t g = ((w - x) * 63) / w;
      uint8_t b = (y * 31) / h;
      uint16_t color = (r << 11) | (g << 5) | b;

      // Swap bytes if in 8-bit mode (buffer needs to be in wire format)
      if (!ili9341_get_pixel_transfer_mode(dev)) {
        color = __builtin_bswap16(color);
      }
      gradient_buffer[y * w + x] = color;
    }
  }

  // Set window for gradient area
  ili9341_set_window(dev, 0, 0, w - 1, h - 1);

  // Draw gradient using send_pixels (should auto-select DMA for large buffer)
  uint32_t start = time_us_32();
  ili9341_send_pixels(dev, gradient_buffer, w * h);
  wait_dma(dev);
  uint32_t elapsed = time_us_32() - start;

  printf("[Test] Gradient (%ux%u = %u pixels): %u us (%.2f ms)\n", w, h, w * h,
         elapsed, elapsed / 1000.0);
}

/**
 * @brief Scene: Power management test (simplified for LVGL)
 * Tests sleep/wakeup, display on/off, and idle mode without drawing
 */
static void scene_power_management(ili9341_t *dev) {
  printf("[Test] Power management test\n");

  printf("[Test]   Drawing test pattern with LVGL...\n");
  sleep_ms(1000);

  // Test display off/on
  printf("[Test]   Display OFF for 2 sec...\n");
  ili9341_display_off(dev);
  sleep_ms(2000);
  printf("[Test]   Display ON\n");
  ili9341_display_on(dev);
  sleep_ms(1000);

  // Test idle mode (reduced colors)
  printf("[Test]   Idle mode ON (8-color) for 2 sec...\n");
  ili9341_set_idle_mode(dev, true);
  sleep_ms(2000);
  printf("[Test]   Idle mode OFF (65K-color)\n");
  ili9341_set_idle_mode(dev, false);
  sleep_ms(1000);

  // Test sleep/wakeup
  printf("[Test]   Sleep mode for 2 sec...\n");
  ili9341_sleep(dev);
  sleep_ms(2000);
  printf("[Test]   Waking up (takes 120ms)...\n");
  ili9341_wakeup(dev);

  printf("[Test] Power management test complete\n");
}

static scene_fn_t scenes[] = {
    scene_full_sync,       scene_full_async,         scene_quadrants,
    scene_stripes_async,   scene_orientation_toggle, scene_backlight_ramp,
    scene_toggle_spi_mode, scene_window_caching,     scene_dma_threshold,
    scene_mixed_ops,       scene_benchmark,          scene_pixel_buffer,
    scene_power_management};

/** Scene names for logging */
static const char *scene_names[] = {
    "Sync fill", "Async fill",   "Quadrants",    "Stripes async", "Orientation",
    "Backlight", "SPI mode",     "Window cache", "DMA threshold", "Mixed ops",
    "Benchmark", "Pixel buffer", "Power mgmt"};

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
