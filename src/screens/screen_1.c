/**
 * @file screen_1.c
 * @brief Screen 1 implementation
 */

#include "screen_1.h"

lv_obj_t *screen_1_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);

  // Set background color
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x1E1E1E), 0);

  // Create title label
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "Screen 1");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
  lv_obj_align(title, LV_ALIGN_CENTER, 0, -20);

  // Create subtitle with instructions
  lv_obj_t *subtitle = lv_label_create(screen);
  lv_label_set_text(subtitle, "Rotate encoder\nto switch screens");
  lv_obj_set_style_text_color(subtitle, lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_style_text_align(subtitle, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(subtitle, LV_ALIGN_CENTER, 0, 20);

  return screen;
}
