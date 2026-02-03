/**
 * @file screen_1.c
 * @brief Screen 1 implementation - Voltmeter with ADC
 */

#include "screen_1.h"
#include "../managers/theme_manager.h"
#include "../shared/digital_display.h"
#include <hardware/adc.h>
#include <stdio.h>

static lv_obj_t *label_voltage = NULL;
static digital_display_t *voltage_display = NULL;
static lv_timer_t *update_timer = NULL;

static void update_timer_cb(lv_timer_t *timer) {
  if (!voltage_display || !label_voltage) {
    return;
  }

  // Read voltage from ADC
  adc_select_input(0);
  uint16_t raw = adc_read();
  float voltage = (raw * 3.3f) / 4095.0f;

  // Update main voltage display using digital_display
  // Convert to millivolts and show last 3 digits
  int millivolts = (int)(voltage * 1000.0f);
  int last_digits = millivolts % 1000;

  digital_display_set_value(voltage_display, last_digits);

  // Update full voltage label with raw value
  char buf[32];
  snprintf(buf, sizeof(buf), "ADC: %.4fV (raw: %u)", voltage, raw);
  lv_label_set_text(label_voltage, buf);
}

lv_obj_t *screen_1_create(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  const theme_colors_t *colors = theme_manager_get_colors();

  // Set background color
  lv_obj_set_style_bg_color(screen, colors->bg_primary, 0);

  // Create title label
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "Voltmeter");
  lv_obj_set_style_text_color(title, colors->accent_green, 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  // Create digital display for voltage (format: X.xxx V)
  voltage_display = digital_display_create(
      screen, 3, "0.", " V", 18, &lv_font_montserrat_40, colors->accent_green);
  digital_display_align(voltage_display, LV_ALIGN_CENTER, 0, -10);

  // Create detailed voltage label
  label_voltage = lv_label_create(screen);
  lv_label_set_text(label_voltage, "ADC: 0.0000 V");
  lv_obj_set_style_text_color(label_voltage, colors->text_primary, 0);
  lv_obj_set_style_text_font(label_voltage, &lv_font_montserrat_14, 0);
  lv_obj_align(label_voltage, LV_ALIGN_CENTER, 0, 60);

  // Create info label
  lv_obj_t *info = lv_label_create(screen);
  lv_label_set_text(info, "GPIO26 (ADC0)");
  lv_obj_set_style_text_color(info, colors->text_secondary, 0);
  lv_obj_set_style_text_font(info, &lv_font_montserrat_14, 0);
  lv_obj_align(info, LV_ALIGN_BOTTOM_MID, 0, -20);

  return screen;
}

void screen_1_on_show(void) {
  // Initialize ADC if not already done
  static bool adc_initialized = false;
  if (!adc_initialized) {
    adc_init();
    adc_gpio_init(26); // GPIO26 is ADC0
    // Note: ADC pins don't support pull-down/up, they are high impedance
    // Connect GPIO26 to GND or signal source to measure
    adc_initialized = true;
  }

  // Create timer when screen becomes active (30 Hz)
  if (!update_timer) {
    update_timer = lv_timer_create(update_timer_cb, 33, NULL);
  }
}

void screen_1_on_hide(void) {
  // Delete timer when screen is hidden
  if (update_timer) {
    lv_timer_delete(update_timer);
    update_timer = NULL;
  }
}
