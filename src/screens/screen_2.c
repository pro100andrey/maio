/**
 * @file screen_2.c
 * @brief Screen 2 implementation - Encoder Display
 */

#include "screen_2.h"
#include "../managers/encoder_manager.h"
#include <stdio.h>

static lv_obj_t *label_rotation = NULL;
static lv_obj_t *label_button = NULL;
static lv_obj_t *label_delta = NULL;
static lv_timer_t *update_timer = NULL;
static int accumulated_rotation = 0;
static int rotation_events = 0;

static void update_timer_cb(lv_timer_t *timer) {
  if (!label_rotation || !label_button || !label_delta) {
    return;
  }
  encoder_t *enc = encoder_manager_get_instance();

  // Update accumulated rotation count (from events)
  char buf[32];
  snprintf(buf, sizeof(buf), "Total: %d", accumulated_rotation);
  lv_label_set_text(label_rotation, buf);

  // Update rotation event counter
  snprintf(buf, sizeof(buf), "Events: %d", rotation_events);
  lv_label_set_text(label_delta, buf);

  // Update button state
  lv_label_set_text(label_button,
                    enc->btn_pressed ? "Button: PRESSED" : "Button: RELEASED");
}

lv_obj_t *screen_2_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);

  // Set background color
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x2E2E2E), 0);

  // Create title label
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "Encoder Monitor");
  lv_obj_set_style_text_color(title, lv_color_hex(0x00FF00), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  // Create rotation count label
  label_rotation = lv_label_create(screen);
  lv_label_set_text(label_rotation, "Total: 0");
  lv_obj_set_style_text_color(label_rotation, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(label_rotation, &lv_font_montserrat_32, 0);
  lv_obj_align(label_rotation, LV_ALIGN_CENTER, 0, -40);

  // Create rotation events label
  label_delta = lv_label_create(screen);
  lv_label_set_text(label_delta, "Events: 0");
  lv_obj_set_style_text_color(label_delta, lv_color_hex(0x00FFFF), 0);
  lv_obj_set_style_text_font(label_delta, &lv_font_montserrat_28, 0);
  lv_obj_align(label_delta, LV_ALIGN_CENTER, 0, 0);

  // Create button state label
  label_button = lv_label_create(screen);
  lv_label_set_text(label_button, "Button: RELEASED");
  lv_obj_set_style_text_color(label_button, lv_color_hex(0xFFFF00), 0);
  lv_obj_set_style_text_font(label_button, &lv_font_montserrat_28, 0);
  lv_obj_align(label_button, LV_ALIGN_CENTER, 0, 50);

  return screen;
}

void screen_2_on_show(void) {
  // Reset counters when screen becomes active
  accumulated_rotation = 0;
  rotation_events = 0;

  // Create timer when screen becomes active
  if (!update_timer) {
    update_timer = lv_timer_create(update_timer_cb, 33, NULL);
  }
}

void screen_2_encoder_rotate(int delta) {
  // Called when encoder rotation event occurs
  accumulated_rotation += delta;
  rotation_events++;
}

void screen_2_on_hide(void) {
  // Delete timer when screen is hidden
  if (update_timer) {
    lv_timer_delete(update_timer);
    update_timer = NULL;
  }
}
