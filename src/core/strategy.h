#ifndef STRATEGY_H
#define STRATEGY_H

#include "../shared/events.h"

/**
 * @brief Strategy interface for application modes
 *
 * Each mode (idle, measurement, menu) implements this interface
 * to handle events in mode-specific way
 */
typedef struct strategy_s {
  /** Mode name for debugging */
  const char *name;

  /** Called when entering this mode */
  void (*on_enter)(void);
  /** Called when leaving this mode */
  void (*on_exit)(void);
  /** Handle event in this mode */
  void (*on_event)(const event_t *event);
} strategy_t;

/**
 * @brief Get current active strategy
 * @return Pointer to current strategy
 */
const strategy_t *strategy_get_current(void);

/**
 * @brief Switch to a different strategy
 * @param strategy New strategy to activate
 */
void strategy_switch(const strategy_t *strategy);

/**
 * @brief Dispatch event to current strategy
 * @param event Event to dispatch
 */
void strategy_dispatch_event(const event_t *event);

#endif
