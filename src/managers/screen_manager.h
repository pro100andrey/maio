/**
 * @file screen_manager.h
 * @brief LVGL screen manager for multi-screen navigation
 */

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <lvgl/lvgl.h>
#include <stdint.h>

/**
 * @brief Screen IDs for navigation
 */
typedef enum { SCREEN_1 = 0, SCREEN_2 = 1, SCREEN_COUNT = 2 } screen_id_t;

/**
 * @brief Initialize screen manager and create all screens
 */
void screen_manager_init(void);

/**
 * @brief Get current screen ID
 * @return Current active screen ID
 */
screen_id_t screen_manager_get_current(void);

/**
 * @brief Switch to specific screen
 * @param screen_id Target screen ID
 */
void screen_manager_show(screen_id_t screen_id);

/**
 * @brief Show next screen (circular)
 */
void screen_manager_next(void);

/**
 * @brief Show previous screen (circular)
 */
void screen_manager_prev(void);

/**
 * @brief Update screen content (if needed)
 */
void screen_manager_update(void);

/**
 * @brief Handle encoder rotation on current screen
 * @param direction Rotation direction (positive = clockwise, negative =
 * counter-clockwise)
 */
void screen_manager_encoder_rotate(int direction);

/**
 * @brief Recreate all screens (used when theme changes)
 */
void screen_manager_recreate_screens(void);

#endif // SCREEN_MANAGER_H
