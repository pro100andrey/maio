/**
 * @file themes.h
 * @brief Color theme definitions
 */

#ifndef THEMES_H
#define THEMES_H

#include <lvgl/lvgl.h>

/**
 * @brief Theme color palette
 */
typedef struct {
  // Backgrounds
  lv_color_t bg_primary;   // Main screen background
  lv_color_t bg_secondary; // Secondary background (cards, panels)

  // Text colors
  lv_color_t text_primary;   // Main text
  lv_color_t text_secondary; // Secondary text
  lv_color_t text_disabled;  // Disabled text

  // Accent colors
  lv_color_t accent_primary; // Primary accent (titles, highlights)
  lv_color_t accent_green;   // Success/data (voltmeter)
  lv_color_t accent_blue;    // Info
  lv_color_t accent_yellow;  // Warning
  lv_color_t accent_red;     // Error
} theme_colors_t;

/**
 * @brief Available theme IDs
 */
typedef enum { THEME_DARK = 0, THEME_LIGHT = 1, THEME_COUNT } theme_id_t;

// Dark theme colors
static const theme_colors_t THEME_DARK_COLORS = {
    .bg_primary = {.blue = 0x1E, .green = 0x1E, .red = 0x1E},   // 0x1E1E1E
    .bg_secondary = {.blue = 0x2E, .green = 0x2E, .red = 0x2E}, // 0x2E2E2E

    .text_primary = {.blue = 0xFF, .green = 0xFF, .red = 0xFF},   // 0xFFFFFF
    .text_secondary = {.blue = 0xAA, .green = 0xAA, .red = 0xAA}, // 0xAAAAAA
    .text_disabled = {.blue = 0x55, .green = 0x55, .red = 0x55},  // 0x555555

    .accent_primary = {.blue = 0xFF,
                       .green = 0xFF,
                       .red = 0x00}, // 0x00FFFF (cyan)
    .accent_green = {.blue = 0x00, .green = 0xFF, .red = 0x00},  // 0x00FF00
    .accent_blue = {.blue = 0xFF, .green = 0xFF, .red = 0x00},   // 0x00FFFF
    .accent_yellow = {.blue = 0x00, .green = 0xFF, .red = 0xFF}, // 0xFFFF00
    .accent_red = {.blue = 0x00, .green = 0x00, .red = 0xFF}     // 0xFF0000
};

// Light theme colors
static const theme_colors_t THEME_LIGHT_COLORS = {
    .bg_primary = {.blue = 0xF0, .green = 0xF0, .red = 0xF0},   // 0xF0F0F0
    .bg_secondary = {.blue = 0xFF, .green = 0xFF, .red = 0xFF}, // 0xFFFFFF

    .text_primary = {.blue = 0x20, .green = 0x20, .red = 0x20},   // 0x202020
    .text_secondary = {.blue = 0x60, .green = 0x60, .red = 0x60}, // 0x606060
    .text_disabled = {.blue = 0xA0, .green = 0xA0, .red = 0xA0},  // 0xA0A0A0

    .accent_primary = {.blue = 0xD0,
                       .green = 0x70,
                       .red = 0x00}, // 0x0070D0 (blue)
    .accent_green = {.blue = 0x00, .green = 0xA0, .red = 0x00},  // 0x00A000
    .accent_blue = {.blue = 0xFF, .green = 0x80, .red = 0x00},   // 0x0080FF
    .accent_yellow = {.blue = 0x00, .green = 0xB0, .red = 0xFF}, // 0xFFB000
    .accent_red = {.blue = 0x00, .green = 0x00, .red = 0xE0}     // 0xE00000
};

// Theme names
static const char *THEME_NAMES[THEME_COUNT] = {"Dark", "Light"};

#endif // THEMES_H
