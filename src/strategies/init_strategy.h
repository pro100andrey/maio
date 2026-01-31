/**
 * @file init_strategy.h
 * @brief Initialization strategy for non-blocking display setup
 *
 * This strategy handles async hardware initialization and LVGL setup,
 * then switches to idle_strategy when ready.
 */

#ifndef INIT_STRATEGY_H
#define INIT_STRATEGY_H

#include "../core/strategy.h"

/**
 * @brief Initialization strategy
 *
 * Handles non-blocking display initialization via EV_TIMER_TICK events
 */
extern const strategy_t init_strategy;

#endif // INIT_STRATEGY_H
