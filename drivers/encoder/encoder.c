
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

/** Total number of GPIO pins on RP2040/RP2350 */
#define GPIO_PIN_COUNT 32

/** Global pointer to active encoder instance (ISR access) */
static encoder_t *global_enc = NULL;

/** Critical section for thread-safe rotation counter access */
static critical_section_t enc_crit_section;

/**
 * @brief Interrupt handler for encoder rotation and button events
 *
 * This ISR handles two types of events:
 * 1. Button press: Debounced with 50ms threshold
 * 2. Rotation: Quadrature decoding with 2ms debounce
 *
 * The handler uses standard quadrature logic:
 * - When phase A changes and A != B: increment (clockwise)
 * - When phase A changes and A == B: decrement (counter-clockwise)
 *
 * @param gpio The GPIO pin that triggered the interrupt
 * @param events Bitmask of event types (rising/falling edge)
 *
 * @note This function runs in interrupt context - keep it fast!
 * @note Separate debounce timers prevent button/rotation conflicts
 */
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

/**
 * @brief Initialize rotary encoder with button
 *
 * Configuration sequence:
 * 1. Links encoder structure to global ISR handler
 * 2. Initializes critical section for thread safety
 * 3. Resets all state fields to safe defaults
 * 4. Configures GPIO pins (input + pull-up resistors)
 * 5. Enables interrupts on phase A and button
 *
 * @param enc Pointer to encoder structure to initialize
 * @param gpio_a GPIO pin number for encoder phase A (0-29)
 * @param gpio_b GPIO pin number for encoder phase B (0-29)
 * @param gpio_sw GPIO pin number for encoder button (0-29)
 *
 * @note Hardware pull-ups are enabled on all pins (suitable for most encoders)
 * @note Phase A triggers on both edges for maximum resolution
 * @note Button triggers on falling edge (active-low assumption)
 */
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

/**
 * @brief Read and reset rotation counter atomically
 *
 * Returns the accumulated rotation steps since last call and resets
 * the counter to zero. This operation is atomic and thread-safe.
 *
 * Typical polling interval: 10-100ms
 * - Faster polling: more responsive UI
 * - Slower polling: less CPU overhead
 *
 * @param enc Pointer to initialized encoder structure
 * @return Signed delta: positive = clockwise, negative = counter-clockwise
 *
 * @note Critical sections ensure safe access from any core
 * @note Counter overflow is prevented by regular polling
 */
int encoder_get_delta(encoder_t *enc) {
  critical_section_enter_blocking(&enc_crit_section);
  int delta = enc->rotation;
  enc->rotation = 0;
  critical_section_exit(&enc_crit_section);

  return delta;
}