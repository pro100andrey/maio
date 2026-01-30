#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>

/**
 * @brief Event types for the event-driven dispatcher
 */
typedef enum {
  /** No event */
  EV_NONE = 0,
  /** Encoder rotated (payload: delta) */
  EV_ENCODER_ROTATE,
  /** Encoder button pressed */
  EV_ENCODER_BUTTON,
  /** Periodic timer tick */
  EV_TIMER_TICK,
  /** Data ready from sensor/ADC */
  EV_DATA_READY,
} event_type_t;

/**
 * @brief Event structure with type and payload
 */
typedef struct {
  /** Event type */
  event_type_t type;
  /** Event payload (delta, value, etc) */
  int32_t payload;
} event_t;

#endif