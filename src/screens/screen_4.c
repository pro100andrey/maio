/**
 * @file screen_4.c
 * @brief Screen 4 with colored squares
 */

#include "screen_4.h"
#include "../../drivers/display/ili9341.h"
#include "../managers/display_manager.h"
#include "../managers/theme_manager.h"

lv_obj_t *screen_4_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  const theme_colors_t *colors = theme_manager_get_colors();

  // Set background color
  lv_obj_set_style_bg_color(screen, colors->bg_primary, 0);

  // Get display dimensions
  ili9341_t *dev = display_manager_get_device();
  if (!dev) {
    return screen;
  }

  uint16_t width = ili9341_get_width(dev);   // 320
  uint16_t height = ili9341_get_height(dev); // 240

  // Calculate square dimensions (2x2 grid)
  uint16_t square_w = width / 2;  // 160
  uint16_t square_h = height / 2; // 120

  // Top-left: RED
  lv_obj_t *square_red = lv_obj_create(screen);
  lv_obj_remove_style_all(square_red);
  lv_obj_set_size(square_red, square_w, square_h);
  lv_obj_set_pos(square_red, 0, 0);
  lv_obj_set_style_bg_color(square_red, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_bg_opa(square_red, LV_OPA_COVER, 0);

  // Top-right: GREEN
  lv_obj_t *square_green = lv_obj_create(screen);
  lv_obj_remove_style_all(square_green);
  lv_obj_set_size(square_green, square_w, square_h);
  lv_obj_set_pos(square_green, square_w, 0);
  lv_obj_set_style_bg_color(square_green, lv_color_hex(0x00FF00), 0);
  lv_obj_set_style_bg_opa(square_green, LV_OPA_COVER, 0);

  // Bottom-left: BLUE
  lv_obj_t *square_blue = lv_obj_create(screen);
  lv_obj_remove_style_all(square_blue);
  lv_obj_set_size(square_blue, square_w, square_h);
  lv_obj_set_pos(square_blue, 0, square_h);
  lv_obj_set_style_bg_color(square_blue, lv_color_hex(0x0000FF), 0);
  lv_obj_set_style_bg_opa(square_blue, LV_OPA_COVER, 0);

  // Bottom-right: WHITE
  lv_obj_t *square_white = lv_obj_create(screen);
  lv_obj_remove_style_all(square_white);
  lv_obj_set_size(square_white, square_w, square_h);
  lv_obj_set_pos(square_white, square_w, square_h);
  lv_obj_set_style_bg_color(square_white, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_bg_opa(square_white, LV_OPA_COVER, 0);

  return screen;
}
