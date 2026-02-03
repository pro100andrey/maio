/**
 * @file screen_3.c
 * @brief Screen 3 implementation
 */

#include "screen_3.h"
#include "../managers/theme_manager.h"

lv_obj_t *screen_3_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);

  // Set background color
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x1E2E3E), 0);

  // Create title label
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "Screen 3");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
  lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

  // Create slider as example widget
  lv_obj_t *slider = lv_slider_create(screen);
  lv_obj_set_size(slider, 180, 10);
  lv_slider_set_value(slider, 50, LV_ANIM_OFF);
  lv_obj_align(slider, LV_ALIGN_CENTER, 0, 40);

  return screen;
}
