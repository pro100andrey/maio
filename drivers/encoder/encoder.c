
/**
 * @file encoder.c
 * @brief Rotary encoder driver implementation
 *
 * Implements polling-based quadrature decoding with half-step algorithm
 * optimized for mechanical encoders with detents.
 */

#include "encoder.h"
#include <hardware/gpio.h>
#include <hardware/sync.h>

/** Debounce time for button press in milliseconds */
#define ENCODER_BUTTON_DEBOUNCE_MS 50

/** Debounce time for rotation events in milliseconds */
#define ENCODER_ROTATION_DEBOUNCE_MS 2

void encoder_init(encoder_t *enc, uint8_t gpio_a, uint8_t gpio_b,
                  uint8_t gpio_sw) {
  // Reset all state fields
  enc->pin_a = gpio_a;
  enc->pin_b = gpio_b;
  enc->pin_sw = gpio_sw;
  enc->rotation = 0;
  enc->btn_pressed = false;
  enc->last_button_time = 0;
  enc->last_rotation_time = 0;
  enc->last_a_state = false;
  enc->last_b_state = false;

  // Configure encoder GPIOs as inputs (no pull-up - module has its own)
  gpio_init(gpio_a);
  gpio_set_dir(gpio_a, GPIO_IN);
  gpio_disable_pulls(gpio_a);

  gpio_init(gpio_b);
  gpio_set_dir(gpio_b, GPIO_IN);
  gpio_disable_pulls(gpio_b);

  // Configure button/switch pin with pull-up (button connects to GND)
  gpio_init(gpio_sw);
  gpio_set_dir(gpio_sw, GPIO_IN);
  gpio_pull_up(gpio_sw); // Enable pull-up for button

  // Initialize last states for edge detection
  enc->last_a_state = gpio_get(gpio_a);
  enc->last_b_state = gpio_get(gpio_b);
  enc->last_sw_state = gpio_get(gpio_sw);
}

void encoder_poll(encoder_t *enc) {
  const uint32_t now = to_ms_since_boot(get_absolute_time());

  // Read current GPIO states
  bool a = gpio_get(enc->pin_a);
  bool b = gpio_get(enc->pin_b);
  bool sw = gpio_get(enc->pin_sw);

  // Half-step algorithm for mechanical encoders with detents
  // Detent position is typically A=1, B=1 (both HIGH)
  // Detect rotation on falling edge of A (transition from detent)
  if (enc->last_a_state && !a &&
      (now - enc->last_rotation_time > ENCODER_ROTATION_DEBOUNCE_MS)) {
    // When A falls from HIGH to LOW, check B to determine direction
    // If B=HIGH: clockwise (A fell first)
    // If B=LOW: counter-clockwise (B fell first, now A follows)
    if (b) {
      enc->rotation++;
    } else {
      enc->rotation--;
    }
    
    enc->last_rotation_time = now;
  }

  // Update state tracking
  enc->last_a_state = a;
  enc->last_b_state = b;

  // Detect button press on falling edge (HIGH to LOW)
  // Button pulls GPIO to GND when pressed
  if (!sw && enc->last_sw_state &&
      (now - enc->last_button_time > ENCODER_BUTTON_DEBOUNCE_MS)) {
    enc->btn_pressed = true;
    enc->last_button_time = now;
  }

  enc->last_sw_state = sw;
}

int encoder_get_delta(encoder_t *enc) {
  // Disable interrupts during read-modify-write to prevent race condition
  uint32_t status = save_and_disable_interrupts();
  int delta = enc->rotation;
  enc->rotation = 0;
  restore_interrupts(status);

  return delta;
}