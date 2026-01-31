#include "event_manager.h"

#include <hardware/sync.h>
#include <pico/platform.h>
#include <pico/time.h>
#include <stdio.h>

/** Global event queue */
static queue_t event_queue;

void event_manager_init(void) {
  queue_init(&event_queue, sizeof(event_t), EVENT_QUEUE_SIZE);
  printf("[EventManager] Initialized with queue size: %d\n", EVENT_QUEUE_SIZE);
}

bool event_manager_post(const event_t *event) {
  if (!event) {
    return false;
  }

  return queue_try_add(&event_queue, event);
}

void event_manager_wait(event_t *event) {
  if (!event) {
    return;
  }

  // This blocks and puts CPU in WFI (Wait For Interrupt) state
  queue_remove_blocking(&event_queue, event);
}

bool event_manager_wait_timeout(event_t *event, uint32_t timeout_ms) {
  if (!event) {
    return false;
  }

  // Try to get event with timeout
  absolute_time_t timeout = make_timeout_time_ms(timeout_ms);

  while (!time_reached(timeout)) {
    if (queue_try_remove(&event_queue, event)) {
      return true;
    }
    // Use WFI to sleep until interrupt (use builtin to avoid declaration
    // issues)
    __asm__ volatile("wfi");
  }

  return false;
}

bool event_manager_try_get(event_t *event) {
  if (!event) {
    return false;
  }

  return queue_try_remove(&event_queue, event);
}

queue_t *event_manager_get_queue(void) { return &event_queue; }
