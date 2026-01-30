#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include "../../drivers/encoder/encoder.h"
#include <stdint.h>

/**
 * @brief Initialize encoder manager
 *
 * Wraps encoder driver and posts events to event queue
 * @param gpio_a Encoder phase A pin
 * @param gpio_b Encoder phase B pin
 * @param gpio_sw Encoder button pin
 */
void encoder_manager_init(uint8_t gpio_a, uint8_t gpio_b, uint8_t gpio_sw);

/**
 * @brief Poll encoder and post events if changed
 *
 * Call this periodically or from timer to generate events
 */
void encoder_manager_poll(void);

/**
 * @brief Get raw encoder instance (advanced use)
 * @return Pointer to encoder structure
 */
encoder_t *encoder_manager_get_instance(void);

#endif
