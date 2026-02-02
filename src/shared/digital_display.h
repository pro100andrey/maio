#ifndef DIGITAL_DISPLAY_H
#define DIGITAL_DISPLAY_H

#include <lvgl/lvgl.h>
#include <stdint.h>

/**
 * @brief Digital display component - emulates monospace font
 *
 * Creates multiple labels with fixed positions for each digit/character
 * to prevent "dancing" numbers when values change
 */
typedef struct {
  lv_obj_t *container;     // Parent container
  lv_obj_t **digit_labels; // Array of labels for each position
  uint8_t digit_count;     // Number of digit positions
  uint8_t spacing;         // Spacing between characters (pixels)
  const char *prefix;      // Prefix string (e.g. "1.")
  const char *suffix;      // Suffix string (e.g. " V")
  const lv_font_t *font;   // Font to use
  lv_color_t color;        // Text color
} digital_display_t;

/**
 * @brief Create digital display component
 *
 * @param parent Parent object
 * @param digit_count Number of digit positions
 * @param prefix Prefix string (can be NULL)
 * @param suffix Suffix string (can be NULL)
 * @param spacing Spacing between characters in pixels
 * @param font Font to use
 * @param color Text color
 * @return digital_display_t* Created display component
 */
digital_display_t *digital_display_create(lv_obj_t *parent, uint8_t digit_count,
                                          const char *prefix,
                                          const char *suffix, uint8_t spacing,
                                          const lv_font_t *font,
                                          lv_color_t color);

/**
 * @brief Set value of digital display
 *
 * @param display Display component
 * @param value Value to display (will be formatted according to digit_count)
 */
void digital_display_set_value(digital_display_t *display, int value);

/**
 * @brief Set position of digital display
 *
 * @param display Display component
 * @param align Alignment mode
 * @param x_offset X offset
 * @param y_offset Y offset
 */
void digital_display_align(digital_display_t *display, lv_align_t align,
                           int16_t x_offset, int16_t y_offset);

/**
 * @brief Destroy digital display component
 *
 * @param display Display component to destroy
 */
void digital_display_destroy(digital_display_t *display);

#endif // DIGITAL_DISPLAY_H
