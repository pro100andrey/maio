/**
 * @file screen_settings.c
 * @brief Settings screen implementation
 */

#include "screen_settings.h"
#include "../../drivers/display/ili9341.h"
#include "../managers/display_manager.h"
#include <stdio.h>

/** Labels for dynamic content */
static lv_obj_t *label_brightness = NULL;
static lv_obj_t *label_fps = NULL;
static lv_obj_t *label_resolution = NULL;

lv_obj_t *screen_settings_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);

  // Set background color
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x3E2E1E), 0);

  // Create title
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "Settings");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  // Create settings list
  int y_offset = 40;

  // Brightness setting
  lv_obj_t *label_bright_title = lv_label_create(screen);
  lv_label_set_text(label_bright_title, "Backlight:");
  lv_obj_set_style_text_color(label_bright_title, lv_color_hex(0xCCCCCC), 0);
  lv_obj_align(label_bright_title, LV_ALIGN_TOP_LEFT, 10, y_offset);

  label_brightness = lv_label_create(screen);
  lv_label_set_text(label_brightness, "100%");
  lv_obj_set_style_text_color(label_brightness, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(label_brightness, LV_ALIGN_TOP_RIGHT, -10, y_offset);

  y_offset += 25;

  // FPS setting
  lv_obj_t *label_fps_title = lv_label_create(screen);
  lv_label_set_text(label_fps_title, "Refresh:");
  lv_obj_set_style_text_color(label_fps_title, lv_color_hex(0xCCCCCC), 0);
  lv_obj_align(label_fps_title, LV_ALIGN_TOP_LEFT, 10, y_offset);

  label_fps = lv_label_create(screen);
  lv_label_set_text(label_fps, "60 FPS");
  lv_obj_set_style_text_color(label_fps, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(label_fps, LV_ALIGN_TOP_RIGHT, -10, y_offset);

  y_offset += 25;

  // Resolution setting
  lv_obj_t *label_res_title = lv_label_create(screen);
  lv_label_set_text(label_res_title, "Display:");
  lv_obj_set_style_text_color(label_res_title, lv_color_hex(0xCCCCCC), 0);
  lv_obj_align(label_res_title, LV_ALIGN_TOP_LEFT, 10, y_offset);

  label_resolution = lv_label_create(screen);
  lv_label_set_text(label_resolution, "320x240");
  lv_obj_set_style_text_color(label_resolution, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(label_resolution, LV_ALIGN_TOP_RIGHT, -10, y_offset);

  y_offset += 25;

  // SPI Speed setting
  lv_obj_t *label_spi_title = lv_label_create(screen);
  lv_label_set_text(label_spi_title, "SPI:");
  lv_obj_set_style_text_color(label_spi_title, lv_color_hex(0xCCCCCC), 0);
  lv_obj_align(label_spi_title, LV_ALIGN_TOP_LEFT, 10, y_offset);

  lv_obj_t *label_spi = lv_label_create(screen);
  lv_label_set_text(label_spi, "75 MHz");
  lv_obj_set_style_text_color(label_spi, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(label_spi, LV_ALIGN_TOP_RIGHT, -10, y_offset);

  // Note at bottom
  lv_obj_t *note = lv_label_create(screen);
  lv_label_set_text(note, "Long press to exit");
  lv_obj_set_style_text_color(note, lv_color_hex(0x888888), 0);
  lv_obj_align(note, LV_ALIGN_BOTTOM_MID, 0, -10);

  return screen;
}

void screen_settings_update(void) {
  if (!label_brightness || !label_fps || !label_resolution) {
    return;
  }

  // Update brightness (example - would get from driver)
  ili9341_t *dev = display_manager_get_device();
  if (dev) {
    static char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", dev->backlight_level * 100 / 255);
    lv_label_set_text(label_brightness, buf);

    // Update resolution
    snprintf(buf, sizeof(buf), "%dx%d", ili9341_get_width(dev),
             ili9341_get_height(dev));
    lv_label_set_text(label_resolution, buf);
  }

  // FPS is constant from lv_conf.h (16ms = ~60 FPS)
  lv_label_set_text(label_fps, "60 FPS");
}
