/**
 * @file ili9341.h
 * @brief ILI9341 TFT display driver with async init and DMA support
 *
 * Features:
 * - Non-blocking initialization via state machine
 * - DMA-based pixel transfer
 * - Dependency injection (pins/SPI passed via config)
 */

#ifndef ILI9341_H
#define ILI9341_H

#include <hardware/dma.h>
#include <hardware/spi.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Display physical dimensions (native portrait mode) */
#define ILI9341_NATIVE_WIDTH 240
#define ILI9341_NATIVE_HEIGHT 320

/** Legacy compatibility macros */
#define ILI9341_WIDTH 240
#define ILI9341_HEIGHT 320

/** Orientation modes */
typedef enum {
  ILI9341_PORTRAIT = 0,     // 240x320
  ILI9341_LANDSCAPE = 1,    // 320x240
  ILI9341_PORTRAIT_INV = 2, // 240x320 (inverted)
  ILI9341_LANDSCAPE_INV = 3 // 320x240 (inverted)
} ili9341_orientation_t;

/** Color format */
#define ILI9341_COLOR_DEPTH 16 // RGB565

/**
 * @brief ILI9341 initialization states (for async init)
 */
typedef enum {
  ILI9341_STATE_IDLE = 0,
  ILI9341_STATE_RESET,
  ILI9341_STATE_RESET_WAIT,
  ILI9341_STATE_SOFT_RESET,
  ILI9341_STATE_SOFT_RESET_WAIT,
  ILI9341_STATE_WAKEUP,
  ILI9341_STATE_WAKEUP_WAIT,
  ILI9341_STATE_CONFIG,
  ILI9341_STATE_DISPLAY_ON,
  ILI9341_STATE_READY
} ili9341_init_state_t;

/**
 * @brief ILI9341 device configuration (dependency injection)
 */
typedef struct {
  /** SPI instance (spi0 or spi1) */
  spi_inst_t *spi;
  /** SPI baudrate in Hz */
  uint32_t spi_baudrate;

  /** GPIO pins */
  uint8_t pin_cs;   // Chip Select
  uint8_t pin_dc;   // Data/Command
  uint8_t pin_rst;  // Reset
  uint8_t pin_led;  // Backlight (optional, 255 = not used)
  uint8_t pin_sck;  // SPI Clock
  uint8_t pin_mosi; // SPI MOSI

  /** Pixel transfer mode: true = 16-bit SPI, false = 8-bit SPI */
  bool use_16bit_pixel_transfer;
} ili9341_config_t;

/**
 * @brief ILI9341 device context
 */
typedef struct {
  ili9341_config_t config;
  ili9341_init_state_t init_state;
  uint32_t state_timer;   // For non-blocking delays
  int dma_channel;        // DMA channel for pixel transfer (-1 if not claimed)
  volatile bool dma_busy; // DMA transfer in progress
  ili9341_orientation_t orientation; // Current display orientation
  uint16_t width;                    // Current width based on orientation
  uint16_t height;                   // Current height based on orientation
  uint8_t backlight_level;           // Saved backlight level for display_on/off
} ili9341_t;

/**
 * @brief Initialize device context (does not touch hardware)
 * @param dev Device context
 * @param config Hardware configuration
 */
void ili9341_init(ili9341_t *dev, const ili9341_config_t *config);

/**
 * @brief Async initialization tick (call from event loop)
 * @param dev Device context
 * @return true when initialization complete, false if still in progress
 */
bool ili9341_init_tick(ili9341_t *dev);

/**
 * @brief Send pixel data via DMA (non-blocking)
 * @param dev Device context
 * @param buffer Pixel buffer (RGB565 format)
 * @param length Number of pixels (not bytes!)
 * @return true if DMA started, false if busy
 */
bool ili9341_send_pixels_dma(ili9341_t *dev, const uint16_t *buffer,
                             size_t length);

/**
 * @brief Check if DMA transfer is complete
 * @param dev Device context
 * @return true if DMA idle, false if busy
 */
bool ili9341_dma_is_idle(ili9341_t *dev);

/**
 * @brief Get DMA channel number (for ISR registration)
 * @param dev Device context
 * @return DMA channel or -1 if not claimed
 */
int ili9341_get_dma_channel(ili9341_t *dev);

/**
 * @brief Send pixel data with automatic DMA/blocking selection
 * @param dev Device context
 * @param buffer Pixel buffer (RGB565 format)
 * @param pixels Number of pixels to send
 * @note Automatically chooses blocking for small transfers (< 2KB) or when DMA
 * busy, otherwise uses DMA. Manages CS internally.
 */
void ili9341_send_pixels(ili9341_t *dev, const uint16_t *buffer, size_t pixels);

