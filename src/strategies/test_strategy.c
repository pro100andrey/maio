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

/**
 * @brief Scene: Window caching optimization test
 * Repeatedly draws to same window to test cache hit performance
 */
static void scene_window_caching(ili9341_t *dev) {
  printf("[Test] Window caching - repeated draws to same area\n");

  // Small rectangles (10x10) where window setup overhead is significant
  uint16_t w = 10, h = 10;
  uint32_t start, elapsed_cached, elapsed_uncached;

  // Test 1: Draw same window 100 times (cache hits after first)
  start = time_us_32();
  for (int i = 0; i < 100; i++) {
    ili9341_fill_rect(dev, 50, 50, w, h, colors[i % 3]);
  }
  elapsed_cached = time_us_32() - start;
  printf("[Test] 100 cached fills (10x10): %u us (%.1f us/op)\n",
         elapsed_cached, elapsed_cached / 100.0);

  // Test 2: Draw 100 different windows (cache misses, same size)
  start = time_us_32();
  for (int i = 0; i < 100; i++) {
    // Draw at different positions to force cache miss
    uint16_t x = (i * 3) % 200;
    uint16_t y = (i * 2) % 150;
    ili9341_fill_rect(dev, x, y, w, h, colors[i % 3]);
  }
  elapsed_uncached = time_us_32() - start;
  printf("[Test] 100 uncached fills (10x10): %u us (%.1f us/op)\n",
         elapsed_uncached, elapsed_uncached / 100.0);

  float speedup = (float)elapsed_uncached / elapsed_cached;
  float saved_us = (elapsed_uncached - elapsed_cached) / 100.0f;
  printf("[Test] Cache speedup: %.2fx (~%.1f us saved/op)\n", speedup,
         saved_us);
}

/**
 * @brief Scene: DMA threshold test (smart DMA vs blocking selection)
 */
static void scene_dma_threshold(ili9341_t *dev) {
  printf("[Test] DMA threshold - testing smart selection\n");

  // Test small buffer (< 2KB, should use blocking)
  uint16_t small_buffer[512]; // 1KB
  for (int i = 0; i < 512; i++) {
    small_buffer[i] = 0xF800; // Red
  }

  // Clear area and set window for small buffer
  ili9341_fill_rect(dev, 0, 0, 240, 2, 0x0000);
  ili9341_set_window(dev, 0, 0, 239, 1); // 240x2 area

  uint32_t start = time_us_32();
  ili9341_send_pixels(dev, small_buffer, 240 * 2); // 480 pixels = 960 bytes
  uint32_t small_time = time_us_32() - start;

  // Test large buffer (> 2KB, should use DMA)
  static uint16_t large_buffer[2048]; // 4KB
  for (int i = 0; i < 2048; i++) {
    large_buffer[i] = 0x07E0; // Green
  }

  // Clear area and set window for large buffer
  ili9341_fill_rect(dev, 0, 50, 240, 9, 0x0000);
  ili9341_set_window(dev, 0, 50, 239, 58); // 240x9 area

  start = time_us_32();
  ili9341_send_pixels(dev, large_buffer, 240 * 9); // 2160 pixels = 4320 bytes
  wait_dma(dev);
  uint32_t large_time = time_us_32() - start;

  printf("[Test] Small (960B): %u us, Large (4.3KB): %u us\n", small_time,
         large_time);
}

/**
 * @brief Scene: Mixed drawing operations (async pipeline)
 * Demonstrates proper async operation chaining with CPU/DMA overlap
 */
