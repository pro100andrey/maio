/**
 * @file ili9341.c
 * @brief ILI9341 TFT display driver implementation
 */

#include "ili9341.h"
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/spi.h>
#include <pico/time.h>
#include <stdio.h>
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
 * @brief Write command with data
 */
static void write_cmd_data(ili9341_t *dev, uint8_t cmd, const uint8_t *data,
                           size_t len) {
  write_cmd(dev, cmd);
  if (data && len > 0) {
    write_data(dev, data, len);
  }
}

void ili9341_init(ili9341_t *dev, const ili9341_config_t *config) {
  memset(dev, 0, sizeof(ili9341_t));
  dev->config = *config;
  dev->init_state = ILI9341_STATE_IDLE;
  dev->dma_channel = -1;
  dev->dma_busy = false;

  // Initialize GPIO
  gpio_init(dev->config.pin_cs);
  gpio_set_dir(dev->config.pin_cs, GPIO_OUT);
  gpio_put(dev->config.pin_cs, 1);

  gpio_init(dev->config.pin_dc);
  gpio_set_dir(dev->config.pin_dc, GPIO_OUT);

  gpio_init(dev->config.pin_rst);
  gpio_set_dir(dev->config.pin_rst, GPIO_OUT);
  gpio_put(dev->config.pin_rst, 1);

  // Initialize SPI
  spi_init(dev->config.spi, dev->config.spi_baudrate);
  gpio_set_function(dev->config.pin_sck, GPIO_FUNC_SPI);
  gpio_set_function(dev->config.pin_mosi, GPIO_FUNC_SPI);

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

  printf("[ILI9341] Initialized: SPI=%s, DMA=%d\n",
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

void ili9341_set_window(ili9341_t *dev, uint16_t x0, uint16_t y0, uint16_t x1,
                        uint16_t y1) {
  uint8_t col_data[4] = {x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF};
  write_cmd_data(dev, ILI9341_CASET, col_data, 4);

  uint8_t row_data[4] = {y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF};
  write_cmd_data(dev, ILI9341_PASET, row_data, 4);

  write_cmd(dev, ILI9341_RAMWR);
}

bool ili9341_send_pixels_dma(ili9341_t *dev, const uint16_t *buffer,
                             size_t length) {
  if (dev->dma_busy || dev->dma_channel < 0) {
    return false;
  }

  dev->dma_busy = true;

  // Configure DMA transfer
  dma_channel_config c = dma_channel_get_default_config(dev->dma_channel);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
  channel_config_set_dreq(&c, spi_get_dreq(dev->config.spi, true));

  CS_LOW();
  DC_DATA();

  dma_channel_configure(dev->dma_channel, &c,
                        &spi_get_hw(dev->config.spi)->dr, // dest
                        buffer,                           // src
                        length,                           // count (pixels)
                        true                              // start immediately
  );

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
