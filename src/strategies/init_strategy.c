/**
 * @file init_strategy.c
 * @brief Initialization strategy implementation
 */

#include "init_strategy.h"
#include "../../board_config.h"
#include "../../drivers/display/ili9341.h"
#include "../core/event_manager.h"
#include "../managers/display_manager.h"
#include "idle_strategy.h"
#include <stdio.h>

/** ILI9341 device instance */
static ili9341_t ili_device;

/** Initialization started flag */
static bool init_started = false;

static void init_on_enter(void) {
  printf("[Init] Starting display initialization...\n");

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
      // Hardware ready!
      printf("[Init] Hardware initialized, setting up LVGL...\n");

      // Initialize LVGL
      display_manager_init(&ili_device);

      // Turn on backlight
      ili9341_set_backlight(&ili_device, 255);

      // Post ready event
      event_t ev = {.type = EV_DISPLAY_READY, .payload = 0};
      event_manager_post(&ev);

      // Switch to idle strategy
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
