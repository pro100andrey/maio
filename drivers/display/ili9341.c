/**
 * @file ili9341.c
 * @brief ILI9341 TFT display driver implementation
 */

#include "ili9341.h"
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/pwm.h>
#include <hardware/spi.h>
#include <pico/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ILI9341 Commands
#define ILI9341_SWRESET 0x01 // Software Reset
#define ILI9341_SLPOUT 0x11  // Sleep Out
#define ILI9341_DISPON 0x29  // Display ON
#define ILI9341_CASET 0x2A   // Column Address Set
#define ILI9341_PASET 0x2B   // Page Address Set
#define ILI9341_RAMWR 0x2C   // Memory Write
#define ILI9341_MADCTL 0x36  // Memory Access Control
#define ILI9341_PIXFMT 0x3A  // Pixel Format Set
#define ILI9341_FRMCTR1 0xB1 // Frame Rate Control (Normal Mode)
#define ILI9341_DFUNCTR 0xB6 // Display Function Control
#define ILI9341_PWCTR1 0xC0  // Power Control 1
#define ILI9341_PWCTR2 0xC1  // Power Control 2
#define ILI9341_VMCTR1 0xC5  // VCOM Control 1
#define ILI9341_VMCTR2 0xC7  // VCOM Control 2
#define ILI9341_GMCTRP1 0xE0 // Positive Gamma Control
#define ILI9341_GMCTRN1 0xE1 // Negative Gamma Control

// Timing constants (ms)
#define RESET_PULSE_MS 10
#define RESET_WAIT_MS 120
#define SOFT_RESET_WAIT_MS 150
#define WAKEUP_WAIT_MS 120

// Helper macros
#define CS_LOW() gpio_put(dev->config.pin_cs, 0)
#define CS_HIGH() gpio_put(dev->config.pin_cs, 1)
#define DC_CMD() gpio_put(dev->config.pin_dc, 0)
#define DC_DATA() gpio_put(dev->config.pin_dc, 1)

// Maximum pixels per DMA burst for generic transfers
#define DMA_CHUNK_PIXELS 512
// Bytes per pixel (RGB565)
#define BYTES_PER_PIXEL 2

// Color storage for DMA repeat mode (fill operations) - stored as 2 bytes
static uint8_t g_dma_fill_color_bytes[2] = {0, 0};

/**
 * @brief Write command byte
 */
static inline void write_cmd(ili9341_t *dev, uint8_t cmd) {
  CS_LOW();
  DC_CMD();
  spi_write_blocking(dev->config.spi, &cmd, 1);
  CS_HIGH();
}

/**
 * @brief Write data bytes
 */
static inline void write_data(ili9341_t *dev, const uint8_t *data, size_t len) {
  CS_LOW();
  DC_DATA();
  spi_write_blocking(dev->config.spi, data, len);
  CS_HIGH();
}

/**
 * @brief Write command with data (optimized - single CS transaction)
 */
static void write_cmd_data(ili9341_t *dev, uint8_t cmd, const uint8_t *data,
                           size_t len) {
  CS_LOW();
  DC_CMD();
  spi_write_blocking(dev->config.spi, &cmd, 1);
  if (data && len > 0) {
    DC_DATA();
    spi_write_blocking(dev->config.spi, data, len);
  }
  CS_HIGH();
}

/**
 * @brief Set address window (optimized - batched commands)
 */
static void ili9341_set_window(ili9341_t *dev, uint16_t x0, uint16_t y0,
                               uint16_t x1, uint16_t y1) {
  // Batch both CASET and PASET in single CS transaction for speed
  CS_LOW();

  // CASET command + data
  DC_CMD();
  uint8_t caset_cmd = ILI9341_CASET;
  spi_write_blocking(dev->config.spi, &caset_cmd, 1);
  DC_DATA();
  uint8_t col_data[4] = {x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF};
  spi_write_blocking(dev->config.spi, col_data, 4);

  // PASET command + data (no CS toggle!)
  DC_CMD();
  uint8_t paset_cmd = ILI9341_PASET;
  spi_write_blocking(dev->config.spi, &paset_cmd, 1);
  DC_DATA();
  uint8_t row_data[4] = {y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF};
  spi_write_blocking(dev->config.spi, row_data, 4);

  // RAMWR command
  DC_CMD();
  uint8_t ramwr_cmd = ILI9341_RAMWR;
  spi_write_blocking(dev->config.spi, &ramwr_cmd, 1);

  CS_HIGH();
  // Note: Next operation should set DC_DATA and CS_LOW before sending pixels
}

