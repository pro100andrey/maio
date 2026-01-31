#include "encoder_manager.h"
#include "../core/event_manager.h"
#include "../shared/events.h"

/** Encoder instance */
static encoder_t encoder;

void encoder_manager_init(uint8_t gpio_a, uint8_t gpio_b, uint8_t gpio_sw) {
  encoder_init(&encoder, gpio_a, gpio_b, gpio_sw);
}

void encoder_manager_poll(void) {
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
    encoder.btn_pressed = false; // Clear flag
    event_t ev = {.type = EV_ENCODER_BUTTON,
                  .payload = 1}; // payload=1 for pressed
    event_manager_post(&ev);
  }
}

encoder_t *encoder_manager_get_instance(void) { return &encoder; }
