#include "../board_config.h"
#include "core/event_manager.h"
#include "core/strategy.h"
#include "managers/encoder_manager.h"
#include "shared/events.h"
#include "strategies/idle_strategy.h"
#include <pico/stdio.h>
#include <pico/time.h>
#include <stdio.h>

/**
 * @brief Timer callback for encoder polling and periodic ticks
 *
 * Called every ENCODER_POLL_MS by hardware timer:
 * - Polls encoder for rotation/button changes
 * - Posts EV_TIMER_TICK every SYSTEM_TICK_MS to avoid event queue flooding
 *
 * @param t Timer handle (unused)
 * @return true to keep timer repeating
 */
static bool timer_callback(struct repeating_timer *t) {
  (void)t;

  // Poll encoder for changes (every 1ms for responsiveness)
  encoder_manager_poll();

  // Post timer tick event less frequently to avoid queue flooding
  static int tick_counter = 0;
  if (++tick_counter >= SYSTEM_TICK_MS) {
    event_t ev = {.type = EV_TIMER_TICK, .payload = 0};
    event_manager_post(&ev);
    tick_counter = 0;
  }

  return true; // keep repeating
}

int main(void) {
  // Initialize stdio for printf
  stdio_init_all();

  // Initialize event system
  event_manager_init();

  // Initialize encoder
  encoder_manager_init(PIN_ENC_A, PIN_ENC_B, PIN_ENC_SW);
  printf("[Main] Encoder initialized on pins A=%d, B=%d, SW=%d\n", PIN_ENC_A,
         PIN_ENC_B, PIN_ENC_SW);

  // Verify GPIO states after initialization
  sleep_ms(100);
  printf("[Main] GPIO state: A=%d, B=%d, SW=%d (expected: 1,1,1)\n",
         gpio_get(PIN_ENC_A), gpio_get(PIN_ENC_B), gpio_get(PIN_ENC_SW));

  // Set initial strategy
  strategy_switch(&idle_strategy);

  // Setup periodic timer (1ms interval)
  struct repeating_timer timer;
  add_repeating_timer_ms(ENCODER_POLL_MS, timer_callback, NULL, &timer);
  printf("[Main] Timer started: %dms interval\n", ENCODER_POLL_MS);
  printf("[Main] Entering event loop...\n\n");

  event_t event;
  while (true) {
    // Wait for event (blocking, CPU sleeps here)
    event_manager_wait(&event);

    // Dispatch event to current strategy
    strategy_dispatch_event(&event);
  }

  return 0;
}
