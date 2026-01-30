#include "strategy.h"
#include <stdio.h>

/** Currently active strategy */
static const strategy_t *current_strategy = NULL;

const strategy_t *strategy_get_current(void) { return current_strategy; }

void strategy_switch(const strategy_t *strategy) {
  if (!strategy) {
    printf("[Strategy] Error: NULL strategy\n");
    return;
  }

  // Exit current strategy
  if (current_strategy && current_strategy->on_exit) {
    printf("[Strategy] Exiting: %s\n", current_strategy->name);
    current_strategy->on_exit();
  }

  // Switch to new strategy
  current_strategy = strategy;

  // Enter new strategy
  if (current_strategy->on_enter) {
    printf("[Strategy] Entering: %s\n", current_strategy->name);
    current_strategy->on_enter();
  }
}

void strategy_dispatch_event(const event_t *event) {
  if (!event) {
    return;
  }

  if (current_strategy && current_strategy->on_event) {
    current_strategy->on_event(event);
  }
}
