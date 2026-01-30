
/**
 * @file encoder.c
 * @brief Rotary encoder driver implementation
 *
 * Implements interrupt-driven quadrature decoding with hardware debouncing
 * and thread-safe rotation counter management.
 */

#include "encoder.h"
#include <hardware/gpio.h>
#include <pico/critical_section.h>
#include <stdio.h>

/** Debounce time for button press in milliseconds */
#define ENCODER_BUTTON_DEBOUNCE_MS 50

/** Debounce time for rotation events in milliseconds */
#define ENCODER_ROTATION_DEBOUNCE_MS 2

/** Global pointer to active encoder instance (ISR access) */
static encoder_t *global_enc = NULL;

/** Critical section for thread-safe rotation counter access */
static critical_section_t enc_crit_section;

void encoder_irq_handler(uint gpio, uint32_t events) {
  encoder_t *const enc = global_enc;
  if (!enc)
    return;

  const uint32_t now = to_ms_since_boot(get_absolute_time());
  const uint32_t gpio_state = sio_hw->gpio_in;

  // Handle button press with configurable debounce
  if (gpio == enc->pin_sw) {
    if (now - enc->last_button_time > ENCODER_BUTTON_DEBOUNCE_MS) {
      enc->btn_pressed = true;
      enc->last_button_time = now;
    }
    return;
  }

  // Handle rotation with configurable debounce for responsive feel
  if (gpio == enc->pin_a &&
      (now - enc->last_rotation_time > ENCODER_ROTATION_DEBOUNCE_MS)) {
    // Read both phase signals atomically from hardware register
    bool a = (gpio_state >> enc->pin_a) & 1;
    bool b = (gpio_state >> enc->pin_b) & 1;

    critical_section_enter_blocking(&enc_crit_section);
    // Standard quadrature logic:
    // If A changed and A != B: clockwise rotation (+1)
    // If A changed and A == B: counter-clockwise rotation (-1)
    if (a != b) {
      enc->rotation++;
    } else {
      enc->rotation--;
    }
    critical_section_exit(&enc_crit_section);

    enc->last_rotation_time = now;
  }
}


void encoder_init(encoder_t *enc, uint8_t gpio_a, uint8_t gpio_b,
                  uint8_t gpio_sw) {
  // Link encoder instance to global pointer for ISR access
  global_enc = enc;

  // Initialize critical section for dual-core safety
  critical_section_init(&enc_crit_section);

  // Reset all state fields to safe initial values
  enc->pin_a = gpio_a;
  enc->pin_b = gpio_b;
  enc->pin_sw = gpio_sw;
  enc->rotation = 0;
  enc->btn_pressed = false;
  enc->last_button_time = 0;
  enc->last_rotation_time = 0;

  // Configure encoder GPIOs as inputs with pull-ups
  gpio_init(gpio_a);
  gpio_set_dir(gpio_a, GPIO_IN);
  gpio_pull_up(gpio_a);

  gpio_init(gpio_b);
  gpio_set_dir(gpio_b, GPIO_IN);
  gpio_pull_up(gpio_b);

  gpio_init(gpio_sw);
  gpio_set_dir(gpio_sw, GPIO_IN);
  gpio_pull_up(gpio_sw);

  // Enable interrupts on phase A (both edges for full resolution)
  gpio_set_irq_enabled_with_callback(enc->pin_a,
                                     GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                     true, &encoder_irq_handler);

  // Enable interrupt on button (falling edge = button pressed to GND)
  gpio_set_irq_enabled(enc->pin_sw, GPIO_IRQ_EDGE_FALL, true);
}

int encoder_get_delta(encoder_t *enc) {
  critical_section_enter_blocking(&enc_crit_section);
  int delta = enc->rotation;
  enc->rotation = 0;
  critical_section_exit(&enc_crit_section);

  return delta;
}