/**
 * @brief Set drawing window for subsequent pixel operations
 * @param dev Device context
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate (inclusive)
 * @param y1 End Y coordinate (inclusive)
 * @note Use before ili9341_send_pixels() for custom buffer drawing.
 *       Window is cached for performance optimization.
 */
void ili9341_set_window(ili9341_t *dev, uint16_t x0, uint16_t y0, uint16_t x1,
                        uint16_t y1);

/**
 * @brief Set pixel transfer mode (8-bit or 16-bit SPI)
 * @param dev Device context
 * @param use_16bit true for 16-bit mode, false for 8-bit mode
 */
void ili9341_set_pixel_transfer_mode(ili9341_t *dev, bool use_16bit);

/**
 * @brief Get current pixel transfer mode
 * @param dev Device context
 * @return true if 16-bit mode, false if 8-bit mode
 */
bool ili9341_get_pixel_transfer_mode(ili9341_t *dev);

/**
 * @brief Set backlight brightness
 * @param dev Device context
 * @param brightness 0-255 (0=off, 255=max)
 */
void ili9341_set_backlight(ili9341_t *dev, uint8_t brightness);

/**
 * @brief Set display orientation
 * @param dev Device context
 * @param orientation Orientation mode
 */
void ili9341_set_orientation(ili9341_t *dev, ili9341_orientation_t orientation);

/**
 * @brief Get current display orientation
 * @param dev Device context
 * @return Current orientation
 */
ili9341_orientation_t ili9341_get_orientation(ili9341_t *dev);

/**
 * @brief Get current display width (varies with orientation)
 * @param dev Device context
 * @return Width in pixels
 */
uint16_t ili9341_get_width(ili9341_t *dev);

/**
 * @brief Get current display height (varies with orientation)
 * @param dev Device context
 * @return Height in pixels
 */
uint16_t ili9341_get_height(ili9341_t *dev);

/**
 * @brief Fill entire screen with solid color (blocking SPI)
 * @param dev Device context
 * @param color RGB565 color value
 */
void ili9341_fill_screen(ili9341_t *dev, uint16_t color);

/**
 * @brief Fill rectangular area with solid color (optimized)
 * @param dev Device context
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width in pixels
 * @param h Height in pixels
 * @param color RGB565 color value
 */
void ili9341_fill_rect(ili9341_t *dev, uint16_t x, uint16_t y, uint16_t w,
                       uint16_t h, uint16_t color);

/**
 * @brief Fill rectangular area using DMA (non-blocking)
 * @param dev Device context
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width in pixels
 * @param h Height in pixels
 * @param color RGB565 color value
 * @return true if DMA started, false if DMA busy
 *
 * @note CPU is free during transfer. Check ili9341_dma_is_idle() before next
 * operation.
 */
bool ili9341_fill_rect_async(ili9341_t *dev, uint16_t x, uint16_t y, uint16_t w,
                             uint16_t h, uint16_t color);

/**
 * @brief Fill entire screen using DMA (non-blocking)
 * @param dev Device context
 * @param color RGB565 color value
 * @return true if DMA started, false if DMA busy
 *
 * @note CPU is free during transfer. Perfect for background updates while doing
 * ADC/sensors.
 */
bool ili9341_fill_screen_async(ili9341_t *dev, uint16_t color);

/**
 * @brief Draw a single pixel
 * @param dev Device context
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB565 color value
 */
void ili9341_draw_pixel(ili9341_t *dev, uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief Draw a line using Bresenham's algorithm
 * @param dev Device context
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 * @param color RGB565 color value
 */
void ili9341_draw_line(ili9341_t *dev, uint16_t x0, uint16_t y0, uint16_t x1,
                       uint16_t y1, uint16_t color);

/**
 * @brief Enter sleep mode (low power, display content retained)
 * @param dev Device context
 * @note Reduces power consumption. Display content is retained in memory.
 *       Use ili9341_wakeup() to exit sleep mode.
 */
void ili9341_sleep(ili9341_t *dev);

/**
 * @brief Exit sleep mode and restore display
 * @param dev Device context
 * @note Takes 120ms to complete. Display automatically turns on.
 */
void ili9341_wakeup(ili9341_t *dev);

/**
 * @brief Turn off display output (display goes blank)
 * @param dev Device context
 * @note Display content is retained in memory. Less power saving than sleep.
 */
void ili9341_display_off(ili9341_t *dev);

/**
 * @brief Turn on display output
 * @param dev Device context
 */
void ili9341_display_on(ili9341_t *dev);

/**
 * @brief Enable/disable idle mode (reduced color depth for power saving)
 * @param dev Device context
 * @param enable true to enable idle mode (8-color), false for normal
 * (65K-color)
 * @note Idle mode reduces color depth to save power while keeping display
 * active. Useful for low-power applications where full color is not needed.
 */
void ili9341_set_idle_mode(ili9341_t *dev, bool enable);

#endif // ILI9341_H
