#include "../board_config.h"
#include "core/event_manager.h"
#include "core/strategy.h"
#include "shared/events.h"
#include "strategies/init_strategy.h"
#include <assert.h>
#include <lvgl/lvgl.h>
#include <pico/stdio.h>
#include <pico/time.h>
#include <stdio.h>

// System tick timer callback
//
// Generates periodic EV_TIMER_TICK events for application logic.
// Encoder polling is handled by encoder_manager's internal timer.
static bool timer_callback(struct repeating_timer *t) {
  (void)t;

  // Update LVGL tick (needed for animations and timers)
  lv_tick_inc(SYSTEM_TICK_MS);

  // Post system tick event for application logic
  event_t ev = {.type = EV_TIMER_TICK, .payload = 0};
  event_manager_post(&ev);

  return true; // keep repeating
}

int main(void) {
  // Initialize stdio for printf

  stdio_init_all();

  // Initialize event system
  event_manager_init();

  // Set initial strategy to init (will handle all hardware initialization)
  strategy_switch(&init_strategy);

  // Setup system tick timer
  struct repeating_timer timer;
  add_repeating_timer_ms(SYSTEM_TICK_MS, timer_callback, NULL, &timer);

  event_t event;
  while (true) {
    // Wait for event (uses __wfi() for power efficiency)
    if (event_manager_wait_timeout(&event, EVENT_MANAGER_WAIT_MS)) {
      // Dispatch event to current strategy
      strategy_dispatch_event(&event);
    }
  }

  return 0;
}