// DMA IRQ handler for automatic completion signaling
static ili9341_t *g_dma_device = NULL;

static void __isr ili9341_dma_irq_handler(void) {
  if (g_dma_device && g_dma_device->dma_channel >= 0) {
    if (dma_hw->ints0 & (1u << g_dma_device->dma_channel)) {
      // Clear interrupt flag
      dma_hw->ints0 = 1u << g_dma_device->dma_channel;

      // Wait for SPI to finish shifting remaining bytes
      while (spi_is_busy(g_dma_device->config.spi)) {
        tight_loop_contents();
      }

      // End transaction (direct GPIO write)
      gpio_put(g_dma_device->config.pin_cs, 1);

      // Mark DMA as idle
      g_dma_device->dma_busy = false;
    }
  }
}

void ili9341_init(ili9341_t *dev, const ili9341_config_t *config) {
  memset(dev, 0, sizeof(ili9341_t));
  dev->config = *config;
  dev->init_state = ILI9341_STATE_IDLE;
  dev->dma_channel = -1;
  dev->dma_busy = false;

  // Set default orientation (portrait)
  dev->orientation = ILI9341_PORTRAIT;
  dev->width = ILI9341_NATIVE_WIDTH;
  dev->height = ILI9341_NATIVE_HEIGHT;

  // Initialize GPIO
  gpio_init(dev->config.pin_cs);
  gpio_set_dir(dev->config.pin_cs, GPIO_OUT);
  gpio_put(dev->config.pin_cs, 1);

  gpio_init(dev->config.pin_dc);
  gpio_set_dir(dev->config.pin_dc, GPIO_OUT);

  gpio_init(dev->config.pin_rst);
  gpio_set_dir(dev->config.pin_rst, GPIO_OUT);
  gpio_put(dev->config.pin_rst, 1);

  // Initialize SPI with explicit 8-bit format
  uint32_t actual_baudrate =
      spi_init(dev->config.spi, dev->config.spi_baudrate);
  spi_set_format(dev->config.spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  gpio_set_function(dev->config.pin_sck, GPIO_FUNC_SPI);
  gpio_set_function(dev->config.pin_mosi, GPIO_FUNC_SPI);

  printf("[ILI9341] SPI configured: requested=%u Hz, actual=%u Hz\n",
         dev->config.spi_baudrate, actual_baudrate);

  // Initialize backlight (PWM)
  if (dev->config.pin_led != 255) {
    gpio_set_function(dev->config.pin_led, GPIO_FUNC_PWM);
    uint32_t slice = pwm_gpio_to_slice_num(dev->config.pin_led);
    pwm_set_wrap(slice, 255);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(dev->config.pin_led), 0);
    pwm_set_enabled(slice, true);
  }

  // Claim DMA channel
  dev->dma_channel = dma_claim_unused_channel(true);

  // Setup DMA interrupt for automatic completion
  g_dma_device = dev;
  dma_channel_set_irq0_enabled(dev->dma_channel, true);
  irq_set_exclusive_handler(DMA_IRQ_0, ili9341_dma_irq_handler);
  irq_set_enabled(DMA_IRQ_0, true);

  printf("[ILI9341] Initialized: SPI=%s, DMA=%d (IRQ enabled)\n",
         dev->config.spi == spi0 ? "spi0" : "spi1", dev->dma_channel);
}

