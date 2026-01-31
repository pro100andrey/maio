#include "idle_strategy.h"
#include "../../drivers/display/ili9341.h"
#include "../managers/display_manager.h"
#include <stdio.h>

static int position = 0;

// Color state
static uint8_t color_index = 0;
static const uint16_t colors[] = {0xF800, 0x07E0, 0x001F}; // Red, Green, Blue
static const char *color_names[] = {"RED", "GREEN", "BLUE"};

static void draw_screen(void) {
  ili9341_t *dev = display_manager_get_device();
  if (!dev)
    return;

  // Fill with current color
  printf("[Idle] Drawing %s screen\n", color_names[color_index]);
  ili9341_fill_screen(dev, colors[color_index]);

  // Always draw green diagonal line from (0,0) to bottom-right
  uint16_t w = ili9341_get_width(dev);
  uint16_t h = ili9341_get_height(dev);
  ili9341_draw_line(dev, 0, 0, w - 1, h - 1, 0x07E0);
}

static void idle_on_enter(void) {
  printf("[Idle] Mode activated\n");
  position = 0;
  color_index = 0;
  draw_screen();
}

static void idle_on_exit(void) { printf("[Idle] Mode deactivated\n"); }

static void idle_on_event(const event_t *event) {
  ili9341_t *dev = display_manager_get_device();

  switch (event->type) {
  case EV_ENCODER_ROTATE:
    position += event->payload;
    printf("[Idle] Encoder position: %d (delta: %d)\n", position,
           event->payload);

    // Change color on rotation
    if (event->payload > 0) {
      color_index = (color_index + 1) % 3;
    } else {
      color_index = (color_index + 2) % 3; // +2 is same as -1 in mod 3
    }
    draw_screen();
    break;

  case EV_ENCODER_BUTTON:
    printf("[Idle] Button event: payload=%d\n", event->payload);

    // Rotate orientation on button press (payload == 1 means pressed)
    if (dev && event->payload == 1) {
      ili9341_orientation_t current = ili9341_get_orientation(dev);
      ili9341_orientation_t next = (current + 1) % 4;
      printf("[Idle] Switching orientation %d -> %d\n", current, next);
      ili9341_set_orientation(dev, next);
      draw_screen();
    }
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
