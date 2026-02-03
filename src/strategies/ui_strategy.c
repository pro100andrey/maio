/**
 * @file ui_strategy.c
 * @brief UI mode strategy with LVGL screen navigation
 */

#include "ui_strategy.h"
#include "../managers/screen_manager.h"
#include "../managers/theme_manager.h"
#include "../screens/screen_settings.h"
#include "../shared/events.h"
#include "idle_strategy.h"
#include <lvgl/lvgl.h>
#include <pico/time.h>
#include <stdio.h>

/** Track button press time for long press detection */
static uint32_t button_press_time = 0;

/**
 * @brief Initialize UI mode
 */
static void ui_on_enter(void) {
  printf("[UI] UI mode activated\n");

  // Initialize theme manager
  theme_manager_init();

  // Initialize screen manager (creates all screens)
  screen_manager_init();

  // Show first screen
  screen_manager_show(SCREEN_1);

  button_press_time = 0;
}

/**
 * @brief Cleanup UI mode
 */
static void ui_on_exit(void) { printf("[UI] UI mode deactivated\n"); }

/**
 * @brief Handle events in UI mode
 */
static void ui_on_event(const event_t *event) {
  switch (event->type) {
  case EV_ENCODER_ROTATE: {
    // Encoder rotation now used for menu navigation on multimeter screen
    screen_manager_encoder_rotate(event->payload);
    break;
  }

  case EV_ENCODER_BUTTON:
    if (event->payload == 1) {
      // Button pressed - record time for long press detection
      button_press_time = to_ms_since_boot(get_absolute_time());
    } else if (event->payload == 0 && button_press_time > 0) {
      // Button released - check press duration
      uint32_t press_duration =
          to_ms_since_boot(get_absolute_time()) - button_press_time;

      if (press_duration > 3000) {
        // Long press (>3 seconds) - exit multimeter/theme/about or return to
        // idle
        screen_id_t current = screen_manager_get_current();
        if (current == SCREEN_MULTIMETER || current == SCREEN_THEME ||
            current == SCREEN_ABOUT) {
          printf("[UI] Long press - returning to screen 1\n");
          screen_manager_show(SCREEN_1);
        } else {
          printf("[UI] Long press - returning to idle\n");
          strategy_switch(&idle_strategy);
        }
      } else {
        // Short press
        screen_id_t current = screen_manager_get_current();

        if (current == SCREEN_SETTINGS) {
          // Settings: short press selects menu item
          printf("[UI] Short press on settings - selecting item\n");
          screen_settings_select();
        } else if (current == SCREEN_THEME || current == SCREEN_ABOUT) {
          // Theme/About: short press returns to Settings
          printf("[UI] Short press - returning to settings\n");
          screen_manager_show(SCREEN_SETTINGS);
        } else {
          // Other screens: switch to next screen
          printf("[UI] Short press - next screen\n");
          screen_manager_next();
        }
      }
      button_press_time = 0;
    }
    break;

  case EV_TIMER_TICK:
    // Call LVGL timer handler to process animations, etc.
    lv_timer_handler();

    // Update current screen if needed
    screen_manager_update();
    break;

  default:
    break;
  }
}

/** UI strategy instance */
const strategy_t ui_strategy = {.name = "UI",
                                .on_enter = ui_on_enter,
                                .on_exit = ui_on_exit,
                                .on_event = ui_on_event};
