/**
 * @file screen_settings.c
 * @brief Settings screen implementation - Navigation menu
 */

#include "screen_settings.h"
#include "../managers/screen_manager.h"
#include "../managers/theme_manager.h"
#include <stdio.h>

/** Menu items */
static lv_obj_t *label_theme = NULL;
static lv_obj_t *label_about = NULL;
static int selected_index = 0;

static void update_selection(void) {
  const theme_colors_t *colors = theme_manager_get_colors();

  // Theme item
  if (selected_index == 0) {
    lv_obj_set_style_text_color(label_theme, colors->accent_primary, 0);
    lv_obj_set_style_text_color(label_about, colors->text_secondary, 0);
  } else {
    lv_obj_set_style_text_color(label_theme, colors->text_secondary, 0);
    lv_obj_set_style_text_color(label_about, colors->accent_primary, 0);
  }
}

lv_obj_t *screen_settings_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  const theme_colors_t *colors = theme_manager_get_colors();

  // Set background color
  lv_obj_set_style_bg_color(screen, colors->bg_primary, 0);

  // Create title
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "Settings");
  lv_obj_set_style_text_color(title, colors->accent_green, 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  // Menu items
  label_theme = lv_label_create(screen);
  lv_label_set_text(label_theme, "Theme");
  lv_obj_set_style_text_font(label_theme, &lv_font_montserrat_28, 0);
  lv_obj_align(label_theme, LV_ALIGN_CENTER, 0, -20);

  label_about = lv_label_create(screen);
  lv_label_set_text(label_about, "About");
  lv_obj_set_style_text_font(label_about, &lv_font_montserrat_28, 0);
  lv_obj_align(label_about, LV_ALIGN_CENTER, 0, 20);

  // Help text
  lv_obj_t *note = lv_label_create(screen);
  lv_label_set_text(note, "Rotate to select\nPress to open");
  lv_obj_set_style_text_color(note, colors->text_disabled, 0);
  lv_obj_set_style_text_font(note, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_align(note, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(note, LV_ALIGN_BOTTOM_MID, 0, -10);

  return screen;
}

void screen_settings_update(void) {
  // Update selection highlight
  update_selection();
}

void screen_settings_on_show(void) {
  selected_index = 0;
  update_selection();
}

void screen_settings_on_hide(void) {
  // Nothing to do
}

void screen_settings_encoder_rotate(int delta) {
  selected_index += delta;

  // Wrap around (2 items: Theme, About)
  if (selected_index < 0) {
    selected_index = 1;
  } else if (selected_index > 1) {
    selected_index = 0;
  }

  update_selection();
}

void screen_settings_select(void) {
  // Navigate to selected screen
  if (selected_index == 0) {
    // Theme
    screen_manager_show(SCREEN_THEME);
  } else {
    // About
    screen_manager_show(SCREEN_ABOUT);
  }
}
