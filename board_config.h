/**
 * @file board_config.h
 * @brief Hardware configuration for Raspberry Pi Pico 2W board
 *
 * Defines all GPIO pin assignments and hardware-specific settings.
 * Modify this file to adapt the firmware to different board layouts.
 */

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>

/**
 * Rotary encoder GPIO pin assignments
 */
typedef enum encoder_pins_e {
  /** Encoder phase A */
  PIN_ENC_A = 10,
  /** Encoder phase B */
  PIN_ENC_B = 11,
  /** Encoder push button */
  PIN_ENC_SW = 12
} encoder_pins_t;

/**
 * Onboard LED pin (controlled via CYW43 wireless chip)
 */
#define LED_PIN CYW43_WL_GPIO_LED_PIN

/**
 * UART GPIO pin assignments
 */
typedef enum uart_pins_e {
  /** UART transmit pin */
  PIN_UART_TX = 0,
  /** UART receive pin */
  PIN_UART_RX = 1
} uart_pins_t;

/** UART baud rate */
#define UART_BAUD_RATE 115200

/**
 * I2C GPIO pin assignments for display/sensors
 */
typedef enum i2c_pins_e {
  /** I2C data line */
  PIN_I2C_SDA = 4,
  /** I2C clock line */
  PIN_I2C_SCL = 5
} i2c_pins_t;

/**
 * @name TFT Display Configuration (ILI9341)
 */
typedef enum tft_pins_e {
  PIN_TFT_SCK = 18,
  PIN_TFT_MOSI = 19,
  PIN_TFT_CS = 17,
  PIN_TFT_DC = 21,
  PIN_TFT_RST = 20,
  PIN_TFT_LED = 22 // Backlight control pin
} tft_pins_t;

#define TFT_SPI_INST spi0
#define TFT_BAUDRATE (40 * 1000 * 1000) // 40 MHz - safe for most ILI9341
#define TFT_USE_16BIT_PIXEL_TRANSFER                                           \
  false // false=8-bit (compatible), true=16-bit (faster DMA)

/**
 * System configuration
 */
/** Encoder polling interval (milliseconds) */
#define ENCODER_POLL_MS 1

/** System tick interval for EV_TIMER_TICK events (milliseconds) */
#define SYSTEM_TICK_MS 10

/** Event queue size (maximum number of pending events) */
#define EVENT_QUEUE_SIZE 10

#endif // BOARD_CONFIG_H
