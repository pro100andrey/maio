/**
 * @file screen_manager.c
 * @brief LVGL screen manager implementation
 */

#include "screen_manager.h"
#include "../screens/screen_1.h"
#include "../screens/screen_2.h"
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

  // Show first screen
  lv_screen_load(screens[SCREEN_1]);
  current_screen = SCREEN_1;

  // Activate first screen
  screen_1_on_show();

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
  switch (current_screen) {
  case SCREEN_1:
    screen_1_on_hide();
    break;
  case SCREEN_2:
    screen_2_on_hide();
    break;
  default:
    break;
  }

  // Update screen content before showing
  switch (screen_id) {
  case SCREEN_1:
    screen_1_on_show();
    break;
  case SCREEN_2:
    screen_2_on_show();
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
  // No dynamic updates needed for simple screens
}

void screen_manager_encoder_rotate(int direction) {
  // Handle encoder rotation based on current screen
  switch (current_screen) {
  case SCREEN_2:
    screen_2_encoder_rotate(direction);
    break;
  default:
    // Other screens don't use encoder rotation
    break;
  }
}

void screen_manager_recreate_screens(void) {
  printf("[ScreenMgr] Recreating screens with new theme...\n");

  screen_id_t saved_screen = current_screen;

  // Delete all screens
  for (int i = 0; i < SCREEN_COUNT; i++) {
    if (screens[i]) {
      lv_obj_del(screens[i]);
      screens[i] = NULL;
    }
  }

  // Recreate all screens
  screens[SCREEN_1] = screen_1_create();
  screens[SCREEN_2] = screen_2_create();

  // Return to first screen
  screen_manager_show(SCREEN_1);

  printf("[ScreenMgr] Screens recreated\n");
}
