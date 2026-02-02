#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include "../../drivers/encoder/encoder.h"
#include <stdint.h>

/**
 * @brief Initialize encoder manager
 *
 * Wraps encoder driver, creates internal 1ms timer for polling,
 * and posts events to event queue automatically.
 * @param gpio_a Encoder phase A pin
 * @param gpio_b Encoder phase B pin
 * @param gpio_sw Encoder button pin
 */
void encoder_manager_init(uint8_t gpio_a, uint8_t gpio_b, uint8_t gpio_sw);

/**
 * @brief Get raw encoder instance (advanced use)
 * @return Pointer to encoder structure
 */
encoder_t *encoder_manager_get_instance(void);

#endif
