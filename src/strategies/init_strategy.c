/**
 * @file init_strategy.c
 * @brief Initialization strategy implementation
 */

#include "init_strategy.h"
#include "../../board_config.h"
#include "../../drivers/display/ili9341.h"
#include "../core/event_manager.h"
#include "../managers/display_manager.h"
#include "../managers/encoder_manager.h"
#include "idle_strategy.h"
#include <stdio.h>

/** ILI9341 device instance */
static ili9341_t ili_device;

/** Initialization started flag */
static bool init_started = false;

static void init_on_enter(void) {
  printf("[Init] Starting hardware initialization...\n");

  // Initialize encoder (starts internal 1ms polling timer)
  encoder_manager_init(PIN_ENC_A, PIN_ENC_B, PIN_ENC_SW);
  printf("[Init] Encoder initialized\n");

  // Configure ILI9341
  ili9341_config_t config = {
      .spi = TFT_SPI_INST,
      .spi_baudrate = TFT_BAUDRATE,
      .pin_cs = PIN_TFT_CS,
      .pin_dc = PIN_TFT_DC,
      .pin_rst = PIN_TFT_RST,
      .pin_led = PIN_TFT_LED,
      .pin_sck = PIN_TFT_SCK,
      .pin_mosi = PIN_TFT_MOSI,
  };

  // Initialize driver (non-blocking)
  ili9341_init(&ili_device, &config);
  init_started = false; // Will start on first tick
}

static void init_on_exit(void) { printf("[Init] Initialization complete\n"); }

static void init_on_event(const event_t *event) {
  if (event->type == EV_TIMER_TICK) {
    if (!init_started) {
      // Post init tick event to start async init
      event_t ev = {.type = EV_DISPLAY_INIT_TICK, .payload = 0};
      event_manager_post(&ev);
      init_started = true;
      return;
    }

    // Continue async init
    if (ili9341_init_tick(&ili_device)) {
      printf("[Init] Hardware initialized\n");

      // Store device reference in manager
      display_manager_init(&ili_device);

      // Turn on backlight
      ili9341_set_backlight(&ili_device, 255);
      printf("[Init] Backlight ON\n");

      // Set default orientation to landscape
      ili9341_set_orientation(&ili_device, ILI9341_LANDSCAPE);

      // Test display: fill screen with red (RGB565: 0xF800)
      printf("[Init] Testing display - filling screen with RED...\n");
      ili9341_fill_screen(&ili_device, 0xF800);

      // Draw green diagonal line from (0,0) to bottom-right
      uint16_t w = ili9341_get_width(&ili_device);
      uint16_t h = ili9341_get_height(&ili_device);
      printf("[Init] Drawing GREEN diagonal line...\n");
      ili9341_draw_line(&ili_device, 0, 0, w - 1, h - 1, 0x07E0);

      printf("[Init] Display test complete!\n");
      printf("[Init] Switching to Idle mode...\n");
      strategy_switch(&idle_strategy);
    }
  }
}

const strategy_t init_strategy = {
    .name = "Init",
    .on_enter = init_on_enter,
    .on_exit = init_on_exit,
    .on_event = init_on_event,
};
