/**
 * @file screen_theme.c
 * @brief Theme selection screen implementation
 */

#include "screen_theme.h"
#include "../managers/theme_manager.h"
#include <stdio.h>

static lv_obj_t *label_title;
static lv_obj_t *label_themes[THEME_COUNT];
static int selected_index = 0;

static void update_selection(void) {
  const theme_colors_t *colors = theme_manager_get_colors();

  for (int i = 0; i < THEME_COUNT; i++) {
    if (i == selected_index) {
      // Highlight selected
      lv_obj_set_style_text_color(label_themes[i], colors->accent_primary, 0);
    } else {
      // Normal text
      lv_obj_set_style_text_color(label_themes[i], colors->text_secondary, 0);
    }
  }
}

lv_obj_t *screen_theme_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  const theme_colors_t *colors = theme_manager_get_colors();

  // Set background
  lv_obj_set_style_bg_color(screen, colors->bg_primary, 0);

  // Title
  label_title = lv_label_create(screen);
  lv_label_set_text(label_title, "Select Theme");
  lv_obj_set_style_text_color(label_title, colors->accent_green, 0);
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_28, 0);
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 20);

  // Theme list
  for (int i = 0; i < THEME_COUNT; i++) {
    label_themes[i] = lv_label_create(screen);
    lv_label_set_text(label_themes[i], theme_manager_get_name(i));
    lv_obj_set_style_text_font(label_themes[i], &lv_font_montserrat_28, 0);
    lv_obj_align(label_themes[i], LV_ALIGN_CENTER, 0, -20 + (i * 40));
  }

  // Back button
  lv_obj_t *label_back = lv_label_create(screen);
  lv_label_set_text(label_back, "Press to apply");
  lv_obj_set_style_text_color(label_back, colors->text_disabled, 0);
  lv_obj_set_style_text_font(label_back, &lv_font_montserrat_14, 0);
  lv_obj_align(label_back, LV_ALIGN_BOTTOM_MID, 0, -10);

  return screen;
}

void screen_theme_on_show(void) {
  // Set selection to current theme
  selected_index = theme_manager_get_current();
  update_selection();
}

void screen_theme_on_hide(void) {
  // Apply selected theme
  theme_manager_set_theme(selected_index);
  theme_manager_apply_to_screens();
}

void screen_theme_encoder_rotate(int delta) {
  selected_index += delta;

  // Wrap around
  if (selected_index < 0) {
    selected_index = THEME_COUNT - 1;
  } else if (selected_index >= THEME_COUNT) {
    selected_index = 0;
  }

  update_selection();
}
