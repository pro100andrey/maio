#include "event_manager.h"
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

bool event_manager_try_get(event_t *event) {
  if (!event) {
    return false;
  }

  return queue_try_remove(&event_queue, event);
}

queue_t *event_manager_get_queue(void) { return &event_queue; }
