/**
 * @file display_manager.c
 * @brief Display manager implementation
 */

#include "display_manager.h"
#include <stdio.h>

/** Reference to ILI9341 device */
static ili9341_t *ili_dev = NULL;

void display_manager_init(ili9341_t *ili9341_dev) {
  ili_dev = ili9341_dev;
  printf("[DisplayMgr] Display manager initialized\n");
}

ili9341_t *display_manager_get_device(void) { return ili_dev; }
