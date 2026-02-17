/**
 * @file screen_2.c
 * @brief Screen 2 implementation
 */

#include "screen_2.h"
#include <stdio.h>

lv_obj_t *screen_2_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);

  // Set background to black
  lv_obj_set_style_bg_color(screen, lv_color_black(), 0);

  // Create label
  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text(label, "Screen 1");
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  printf("[Screen2] Created\n");
  return screen;
}

void screen_2_on_show(void) {
  printf("[Screen2] Showing\n");
}

void screen_2_encoder_rotate(int delta) {
  // Not used
}

void screen_2_on_hide(void) {
  printf("[Screen2] Hiding\n");
}
