#include "idle_strategy.h"
#include "../managers/display_manager.h"
#include <lvgl.h>
#include <stdio.h>

static int position = 0;

/** LVGL screen for this strategy */
static lv_obj_t *screen = NULL;

/** LVGL label widget for encoder position */
static lv_obj_t *label_position = NULL;
static lv_obj_t *label_title = NULL;

static void idle_on_enter(void) {
  printf("[Idle] Mode activated\n");
  position = 0;

  // Create LVGL screen
  screen = lv_obj_create(NULL);

  // Create title label
  label_title = lv_label_create(screen);
  lv_label_set_text(label_title, "MAIO - Idle Mode");
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_14, 0);

  // Create position label
  label_position = lv_label_create(screen);
  lv_label_set_text_fmt(label_position, "Position: %d", position);
  lv_obj_align(label_position, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(label_position, &lv_font_montserrat_14, 0);

  // Load screen
  lv_screen_load(screen);
  printf("[Idle] LVGL screen created and loaded\n");
}

static void idle_on_exit(void) {
  printf("[Idle] Mode deactivated\n");

  // Delete LVGL screen and all widgets
  if (screen) {
    lv_obj_del(screen);
    screen = NULL;
    label_position = NULL;
    label_title = NULL;
  }
}

static void idle_on_event(const event_t *event) {
  switch (event->type) {
  case EV_ENCODER_ROTATE:
    position += event->payload;
    printf("[Idle] Encoder position: %d (delta: %d)\n", position,
           event->payload);

    // Update LVGL label
    if (label_position) {
      lv_label_set_text_fmt(label_position, "Position: %d", position);
    }
    break;

  case EV_ENCODER_BUTTON:
    printf("[Idle] Button pressed at position: %d\n", position);
    position = 0; // Reset on button press

    // Update LVGL label
    if (label_position) {
      lv_label_set_text_fmt(label_position, "Position: %d", position);
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
