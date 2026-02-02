#ifndef IDLE_STRATEGY_H
#define IDLE_STRATEGY_H

#include "../core/strategy.h"

/**
 * @brief Idle/standby mode strategy
 *
 * Default idle state - minimal activity, waiting for user input.
 * - Shows simple idle screen
 * - Encoder rotation tracked but not acted upon
 * - Button press switches to test mode
 */
extern const strategy_t idle_strategy;

#endif
