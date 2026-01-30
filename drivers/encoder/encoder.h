/**
 * @file encoder.h
 * @brief Rotary encoder driver with integrated button support
 *
 * This driver provides polling-based quadrature decoding for rotary encoders
 * with an integrated push button. It uses half-step algorithm optimized for
 * mechanical encoders with detents (one click = one increment).
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
  /** Last button timestamp (ms) for debouncing */
  uint32_t last_button_time;
  /** Last rotation timestamp (ms) for debouncing */
  uint32_t last_rotation_time;
  /** Accumulated rotation count (increments/decrements) */
  int rotation;
  /** GPIO pin for encoder phase A */
  uint8_t pin_a;
  /** GPIO pin for encoder phase B */
  uint8_t pin_b;
  /** GPIO pin for encoder button/switch */
  uint8_t pin_sw;
  /** Button state flag (true = pressed) */
  bool btn_pressed;
  /** Last state of phase A for edge detection */
  bool last_a_state;
  /** Last state of phase B for edge detection */
  bool last_b_state;
  /** Last state of button for edge detection */
  bool last_sw_state;
} encoder_t;

/**
 * @brief Initialize rotary encoder with button
 *
 * Configures the specified GPIO pins for encoder operation:
 * - Phase A and B pins: inputs without pull resistors (module has external
 * pull-ups to VCC)
 * - Button pin: input with internal pull-up enabled (button connects to GND)
 * - Polling-based operation (no interrupts)
 *
 * @param enc Pointer to encoder structure to initialize
 * @param gpio_a GPIO pin number for encoder phase A (0-29)
 * @param gpio_b GPIO pin number for encoder phase B (0-29)
 * @param gpio_sw GPIO pin number for encoder button (0-29)
 *
 * @note Call encoder_poll() regularly (every 1ms) for responsive operation.
 * @note Uses half-step algorithm: one detent click = one increment.
 */
void encoder_init(encoder_t *enc, uint8_t gpio_a, uint8_t gpio_b,
                  uint8_t gpio_sw);

/**
 * @brief Poll encoder for state changes
 *
 * Reads GPIO states and updates rotation counter and button flag.
 * Implements half-step algorithm with debouncing.
 *
 * @param enc Pointer to encoder structure
 *
 * @note Must be called regularly (every 1ms) from timer callback.
 * @note Rotation: detects falling edge of phase A
 * @note Button: detects falling edge (HIGH to LOW when pressed)
 */
void encoder_poll(encoder_t *enc);

/**
 * @brief Get encoder rotation delta and reset counter
 *
 * Returns accumulated rotation since last call and resets counter to zero.
 * Positive values = clockwise, negative values = counter-clockwise.
 *
 * @param enc Pointer to initialized encoder structure
 * @return Rotation steps since last call (positive or negative)
 *
 * @note Thread-safe: uses interrupt disable/restore for atomic access.
 * @note Call regularly to prevent counter overflow.
 */
int encoder_get_delta(encoder_t *enc);

#endif // ENCODER_H