static void scene_mixed_ops(ili9341_t *dev) {
  printf("[Test] Mixed operations - async pipeline\n");

  uint16_t w = ili9341_get_width(dev);
  uint16_t h = ili9341_get_height(dev);
  uint32_t start = time_us_32();

  // Step 1: Launch background fill (async)
  ili9341_fill_screen_async(dev, 0x0000);
  printf("[Test]   1. Background DMA started, CPU free for work...\n");

  // CPU work: Prepare next operation parameters while DMA runs
  uint16_t rect_params[][5] = {
      {10, 10, 60, 40, 0xF800}, // Red
      {80, 10, 60, 40, 0x07E0}, // Green
      {150, 10, 60, 40, 0x001F} // Blue
  };

  // Step 2: Wait for background, launch first rectangle
  wait_dma(dev);
  ili9341_fill_rect_async(dev, rect_params[0][0], rect_params[0][1],
                          rect_params[0][2], rect_params[0][3],
                          rect_params[0][4]);
  printf("[Test]   2. Red rect DMA started...\n");

  // Step 3: Wait, launch green rectangle
  wait_dma(dev);
  ili9341_fill_rect_async(dev, rect_params[1][0], rect_params[1][1],
                          rect_params[1][2], rect_params[1][3],
                          rect_params[1][4]);
  printf("[Test]   3. Green rect DMA started...\n");

  // Step 4: Wait, launch blue rectangle
  wait_dma(dev);
  ili9341_fill_rect_async(dev, rect_params[2][0], rect_params[2][1],
                          rect_params[2][2], rect_params[2][3],
                          rect_params[2][4]);
  printf("[Test]   4. Blue rect DMA started...\n");

  // Step 5: Wait for blue rect, draw lines (sync operations - small data)
  wait_dma(dev);
  ili9341_draw_line(dev, 0, 60, w - 1, 60, 0xFFFF);       // Horizontal
  ili9341_draw_line(dev, 120, 70, 120, h - 10, 0xFFFF);   // Vertical
  ili9341_draw_line(dev, 10, 70, w - 10, h - 10, 0xFFE0); // Diagonal
  printf("[Test]   5. Lines drawn (sync)\n");

  // Step 6: Draw pixels (star pattern) - sync, small operations
  uint16_t cx = w / 2, cy = h - 50;
  for (int angle = 0; angle < 360; angle += 30) {
    double rad = angle * 3.14159 / 180.0;
    int px = cx + (int)(30 * cos(rad));
    int py = cy + (int)(30 * sin(rad));
    ili9341_draw_pixel(dev, px, py, 0xF81F); // Magenta
  }
  printf("[Test]   6. Star pattern drawn\n");

  uint32_t elapsed = time_us_32() - start;
  printf("[Test] Total async pipeline: %u us (%.2f ms)\n", elapsed,
         elapsed / 1000.0);
}

/**
 * @brief Scene: Performance benchmark
 * Measures timing for various operations
 */
static void scene_benchmark(ili9341_t *dev) {
  printf("[Test] Performance benchmark\n");

  uint32_t start, elapsed;

  // Full screen fill (sync)
  start = time_us_32();
  ili9341_fill_screen(dev, 0x0000);
  elapsed = time_us_32() - start;
  printf("  Full screen sync: %u us (%.2f ms)\n", elapsed, elapsed / 1000.0);

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

  // Small rectangles (100x50)
  start = time_us_32();
  for (int i = 0; i < 10; i++) {
    ili9341_fill_rect(dev, 10 + i * 5, 10, 100, 50, colors[i % 3]);
  }
  elapsed = time_us_32() - start;
  printf("  10 rects (100x50): %u us (%.1f us/rect)\n", elapsed,
         elapsed / 10.0);

  // Pixels (diagonal line)
  start = time_us_32();
  for (int i = 0; i < 100; i++) {
    ili9341_draw_pixel(dev, i, i, 0xFFFF);
  }
  elapsed = time_us_32() - start;
  printf("  100 pixels: %u us (%.1f us/pixel)\n", elapsed, elapsed / 100.0);

  // Lines
  start = time_us_32();
  for (int i = 0; i < 10; i++) {
    ili9341_draw_line(dev, 0, i * 20, 239, i * 20, 0x07E0);
  }
  elapsed = time_us_32() - start;
  printf("  10 h-lines: %u us (%.1f us/line)\n", elapsed, elapsed / 10.0);
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

  // Clear screen
  ili9341_fill_screen(dev, 0x0000);

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

static scene_fn_t scenes[] = {
    scene_full_sync,       scene_full_async,         scene_quadrants,
    scene_stripes_async,   scene_orientation_toggle, scene_backlight_ramp,
    scene_toggle_spi_mode, scene_window_caching,     scene_dma_threshold,
    scene_mixed_ops,       scene_benchmark,          scene_pixel_buffer};

/** Scene names for logging */
static const char *scene_names[] = {
    "Sync fill",     "Async fill", "Quadrants", "Stripes async",
    "Orientation",   "Backlight",  "SPI mode",  "Window cache",
    "DMA threshold", "Mixed ops",  "Benchmark", "Pixel buffer"};

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
