/**
 * @file idle_strategy.c
 * @brief Idle/standby strategy implementation
 *
 * Default idle mode - minimal activity, waiting for user input.
 * Can switch to test mode for display testing or to measurement modes.
 */

#include "idle_strategy.h"
#include "../../drivers/display/ili9341.h"
#include "../managers/display_manager.h"
#include "test_strategy.h"
#include <pico/time.h>
#include <stdio.h>

static int encoder_position = 0;

static void idle_on_enter(void) {
  printf("[Idle] Mode activated\n");
  printf("[Idle] Press button to enter test mode\n");

  // Display simple idle screen (primitives removed - use LVGL for drawing)
  ili9341_t *dev = display_manager_get_device();
  if (dev) {
    // Clear screen using async fill
    ili9341_fill_screen_async(dev, 0x0000);
    while (!ili9341_dma_is_idle(dev)) {
      tight_loop_contents();
    }
    printf("[Idle] Screen cleared (border drawing removed - use LVGL)\n");
  }

  encoder_position = 0;
}

static void idle_on_exit(void) { printf("[Idle] Mode deactivated\n"); }

static void idle_on_event(const event_t *event) {
  switch (event->type) {
  case EV_ENCODER_ROTATE:
    encoder_position += event->payload;
    printf("[Idle] Encoder position: %d\n", encoder_position);
    break;

  case EV_ENCODER_BUTTON:
    if (event->payload == 1) {
      printf("[Idle] Button pressed - switching to test mode\n");
      strategy_switch(&test_strategy);
    }
    break;

  case EV_TIMER_TICK:
    // No periodic action in idle mode
    break;

  default:
    break;
  }
}

const strategy_t idle_strategy = {
    .name = "Idle",
    .on_enter = idle_on_enter,
    .on_exit = idle_on_exit,
    .on_event = idle_on_event,
};
