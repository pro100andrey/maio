#ifndef SCREEN_MULTIMETER_H
#define SCREEN_MULTIMETER_H

#include <lvgl/lvgl.h>

/**
 * @brief Create multimeter screen
 * @return Pointer to created screen object
 */
lv_obj_t *screen_multimeter_create(void);

/**
 * @brief Update multimeter screen (called periodically)
 */
void screen_multimeter_update(void);

/**
 * @brief Handle encoder rotation on multimeter screen
 * @param direction Rotation direction (positive = clockwise, negative =
 * counter-clockwise)
 */
void screen_multimeter_encoder_rotate(int direction);

/**
 * @brief Called when entering multimeter screen
 */
void screen_multimeter_on_enter(void);

/**
 * @brief Called when exiting multimeter screen
 */
void screen_multimeter_on_exit(void);

#endif // SCREEN_MULTIMETER_H