bool ili9341_init_tick(ili9341_t *dev) {
  uint32_t now = to_ms_since_boot(get_absolute_time());

  switch (dev->init_state) {
  case ILI9341_STATE_IDLE:
    // Start initialization
    printf("[ILI9341] Starting async init...\n");
    dev->init_state = ILI9341_STATE_RESET;
    return false;

  case ILI9341_STATE_RESET:
    // Hardware reset
    gpio_put(dev->config.pin_rst, 0);
    dev->state_timer = now + RESET_PULSE_MS;
    dev->init_state = ILI9341_STATE_RESET_WAIT;
    return false;

  case ILI9341_STATE_RESET_WAIT:
    if (now >= dev->state_timer) {
      gpio_put(dev->config.pin_rst, 1);
      dev->state_timer = now + RESET_WAIT_MS;
      dev->init_state = ILI9341_STATE_SOFT_RESET;
    }
    return false;

  case ILI9341_STATE_SOFT_RESET:
    if (now >= dev->state_timer) {
      write_cmd(dev, ILI9341_SWRESET);
      dev->state_timer = now + SOFT_RESET_WAIT_MS;
      dev->init_state = ILI9341_STATE_SOFT_RESET_WAIT;
    }
    return false;

  case ILI9341_STATE_SOFT_RESET_WAIT:
    if (now >= dev->state_timer) {
      dev->init_state = ILI9341_STATE_WAKEUP;
    }
    return false;

  case ILI9341_STATE_WAKEUP:
    write_cmd(dev, ILI9341_SLPOUT);
    dev->state_timer = now + WAKEUP_WAIT_MS;
    dev->init_state = ILI9341_STATE_WAKEUP_WAIT;
    return false;

  case ILI9341_STATE_WAKEUP_WAIT:
    if (now >= dev->state_timer) {
      dev->init_state = ILI9341_STATE_CONFIG;
    }
    return false;

  case ILI9341_STATE_CONFIG: {
    printf("[ILI9341] Configuring display...\n");

    // Power control
    uint8_t pwr1[] = {0x23};
    write_cmd_data(dev, ILI9341_PWCTR1, pwr1, 1);

    uint8_t pwr2[] = {0x10};
    write_cmd_data(dev, ILI9341_PWCTR2, pwr2, 1);

    uint8_t vcom1[] = {0x3E, 0x28};
    write_cmd_data(dev, ILI9341_VMCTR1, vcom1, 2);

    uint8_t vcom2[] = {0x86};
    write_cmd_data(dev, ILI9341_VMCTR2, vcom2, 1);

    // Memory Access Control (rotation, RGB order)
    uint8_t madctl[] = {0x48}; // MX, BGR
    write_cmd_data(dev, ILI9341_MADCTL, madctl, 1);

    // Pixel Format (16-bit RGB565)
    uint8_t pixfmt[] = {0x55};
    write_cmd_data(dev, ILI9341_PIXFMT, pixfmt, 1);

    // Frame Rate Control
    uint8_t frmctr[] = {0x00, 0x18};
    write_cmd_data(dev, ILI9341_FRMCTR1, frmctr, 2);

    // Display Function Control
    uint8_t dfunctr[] = {0x08, 0x82, 0x27};
    write_cmd_data(dev, ILI9341_DFUNCTR, dfunctr, 3);

    dev->init_state = ILI9341_STATE_DISPLAY_ON;
    return false;
  }

  case ILI9341_STATE_DISPLAY_ON:
    write_cmd(dev, ILI9341_DISPON);
    dev->init_state = ILI9341_STATE_READY;
    printf("[ILI9341] Display ready!\n");
    return true;

  case ILI9341_STATE_READY:
    return true; // Already ready
  }

  return false;
}

bool ili9341_send_pixels_dma(ili9341_t *dev, const uint16_t *buffer,
                             size_t length) {
  if (dev->dma_busy || dev->dma_channel < 0) {
    return false;
  }

  // Mark DMA as busy
  dev->dma_busy = true;

  // Configure DMA transfer
  dma_channel_config c = dma_channel_get_default_config(dev->dma_channel);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_dreq(&c, spi_get_dreq(dev->config.spi, true));
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, false);

  CS_LOW();
  DC_DATA();

  dma_channel_configure(dev->dma_channel, &c,
                        &spi_get_hw(dev->config.spi)->dr, // dest
                        buffer,                           // src (bytes)
                        length * BYTES_PER_PIXEL,         // count (bytes)
                        true                              // start immediately
  );

  // IRQ handler will set CS_HIGH and clear dma_busy
  return true;
}

