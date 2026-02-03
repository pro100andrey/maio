#include "screen_multimeter.h"
#include "../managers/theme_manager.h"
#include "../shared/digital_display.h"
#include <stdlib.h>

// Multimeter screen components
static lv_obj_t *screen = NULL;
static digital_display_t *voltage_display = NULL;
static lv_obj_t *label_menu_item1 = NULL;
static lv_obj_t *label_menu_item2 = NULL;
static lv_obj_t *label_menu_item3 = NULL;
static lv_obj_t *label_menu_exit = NULL;

static lv_timer_t *update_timer = NULL;
static uint8_t selected_menu_item = 0;
static uint8_t update_counter = 0;

// Menu items
static const char *menu_items[] = {"DC Voltage", "AC Voltage", "Resistance",
                                   "Exit"};

#define MENU_ITEM_COUNT 4
#define LONG_PRESS_TIME_MS 3000

/**
 * @brief Generate voltage reading in format 1.0000xx
 */
static float generate_random_voltage(void) {
  // Generate voltage 1.0000xx where xx changes (00-99)
  int last_two_digits = rand() % 100;
  return 1.0f + (float)last_two_digits / 1000000.0f;
}

/**
 * @brief Update timer callback (33ms = 30 times per second)
 */
static void update_timer_cb(lv_timer_t *timer) {
  if (!voltage_display)
    return;

  // Only update last 2 digits (00-99)
  int last_two_digits = rand() % 100;
  digital_display_set_value(voltage_display, last_two_digits);

  update_counter++;
}

/**
 * @brief Update menu item colors based on selection
 */
static void update_menu_selection(void) {
  const theme_colors_t *colors = theme_manager_get_colors();
  lv_obj_t *menu_labels[] = {label_menu_item1, label_menu_item2,
                             label_menu_item3, label_menu_exit};

  for (uint8_t i = 0; i < MENU_ITEM_COUNT; i++) {
    if (menu_labels[i]) {
      if (i == selected_menu_item) {
        lv_obj_set_style_text_color(menu_labels[i], colors->accent_yellow,
                                    0); // Yellow
        lv_obj_set_style_text_font(menu_labels[i], &lv_font_montserrat_14, 0);
      } else {
        lv_obj_set_style_text_color(menu_labels[i], colors->text_secondary,
                                    0); // Gray
        lv_obj_set_style_text_font(menu_labels[i], &lv_font_montserrat_14, 0);
      }
    }
  }
}

lv_obj_t *screen_multimeter_create(void) {
  const theme_colors_t *colors = theme_manager_get_colors();
  // Create screen
  screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, colors->bg_primary, 0);

  int16_t menu_spacing = 20;

  // Top menu items (3 items starting from x=0, top)
  // Menu Item 1 - at top
  label_menu_item1 = lv_label_create(screen);
  lv_label_set_text(label_menu_item1, menu_items[0]);
  lv_obj_set_pos(label_menu_item1, 0, 10);

  // Menu Item 2
  label_menu_item2 = lv_label_create(screen);
  lv_label_set_text(label_menu_item2, menu_items[1]);
  lv_obj_set_pos(label_menu_item2, 0, 10 + menu_spacing);

  // Menu Item 3
  label_menu_item3 = lv_label_create(screen);
  lv_label_set_text(label_menu_item3, menu_items[2]);
  lv_obj_set_pos(label_menu_item3, 0, 10 + menu_spacing * 2);

  // Create digital display for voltage: "1.0000" + "xx" + " V"
  // digit_count=2 for last two digits, prefix="1.0000", suffix=" V",
  // spacing=20px
  voltage_display = digital_display_create(screen,
                                           2,        // 2 digits (00-99)
                                           "1.0000", // prefix
                                           " V",     // suffix
                                           20, // spacing between characters
                                           &lv_font_montserrat_32, // font
                                           colors->accent_green // green color
  );
  digital_display_align(voltage_display, LV_ALIGN_CENTER, 0, 0);

  // Bottom menu items (Exit)
  label_menu_exit = lv_label_create(screen);
  lv_label_set_text(label_menu_exit, menu_items[3]);
  lv_obj_set_style_text_color(label_menu_exit, colors->accent_red,
                              0); // Red
  lv_obj_set_pos(label_menu_exit, 0, 240 - 30);

  // Initialize menu selection
  selected_menu_item = 0;
  update_menu_selection();

  return screen;
}

void screen_multimeter_update(void) {
  // Regular update function (called from main loop)
  // Timer handles the actual updates
}

void screen_multimeter_encoder_rotate(int direction) {
  if (direction > 0) {
    // Rotate clockwise - move down in menu
    selected_menu_item = (selected_menu_item + 1) % MENU_ITEM_COUNT;
  } else if (direction < 0) {
    // Rotate counter-clockwise - move up in menu
    if (selected_menu_item == 0) {
      selected_menu_item = MENU_ITEM_COUNT - 1;
    } else {
      selected_menu_item--;
    }
  }

  update_menu_selection();
}

void screen_multimeter_on_enter(void) {
  // Start update timer (33ms = 30 times per second)
  if (update_timer == NULL) {
    update_timer = lv_timer_create(update_timer_cb, 33, NULL);
  } else {
    lv_timer_resume(update_timer);
  }

  // Reset selection
  selected_menu_item = 0;
  update_menu_selection();

  // Generate initial reading
  update_timer_cb(NULL);
}

void screen_multimeter_on_exit(void) {
  // Pause update timer
  if (update_timer) {
    lv_timer_pause(update_timer);
  }
}
