/**
 * @file theme_manager.c
 * @brief Theme management implementation
 */

#include "theme_manager.h"
#include "screen_manager.h"
#include <stdio.h>

/** Current active theme */
static theme_id_t current_theme = THEME_DARK;

/** Pointer to current theme colors */
static const theme_colors_t *current_colors = &THEME_DARK_COLORS;

void theme_manager_init(void) {
  current_theme = THEME_DARK;
  current_colors = &THEME_DARK_COLORS;
  printf("[ThemeMgr] Initialized with theme: %s\n", THEME_NAMES[current_theme]);
}

theme_id_t theme_manager_get_current(void) { return current_theme; }

void theme_manager_set_theme(theme_id_t theme) {
  if (theme >= THEME_COUNT) {
    printf("[ThemeMgr] Invalid theme ID: %d\n", theme);
    return;
  }

  current_theme = theme;

  // Update color pointer
  switch (theme) {
  case THEME_DARK:
    current_colors = &THEME_DARK_COLORS;
    break;
  case THEME_LIGHT:
    current_colors = &THEME_LIGHT_COLORS;
    break;
  default:
    current_colors = &THEME_DARK_COLORS;
    break;
  }

  printf("[ThemeMgr] Theme changed to: %s\n", THEME_NAMES[theme]);
}

const theme_colors_t *theme_manager_get_colors(void) { return current_colors; }

const char *theme_manager_get_name(theme_id_t theme) {
  if (theme >= THEME_COUNT) {
    return "Unknown";
  }
  return THEME_NAMES[theme];
}

void theme_manager_apply_to_screens(void) {
  printf("[ThemeMgr] Applying theme to all screens...\n");
  screen_manager_recreate_screens();
}
