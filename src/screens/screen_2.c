/**
 * @file screen_2.c
 * @brief Screen 2 implementation
 */

#include "screen_2.h"

lv_obj_t *screen_2_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);

  // Set background color (different from screen 1)
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x2E2E2E), 0);

  // Create title label
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "Screen 2");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
  lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

  // Create bar as example widget
  lv_obj_t *bar = lv_bar_create(screen);
  lv_obj_set_size(bar, 200, 20);
  lv_bar_set_value(bar, 65, LV_ANIM_OFF);
  lv_obj_align(bar, LV_ALIGN_CENTER, 0, 40);

  return screen;
}
