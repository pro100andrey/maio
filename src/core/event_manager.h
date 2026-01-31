#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "../shared/events.h"
#include <pico/util/queue.h>
#include <stdbool.h>

/**
 * @brief Initialize event manager and queue
 */
void event_manager_init(void);

/**
 * @brief Post event to the queue (thread-safe)
 * @param event Event to post
 * @return true if posted successfully, false if queue full
 */
bool event_manager_post(const event_t *event);

/**
 * @brief Wait for next event (blocking, uses WFI internally)
 * @param event Pointer to receive event
 */
void event_manager_wait(event_t *event);

/**
 * @brief Wait for event with timeout (for LVGL integration)
 * @param event Pointer to receive event
 * @param timeout_ms Maximum time to wait in milliseconds
 * @return true if event received, false if timeout expired
 */
bool event_manager_wait_timeout(event_t *event, uint32_t timeout_ms);

/**
 * @brief Try to get event without blocking
 * @param event Pointer to receive event
 * @return true if event received, false if queue empty
 */
bool event_manager_try_get(event_t *event);

/**
 * @brief Get reference to global event queue (for advanced use)
 * @return Pointer to queue
 */
queue_t *event_manager_get_queue(void);

#endif
