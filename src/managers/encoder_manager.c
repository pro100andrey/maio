#include "encoder_manager.h"
#include "../../board_config.h"
#include "../core/event_manager.h"
#include "../shared/events.h"
#include <pico/time.h>

/** Encoder instance */
static encoder_t encoder;

/** Internal timer for polling */
static struct repeating_timer encoder_timer;

/**
 * @brief Internal timer callback for encoder polling
 */
static bool encoder_timer_callback(struct repeating_timer *t) {
  (void)t;

  // Poll GPIO for state changes
  encoder_poll(&encoder);

  // Check for rotation
  int delta = encoder_get_delta(&encoder);
  if (delta != 0) {
    event_t ev = {.type = EV_ENCODER_ROTATE, .payload = delta};
    event_manager_post(&ev);
  }

  // Check for button press
  if (encoder.btn_pressed) {
    // Clear flag
    encoder.btn_pressed = false;
    // payload=1 for pressed
    event_t ev = {.type = EV_ENCODER_BUTTON, .payload = 1};
    event_manager_post(&ev);
  }

  return true; // Keep repeating
}

void encoder_manager_init(uint8_t gpio_a, uint8_t gpio_b, uint8_t gpio_sw) {
  // Initialize encoder driver
  encoder_init(&encoder, gpio_a, gpio_b, gpio_sw);

  // Start internal timer for autonomous polling (1ms interval)
  add_repeating_timer_ms(ENCODER_POLL_MS, encoder_timer_callback, NULL,
                         &encoder_timer);
}

encoder_t *encoder_manager_get_instance(void) { return &encoder; }