bool ili9341_dma_is_idle(ili9341_t *dev) { return !dev->dma_busy; }

int ili9341_get_dma_channel(ili9341_t *dev) { return dev->dma_channel; }

void ili9341_set_backlight(ili9341_t *dev, uint8_t brightness) {
  if (dev->config.pin_led != 255) {
    uint32_t slice = pwm_gpio_to_slice_num(dev->config.pin_led);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(dev->config.pin_led),
                       brightness);
  }
}

// Orientation control functions
void ili9341_set_orientation(ili9341_t *dev,
                             ili9341_orientation_t orientation) {
  dev->orientation = orientation;

  uint8_t madctl_value;
  switch (orientation) {
  case ILI9341_PORTRAIT:
    madctl_value = 0x48; // MY=0, MX=1, MV=0, BGR=1
    dev->width = ILI9341_NATIVE_WIDTH;
    dev->height = ILI9341_NATIVE_HEIGHT;
    break;
  case ILI9341_LANDSCAPE:
    madctl_value = 0x28; // MY=0, MX=0, MV=1, BGR=1
    dev->width = ILI9341_NATIVE_HEIGHT;
    dev->height = ILI9341_NATIVE_WIDTH;
    break;
  case ILI9341_PORTRAIT_INV:
    madctl_value = 0x88; // MY=1, MX=0, MV=0, BGR=1
    dev->width = ILI9341_NATIVE_WIDTH;
    dev->height = ILI9341_NATIVE_HEIGHT;
    break;
  case ILI9341_LANDSCAPE_INV:
    madctl_value = 0xE8; // MY=1, MX=1, MV=1, BGR=1
    dev->width = ILI9341_NATIVE_HEIGHT;
    dev->height = ILI9341_NATIVE_WIDTH;
    break;
  default:
    madctl_value = 0x48;
    dev->width = ILI9341_NATIVE_WIDTH;
    dev->height = ILI9341_NATIVE_HEIGHT;
    break;
  }

  write_cmd(dev, 0x36); // MADCTL
  write_data(dev, &madctl_value, 1);

  printf("[ILI9341] Orientation set to %d (%ux%u, MADCTL=0x%02X)\n",
         orientation, dev->width, dev->height, madctl_value);
}

ili9341_orientation_t ili9341_get_orientation(ili9341_t *dev) {
  return dev->orientation;
}

uint16_t ili9341_get_width(ili9341_t *dev) { return dev->width; }

uint16_t ili9341_get_height(ili9341_t *dev) { return dev->height; }

// Fast rectangle fill with single window setup
void ili9341_fill_rect(ili9341_t *dev, uint16_t x, uint16_t y, uint16_t w,
                       uint16_t h, uint16_t color) {
  // Bounds check
  if (x >= dev->width || y >= dev->height)
    return;
  if (x + w > dev->width)
    w = dev->width - x;
  if (y + h > dev->height)
    h = dev->height - y;

  uint32_t pixel_count = w * h;
  if (pixel_count == 0)
    return;

  // Set window once
  ili9341_set_window(dev, x, y, x + w - 1, y + h - 1);

  // Prepare swapped color for ILI9341 (big-endian)
  uint16_t swapped_color = (color >> 8) | (color << 8);

  // Use stack buffer for speed (512 pixels = 1KB)
#define FAST_CHUNK 512
  uint16_t buffer[FAST_CHUNK];
  for (int i = 0; i < FAST_CHUNK; i++) {
    buffer[i] = swapped_color;
  }

  CS_LOW();
  DC_DATA();

  // Send in chunks
  uint32_t remaining = pixel_count;
  while (remaining > 0) {
    uint32_t chunk = (remaining > FAST_CHUNK) ? FAST_CHUNK : remaining;
    spi_write_blocking(dev->config.spi, (const uint8_t *)buffer, chunk * 2);
    remaining -= chunk;
  }

  CS_HIGH();
}

void ili9341_fill_screen(ili9341_t *dev, uint16_t color) {
  // Use optimized fill_rect instead of manual loop
  ili9341_fill_rect(dev, 0, 0, dev->width, dev->height, color);
}

