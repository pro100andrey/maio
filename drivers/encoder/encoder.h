/**
 * @file encoder.h
 * @brief Rotary encoder driver with integrated button support
 *
 * This driver provides interrupt-based quadrature decoding for rotary encoders
 * with an integrated push button. It uses hardware interrupts for fast response
 * and critical sections for thread-safe operation on dual-core systems.
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <pico/stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Rotary encoder state structure
 *
 * Contains all state information for a single rotary encoder instance.
 * All fields are managed internally by the driver.
 */
typedef struct encoder_s {
  /** Last button interrupt timestamp (ms) for debouncing */
  uint32_t last_button_time;
  /** Last rotation interrupt timestamp (ms) for debouncing */
  uint32_t last_rotation_time;
  /** Accumulated rotation count (increments/decrements) */
  int volatile rotation;
  /** GPIO pin for encoder phase A */
  uint8_t pin_a;
  /** GPIO pin for encoder phase B */
  uint8_t pin_b;
  /** GPIO pin for encoder button/switch */
  uint8_t pin_sw;
  /** Button state flag (true = pressed) */
  bool volatile btn_pressed;
} encoder_t;

/**
 * @brief Initialize rotary encoder with button
 *
 * Configures the specified GPIO pins for encoder operation:
 * - Phase A and B pins are set as inputs with pull-ups
 * - Button pin is set as input with pull-up
 * - Interrupts are enabled on phase A (rising/falling edges)
 * - Interrupt is enabled on button (falling edge)
 *
 * @param enc Pointer to encoder structure to initialize
 * @param gpio_a GPIO pin number for encoder phase A (0-29)
 * @param gpio_b GPIO pin number for encoder phase B (0-29)
 * @param gpio_sw GPIO pin number for encoder button (0-29)
 *
 * @note This function enables hardware interrupts. Ensure interrupts are
 *       properly handled in your application.
 * @note Only one encoder instance is supported due to global ISR handler.
 */
void encoder_init(encoder_t *enc, uint8_t gpio_a, uint8_t gpio_b,
                  uint8_t gpio_sw);

/**
 * @brief Get encoder rotation delta and reset counter
 *
 * Returns the accumulated rotation count since the last call and resets
 * the internal counter to zero. Positive values indicate clockwise rotation,
 * negative values indicate counter-clockwise rotation.
 *
 * @param enc Pointer to initialized encoder structure
 * @return Number of rotation steps since last call (signed integer)
 *
 * @note This function is thread-safe and uses critical sections internally.
 * @note Call this function periodically to prevent counter overflow.
 *
 * @example
 * int delta = encoder_get_delta(&my_encoder);
 * if (delta > 0) {
 *     printf("Rotated clockwise by %d steps\n", delta);
 * } else if (delta < 0) {
 *     printf("Rotated counter-clockwise by %d steps\n", -delta);
 * }
 */
int encoder_get_delta(encoder_t *enc);

#endif // ENCODER_H