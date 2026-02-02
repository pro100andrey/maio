#ifndef TEST_STRATEGY_H
#define TEST_STRATEGY_H

#include "../core/strategy.h"

/**
 * @brief Display test strategy
 *
 * Test mode for display driver - cycles through various demo scenes
 * to verify display functionality (sync/async fills, DMA, orientation, etc.)
 */
extern const strategy_t test_strategy;

#endif