/**
 * @brief Fill rectangle using DMA with ring buffer (non-blocking, optimized)
 *
 * Uses DMA with 2-byte ring buffer to repeat color pattern efficiently.
 * Faster than chunked buffer approach and uses minimal memory.
 *
 * @param dev Device context
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color RGB565 color
 * @return true if DMA started, false if DMA busy
 *
 * Note: Caller must check ili9341_dma_is_idle() before next DMA operation
 */
bool ili9341_fill_rect_async(ili9341_t *dev, uint16_t x, uint16_t y, uint16_t w,
                             uint16_t h, uint16_t color) {
  // Check if DMA is busy
  if (dev->dma_busy) {
    return false;
  }

  // Bounds check
  if (x >= dev->width || y >= dev->height)
    return false;
  if (x + w > dev->width)
    w = dev->width - x;
  if (y + h > dev->height)
    h = dev->height - y;

  uint32_t pixel_count = w * h;
  if (pixel_count == 0)
    return false;

  uint32_t byte_count = pixel_count * BYTES_PER_PIXEL;

  // Prepare swapped color for ILI9341 (byte order)
  uint16_t swapped_color = (color >> 8) | (color << 8);
  g_dma_fill_color_bytes[0] = (uint8_t)(swapped_color & 0xFF);
  g_dma_fill_color_bytes[1] = (uint8_t)(swapped_color >> 8);

  // Set window
  ili9341_set_window(dev, x, y, x + w - 1, y + h - 1);

  // Mark DMA as busy
  dev->dma_busy = true;

  // Configure DMA with 2-byte ring buffer:
  // - Transfer size: 8-bit (matches SPI format)
  // - Read increment: true, but wraps every 2 bytes (ring buffer)
  // - Ring: true with size=2 (power of 2)
  dma_channel_config c = dma_channel_get_default_config(dev->dma_channel);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8); // 8-bit matches SPI
  channel_config_set_dreq(&c, spi_get_dreq(dev->config.spi, true));
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, false);
  channel_config_set_ring(&c, false, 1); // Ring on read, size=2^1=2 bytes

  CS_LOW();
  DC_DATA();

  // Clear any stale interrupt flag before starting
  dma_hw->ints0 = 1u << dev->dma_channel;

  // Start DMA: send byte_count bytes, reading from 2-byte ring buffer
  dma_channel_configure(dev->dma_channel, &c, &spi_get_hw(dev->config.spi)->dr,
                        g_dma_fill_color_bytes, byte_count, true);

  return true;
}

/**
 * @brief Fill entire screen using DMA (non-blocking)
 * @param dev Device context
 * @param color RGB565 color
 * @return true if DMA started, false if DMA busy
 */
bool ili9341_fill_screen_async(ili9341_t *dev, uint16_t color) {
  return ili9341_fill_rect_async(dev, 0, 0, dev->width, dev->height, color);
}

void ili9341_draw_pixel(ili9341_t *dev, uint16_t x, uint16_t y,
                        uint16_t color) {
  if (x >= dev->width || y >= dev->height) {
    return; // Out of bounds
  }

  // Set window to single pixel
  ili9341_set_window(dev, x, y, x, y);

  // Swap bytes for ILI9341
  uint16_t swapped_color = (color >> 8) | (color << 8);

  CS_LOW();
  DC_DATA();
  spi_write_blocking(dev->config.spi, (const uint8_t *)&swapped_color, 2);
  CS_HIGH();
}

void ili9341_draw_line(ili9341_t *dev, uint16_t x0, uint16_t y0, uint16_t x1,
                       uint16_t y1, uint16_t color) {
  // Bresenham's line algorithm
  int16_t dx = abs(x1 - x0);
  int16_t dy = abs(y1 - y0);
  int16_t sx = (x0 < x1) ? 1 : -1;
  int16_t sy = (y0 < y1) ? 1 : -1;
  int16_t err = dx - dy;

  while (true) {
    ili9341_draw_pixel(dev, x0, y0, color);

    if (x0 == x1 && y0 == y1) {
      break;
    }

    int16_t e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}
