/**
 * @file theme_manager.h
 * @brief Theme management system
 */

#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include "../shared/themes.h"

/**
 * @brief Initialize theme manager
 * @note Call once during UI initialization
 */
void theme_manager_init(void);

/**
 * @brief Get current theme ID
 * @return Current theme ID
 */
theme_id_t theme_manager_get_current(void);

/**
 * @brief Set active theme
 * @param theme Theme ID to activate
 */
void theme_manager_set_theme(theme_id_t theme);

/**
 * @brief Get color palette for current theme
 * @return Pointer to current theme colors
 */
const theme_colors_t *theme_manager_get_colors(void);

/**
 * @brief Get theme name
 * @param theme Theme ID
 * @return Theme name string
 */
const char *theme_manager_get_name(theme_id_t theme);

/**
 * @brief Apply current theme to all screens
 * @note Triggers screen manager to recreate all screens
 */
void theme_manager_apply_to_screens(void);

#endif // THEME_MANAGER_H
