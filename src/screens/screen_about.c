/**
 * @file screen_about.c
 * @brief About screen implementation
 */

#include "screen_about.h"
#include "../managers/theme_manager.h"

lv_obj_t *screen_about_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  const theme_colors_t *colors = theme_manager_get_colors();

  // Set background
  lv_obj_set_style_bg_color(screen, colors->bg_primary, 0);

  // Title
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "Maio");
  lv_obj_set_style_text_color(title, colors->accent_primary, 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_40, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

  // Version
  lv_obj_t *version = lv_label_create(screen);
  lv_label_set_text(version, "Version 0.1");
  lv_obj_set_style_text_color(version, colors->text_secondary, 0);
  lv_obj_set_style_text_font(version, &lv_font_montserrat_14, 0);
  lv_obj_align(version, LV_ALIGN_TOP_MID, 0, 80);

  // Hardware info
  lv_obj_t *hw_info = lv_label_create(screen);
  lv_label_set_text(hw_info,
                    "Hardware:\nRaspberry Pi Pico 2W\nILI9341 Display");
  lv_obj_set_style_text_color(hw_info, colors->text_primary, 0);
  lv_obj_set_style_text_font(hw_info, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_align(hw_info, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(hw_info, LV_ALIGN_CENTER, 0, 0);

  // Footer
  lv_obj_t *footer = lv_label_create(screen);
  lv_label_set_text(footer, "Press button to return");
  lv_obj_set_style_text_color(footer, colors->text_disabled, 0);
  lv_obj_set_style_text_font(footer, &lv_font_montserrat_14, 0);
  lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -10);

  return screen;
}
