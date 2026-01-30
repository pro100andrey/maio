#include "idle_strategy.h"
#include <stdio.h>

static int position = 0;

static void idle_on_enter(void) {
  printf("[Idle] Mode activated\n");
  position = 0;
}

static void idle_on_exit(void) { printf("[Idle] Mode deactivated\n"); }

static void idle_on_event(const event_t *event) {
  switch (event->type) {
  case EV_ENCODER_ROTATE:
    position += event->payload;
    printf("[Idle] Encoder position: %d (delta: %d)\n", position,
           event->payload);
    break;

  case EV_ENCODER_BUTTON:
    printf("[Idle] Button pressed at position: %d\n", position);
    position = 0; // Reset on button press
    break;

  default:
    break;
  }
}

const strategy_t idle_strategy = {
    .name = "Idle",
    .on_enter = idle_on_enter,
    .on_exit = idle_on_exit,
    .on_event = idle_on_event,
};
