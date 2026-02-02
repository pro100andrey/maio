#include "digital_display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

digital_display_t *digital_display_create(lv_obj_t *parent, uint8_t digit_count,
                                          const char *prefix,
                                          const char *suffix, uint8_t spacing,
                                          const lv_font_t *font,
                                          lv_color_t color) {
  // Allocate display structure
  digital_display_t *display =
      (digital_display_t *)malloc(sizeof(digital_display_t));
  if (!display)
    return NULL;

  display->digit_count = digit_count;
  display->spacing = spacing;
  display->prefix = prefix;
  display->suffix = suffix;
  display->font = font;
  display->color = color;

  // Create container
  display->container = lv_obj_create(parent);
  lv_obj_set_size(display->container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(display->container, 0, 0); // Transparent background
  lv_obj_set_style_border_width(display->container, 0, 0); // No border
  lv_obj_set_style_pad_all(display->container, 0, 0);      // No padding

  // Calculate total number of labels (prefix + digits + suffix)
  uint8_t prefix_len = prefix ? strlen(prefix) : 0;
  uint8_t suffix_len = suffix ? strlen(suffix) : 0;
  uint8_t total_labels = prefix_len + digit_count + suffix_len;

  // Allocate label array
  display->digit_labels =
      (lv_obj_t **)malloc(sizeof(lv_obj_t *) * total_labels);
  if (!display->digit_labels) {
    free(display);
    return NULL;
  }

  // Create labels for each position
  int16_t x_pos = 0;
  uint8_t label_idx = 0;

  // Create prefix labels
  if (prefix) {
    for (uint8_t i = 0; i < prefix_len; i++) {
      display->digit_labels[label_idx] = lv_label_create(display->container);
      lv_obj_set_style_text_font(display->digit_labels[label_idx], font, 0);
      lv_obj_set_style_text_color(display->digit_labels[label_idx], color, 0);

      char ch[2] = {prefix[i], '\0'};
      lv_label_set_text(display->digit_labels[label_idx], ch);
      lv_obj_set_pos(display->digit_labels[label_idx], x_pos, 0);

      x_pos += spacing;
      label_idx++;
    }
  }

  // Create digit labels
  for (uint8_t i = 0; i < digit_count; i++) {
    display->digit_labels[label_idx] = lv_label_create(display->container);
    lv_obj_set_style_text_font(display->digit_labels[label_idx], font, 0);
    lv_obj_set_style_text_color(display->digit_labels[label_idx], color, 0);
    lv_label_set_text(display->digit_labels[label_idx], "0");
    lv_obj_set_pos(display->digit_labels[label_idx], x_pos, 0);

    x_pos += spacing;
    label_idx++;
  }

  // Create suffix labels
  if (suffix) {
    for (uint8_t i = 0; i < suffix_len; i++) {
      display->digit_labels[label_idx] = lv_label_create(display->container);
      lv_obj_set_style_text_font(display->digit_labels[label_idx], font, 0);
      lv_obj_set_style_text_color(display->digit_labels[label_idx], color, 0);

      char ch[2] = {suffix[i], '\0'};
      lv_label_set_text(display->digit_labels[label_idx], ch);
      lv_obj_set_pos(display->digit_labels[label_idx], x_pos, 0);

      x_pos += spacing;
      label_idx++;
    }
  }

  return display;
}

void digital_display_set_value(digital_display_t *display, int value) {
  if (!display)
    return;

  // Format value with leading zeros
  char format_str[16];
  snprintf(format_str, sizeof(format_str), "%%0%dd", display->digit_count);

  char value_str[16];
  snprintf(value_str, sizeof(value_str), format_str, value);

  // Update only digit labels (skip prefix)
  uint8_t prefix_len = display->prefix ? strlen(display->prefix) : 0;

  for (uint8_t i = 0; i < display->digit_count; i++) {
    char ch[2] = {value_str[i], '\0'};
    lv_label_set_text(display->digit_labels[prefix_len + i], ch);
  }
}

void digital_display_align(digital_display_t *display, lv_align_t align,
                           int16_t x_offset, int16_t y_offset) {
  if (!display || !display->container)
    return;
  lv_obj_align(display->container, align, x_offset, y_offset);
}

void digital_display_destroy(digital_display_t *display) {
  if (!display)
    return;

  if (display->digit_labels) {
    free(display->digit_labels);
  }

  if (display->container) {
    lv_obj_del(display->container);
  }

  free(display);
}
