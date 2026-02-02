/**
 * @file screen_manager.c
 * @brief LVGL screen manager implementation
 */

#include "screen_manager.h"
#include "../screens/screen_1.h"
#include "../screens/screen_2.h"
#include "../screens/screen_3.h"
#include "../screens/screen_4.h"
#include "../screens/screen_multimeter.h"
#include "../screens/screen_settings.h"
#include <stdio.h>

/** Array of all screens */
static lv_obj_t *screens[SCREEN_COUNT] = {NULL};

/** Current active screen index */
static screen_id_t current_screen = SCREEN_1;

void screen_manager_init(void) {
  printf("[ScreenMgr] Initializing screens...\n");

  // Create all screens
  screens[SCREEN_1] = screen_1_create();
  screens[SCREEN_2] = screen_2_create();
  screens[SCREEN_3] = screen_3_create();
  screens[SCREEN_4] = screen_4_create();
  screens[SCREEN_MULTIMETER] = screen_multimeter_create();
  screens[SCREEN_SETTINGS] = screen_settings_create();

  // Show first screen
  lv_screen_load(screens[SCREEN_1]);
  current_screen = SCREEN_1;

  printf("[ScreenMgr] Initialized with %d screens\n", SCREEN_COUNT);
}

screen_id_t screen_manager_get_current(void) { return current_screen; }

void screen_manager_show(screen_id_t screen_id) {
  if (screen_id >= SCREEN_COUNT || screens[screen_id] == NULL) {
    printf("[ScreenMgr] Invalid screen ID: %d\n", screen_id);
    return;
  }

  if (screen_id == current_screen) {
    return; // Already showing this screen
  }

  printf("[ScreenMgr] Switching to screen %d\n", screen_id);

  // Exit callback for previous screen
  if (current_screen == SCREEN_MULTIMETER) {
    screen_multimeter_on_exit();
  }

  // Update screen content before showing
  switch (screen_id) {
  case SCREEN_MULTIMETER:
    screen_multimeter_on_enter();
    break;
  case SCREEN_SETTINGS:
    screen_settings_update();
    break;
  default:
    break;
  }

  // Load screen with fade animation
  lv_screen_load_anim(screens[screen_id], LV_SCR_LOAD_ANIM_FADE_IN, 200, 0,
                      false);
  current_screen = screen_id;
}

void screen_manager_next(void) {
  screen_id_t next = (current_screen + 1) % SCREEN_COUNT;
  screen_manager_show(next);
}

void screen_manager_prev(void) {
  screen_id_t prev = (current_screen + SCREEN_COUNT - 1) % SCREEN_COUNT;
  screen_manager_show(prev);
}

void screen_manager_update(void) {
  // Update current screen if needed
  switch (current_screen) {
  case SCREEN_MULTIMETER:
    screen_multimeter_update();
    break;
  case SCREEN_SETTINGS:
    screen_settings_update();
    break;
  default:
    break;
  }
}

void screen_manager_encoder_rotate(int direction) {
  // Handle encoder rotation based on current screen
  if (current_screen == SCREEN_MULTIMETER) {
    screen_multimeter_encoder_rotate(direction);
  }
  // Other screens don't use encoder rotation
}
