# ILI9341 TFT Display Driver

High-performance ILI9341 320×240 TFT LCD driver for RP2040/RP2350 with DMA support, optimized for real-time applications.

## Features

### Core Capabilities

- **Non-blocking initialization** - State machine-based async initialization
- **DMA-accelerated transfers** - IRQ-driven completion, 99% CPU-free during large transfers
- **Smart transfer selection** - Automatic DMA/blocking choice based on data size
- **Window caching** - Optimized repeated operations to same screen area
- **Dual SPI modes** - 8-bit or 16-bit pixel transfers
- **75 MHz SPI speed** - Direct register access for maximum throughput (4.27 Mpix/s)
- **Multiple orientations** - Portrait, landscape, and inverted variants
- **Hardware PWM backlight** - Gamma-correctable brightness control

### Performance

- **Full screen fill**: 18 ms (76800 pixels)
- **DMA launch overhead**: 10-50 μs
- **Efficiency**: 91% of theoretical maximum at 75 MHz SPI
- **CPU utilization**: ~1% during DMA transfers

## Hardware Setup

### Pin Configuration

```c
ili9341_config_t config = {
    .spi = spi0,                    // SPI instance
    .spi_baudrate = 62500000,       // 62.5 MHz (will be boosted to 75 MHz)
    .pin_cs = 17,                   // Chip Select
    .pin_dc = 20,                   // Data/Command
    .pin_rst = 21,                  // Reset
    .pin_led = 22,                  // Backlight (255 = not used)
    .pin_sck = 18,                  // SPI Clock
    .pin_mosi = 19,                 // SPI MOSI
    .use_16bit_pixel_transfer = false  // 8-bit mode (safer, byte-swapped)
};
```

### Wiring

| Pin Function | RP2040 GPIO | ILI9341 Pin |
|--------------|-------------|-------------|
| SPI SCK      | 18          | SCK         |
| SPI MOSI     | 19          | SDI (MOSI)  |
| CS           | 17          | CS          |
| DC           | 20          | DC/RS       |
| RST          | 21          | RESET       |
| LED          | 22          | LED+        |
| GND          | GND         | GND         |
| 3.3V         | 3V3         | VCC         |

**Note**: ILI9341 typically operates at 3.3V. Check your module specifications.

## Basic Usage

### Initialization

#### Blocking Initialization (Simple)

```c
#include "drivers/display/ili9341.h"

ili9341_t display;

int main() {
    // Initialize driver
    ili9341_init(&display, &config);
    
    // Wait for initialization to complete
    while (!ili9341_init_tick(&display)) {
        sleep_ms(10);
    }
    
    // Display is ready
    ili9341_set_backlight(&display, 255);
    ili9341_fill_screen(&display, 0x001F);  // Blue screen
}
```

#### Non-blocking Initialization (Recommended)

```c
// In main event loop
bool display_ready = false;

void event_loop_tick() {
    if (!display_ready) {
        display_ready = ili9341_init_tick(&display);
        if (display_ready) {
            printf("Display initialized!\n");
            ili9341_set_backlight(&display, 255);
        }
    } else {
        // Run application logic
    }
}
```

### Drawing Operations

#### Synchronous API (Blocking)

```c
// Fill entire screen
ili9341_fill_screen(&display, 0xF800);  // Red

// Fill rectangle
ili9341_fill_rect(&display, 10, 10, 100, 50, 0x07E0);  // Green rect

// Draw pixel
ili9341_draw_pixel(&display, 50, 50, 0xFFFF);  // White pixel

// Draw line
ili9341_draw_line(&display, 0, 0, 239, 319, 0x001F);  // Blue diagonal
```

#### Asynchronous API (DMA, Non-blocking)

```c
// Launch full screen fill (DMA)
if (ili9341_fill_screen_async(&display, 0x0000)) {
    printf("DMA started, CPU is free!\n");
    
    // CPU can do other work here (ADC, calculations, etc.)
    do_other_work();
    
    // Wait for completion before next display operation
    while (!ili9341_dma_is_idle(&display)) {
        tight_loop_contents();
    }
}

// Launch rectangle fill (DMA)
if (ili9341_fill_rect_async(&display, 50, 50, 100, 80, 0xF800)) {
    // Do work while DMA runs
    process_sensor_data();
    
    // Wait before next operation
    while (!ili9341_dma_is_idle(&display)) {}
}
```

### Custom Buffer Drawing

```c
// Create gradient buffer
uint16_t gradient[240 * 100];
for (int y = 0; y < 100; y++) {
    for (int x = 0; x < 240; x++) {
        uint8_t r = (x * 31) / 240;
        uint8_t g = ((240 - x) * 63) / 240;
        uint8_t b = (y * 31) / 100;
        uint16_t color = (r << 11) | (g << 5) | b;  // RGB565
        
        // Byte swap if using 8-bit mode
        if (!ili9341_get_pixel_transfer_mode(&display)) {
            color = __builtin_bswap16(color);
        }
        gradient[y * 240 + x] = color;
    }
}

// Set drawing window
ili9341_set_window(&display, 0, 0, 239, 99);

// Send pixels (auto-selects DMA for large buffers)
ili9341_send_pixels(&display, gradient, 240 * 100);

// Wait if DMA was used
while (!ili9341_dma_is_idle(&display)) {}
```

## Advanced Techniques

### Async Pipeline for Maximum Performance

```c
// Chain async operations with CPU work in between
void render_ui() {
    uint32_t start = time_us_32();
    
    // Step 1: Launch background clear
    ili9341_fill_screen_async(&display, 0x0000);
    
    // CPU work: Prepare next operation while DMA runs
    uint16_t colors[] = {0xF800, 0x07E0, 0x001F};
    
    // Step 2: Wait for background, launch first rect
    while (!ili9341_dma_is_idle(&display)) {}
    ili9341_fill_rect_async(&display, 10, 10, 60, 40, colors[0]);
    
    // CPU work: Read sensors while DMA runs
    float temperature = read_temperature();
    
    // Step 3: Wait, launch second rect
    while (!ili9341_dma_is_idle(&display)) {}
    ili9341_fill_rect_async(&display, 80, 10, 60, 40, colors[1]);
    
    // CPU work: Process data
    update_graph(temperature);
    
    // Step 4: Wait, draw sync elements (small operations)
    while (!ili9341_dma_is_idle(&display)) {}
    ili9341_draw_line(&display, 0, 60, 239, 60, 0xFFFF);
    
    uint32_t elapsed = time_us_32() - start;
    printf("Total render: %u μs\n", elapsed);
}
```

### Window Caching Optimization

Window caching saves ~3 μs per operation when drawing to same area repeatedly:

```c
// GOOD: Repeated draws to same window benefit from cache
for (int i = 0; i < 100; i++) {
    ili9341_fill_rect(&display, 50, 50, 10, 10, colors[i % 3]);
    // First call: full window setup
    // Subsequent calls: cache hit (faster)
}

// LESS EFFICIENT: Different windows don't benefit from cache
for (int i = 0; i < 100; i++) {
    ili9341_fill_rect(&display, i, i, 10, 10, colors[i % 3]);
    // Each call: cache miss (full window setup)
}
```

**Note**: Window cache is invalidated when CS goes HIGH, so it works best for consecutive operations to the same area.

### Smart DMA Threshold

Driver automatically selects optimal transfer method:

- **Small buffers (< 2KB)**: Blocking SPI (faster due to low DMA overhead)
- **Large buffers (≥ 2KB)**: DMA transfer (CPU stays free)

```c
// Automatically uses blocking (960 bytes)
uint16_t small_buf[480];
ili9341_send_pixels(&display, small_buf, 480);

// Automatically uses DMA (4320 bytes)
uint16_t large_buf[2160];
ili9341_send_pixels(&display, large_buf, 2160);
```

Override threshold in `ili9341.c`:

```c
#define DMA_THRESHOLD_BYTES 2048  // Adjust based on your use case
```

## Display Configuration

### Orientation

```c
// Portrait (default): 240×320
ili9341_set_orientation(&display, ILI9341_PORTRAIT);

// Landscape: 320×240
ili9341_set_orientation(&display, ILI9341_LANDSCAPE);

// Inverted variants
ili9341_set_orientation(&display, ILI9341_PORTRAIT_INV);
ili9341_set_orientation(&display, ILI9341_LANDSCAPE_INV);

// Get current dimensions
uint16_t w = ili9341_get_width(&display);
uint16_t h = ili9341_get_height(&display);
```

### Backlight Control

```c
// Linear brightness (0-255)
ili9341_set_backlight(&display, 128);  // 50%

// Gamma-corrected for perceptual linearity
double percent = 50.0;  // 50%
double gamma_level = pow(percent / 100.0, 2.2);
uint8_t level = (uint8_t)(gamma_level * 255.0);
ili9341_set_backlight(&display, level);
```

### SPI Transfer Modes

#### 8-bit Mode (Default, Recommended)

```c
config.use_16bit_pixel_transfer = false;

// Pixels need byte swap for RGB565
uint16_t color = 0xF800;  // Red in RGB565
uint16_t swapped = __builtin_bswap16(color);  // Ready for 8-bit SPI
```

**Pros**: Compatible with all RP2040/RP2350, more reliable  
**Cons**: Requires byte swapping for buffers

#### 16-bit Mode (Experimental)

```c
config.use_16bit_pixel_transfer = true;

// No byte swap needed (MSB_FIRST handles order)
uint16_t color = 0xF800;  // Red, ready to send
```

**Pros**: No byte swapping overhead  
**Cons**: May have compatibility issues with some boards

Toggle at runtime:

```c
ili9341_set_pixel_transfer_mode(&display, true);   // Switch to 16-bit
ili9341_set_pixel_transfer_mode(&display, false);  // Switch to 8-bit
```

## Color Format (RGB565)

### Color Creation

```c
// RGB to RGB565 macro
#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

uint16_t red    = RGB565(255, 0, 0);    // 0xF800
uint16_t green  = RGB565(0, 255, 0);    // 0x07E0
uint16_t blue   = RGB565(0, 0, 255);    // 0x001F
uint16_t white  = RGB565(255, 255, 255); // 0xFFFF
uint16_t black  = RGB565(0, 0, 0);      // 0x0000

// Common colors (predefined)
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_WHITE   0xFFFF
#define COLOR_BLACK   0x0000
```

### Buffer Preparation

```c
// For 8-bit mode: byte swap required
void fill_buffer_8bit(uint16_t *buf, size_t count, uint16_t color) {
    uint16_t swapped = __builtin_bswap16(color);
    for (size_t i = 0; i < count; i++) {
        buf[i] = swapped;
    }
}

// For 16-bit mode: direct assignment
void fill_buffer_16bit(uint16_t *buf, size_t count, uint16_t color) {
    for (size_t i = 0; i < count; i++) {
        buf[i] = color;
    }
}
```

## Performance Optimization Tips

### 1. Use Async API for Large Operations

```c
// BAD: Blocking, CPU waits 18 ms
ili9341_fill_screen(&display, 0x0000);
do_sensor_work();  // CPU was idle during fill

// GOOD: Non-blocking, CPU free during fill
ili9341_fill_screen_async(&display, 0x0000);
do_sensor_work();  // CPU works while DMA runs
while (!ili9341_dma_is_idle(&display)) {}
```

### 2. Batch Small Operations

```c
// BAD: Many small window setups
for (int i = 0; i < 100; i++) {
    ili9341_draw_pixel(&display, x[i], y[i], color[i]);
}

// GOOD: One window, batch pixels
ili9341_set_window(&display, x_min, y_min, x_max, y_max);
// Prepare buffer with all pixels
ili9341_send_pixels(&display, buffer, pixel_count);
```

### 3. Minimize Mode Switching

```c
// BAD: Frequent 8/16-bit switching
for (int i = 0; i < 10; i++) {
    ili9341_set_pixel_transfer_mode(&display, true);
    draw_something();
    ili9341_set_pixel_transfer_mode(&display, false);
}

// GOOD: Set once, draw multiple
ili9341_set_pixel_transfer_mode(&display, true);
for (int i = 0; i < 10; i++) {
    draw_something();
}
ili9341_set_pixel_transfer_mode(&display, false);
```

### 4. Pre-compute Buffers

```c
// GOOD: Compute once, reuse
static uint16_t gradient_buffer[240 * 100];
static bool initialized = false;

if (!initialized) {
    // Compute gradient once
    compute_gradient(gradient_buffer, 240, 100);
    initialized = true;
}

// Fast redraw
ili9341_set_window(&display, 0, 0, 239, 99);
ili9341_send_pixels(&display, gradient_buffer, 240 * 100);
```

## Troubleshooting

### Display Shows Garbage/Wrong Colors

**Cause**: Incorrect SPI mode or byte order  
**Solution**:

```c
// Try 8-bit mode with byte swap
config.use_16bit_pixel_transfer = false;

// Or explicitly swap colors
uint16_t color = __builtin_bswap16(0xF800);
```

### Display Not Responding

**Checks**:

1. Verify wiring (especially CS, DC, RST)
2. Check SPI clock speed (may be too high for long wires)
3. Ensure 3.3V power supply is adequate
4. Try slower SPI speed temporarily:

   ```c
   config.spi_baudrate = 1000000;  // 1 MHz for testing
   ```

### DMA Not Working

**Checks**:

1. Verify DMA channel was claimed successfully
2. Check IRQ is enabled
3. Ensure `ili9341_dma_is_idle()` is called before next operation
4. Monitor DMA channel allocation:

   ```c
   printf("DMA channel: %d\n", ili9341_get_dma_channel(&display));
   ```

### Flickering or Tearing

**Cause**: Drawing while previous operation still in progress  
**Solution**:

```c
// Always wait for DMA before next operation
while (!ili9341_dma_is_idle(&display)) {
    tight_loop_contents();
}
```

### Slow Performance

**Checks**:

1. Verify SPI is running at 75 MHz:

   ```bash
   # Check console output during init
   [ILI9341] Maximum SPI: CPSR=2, SCR=0, actual=75000000 Hz (75.0 MHz)
   ```

2. Ensure `clk_peri` is at 150 MHz (not 48 MHz)
3. Use async API for operations > 2KB
4. Batch small operations into buffers

## API Reference

### API Initialization

| Function | Description |
| -------- | ----------- |
| `ili9341_init()` | Initialize device context |
| `ili9341_init_tick()` | Non-blocking init state machine tick |

### Display Control

| Function | Description |
| -------- | ----------- |
| `ili9341_set_backlight()` | Set backlight brightness (0-255) |
| `ili9341_set_orientation()` | Change display orientation |
| `ili9341_get_orientation()` | Get current orientation |
| `ili9341_get_width()` | Get current width |
| `ili9341_get_height()` | Get current height |

### Synchronous Drawing (Blocking)

| Function | Description |
| -------- | ----------- |
| `ili9341_fill_screen()` | Fill entire screen |
| `ili9341_fill_rect()` | Fill rectangle |
| `ili9341_draw_pixel()` | Draw single pixel |
| `ili9341_draw_line()` | Draw line (Bresenham) |

### Asynchronous Drawing (DMA)

| Function | Description |
| -------- | ----------- |
| `ili9341_fill_screen_async()` | Fill screen via DMA |
| `ili9341_fill_rect_async()` | Fill rectangle via DMA |
| `ili9341_dma_is_idle()` | Check if DMA complete |

### Custom Buffers

| Function | Description |
| -------- | ----------- |
| `ili9341_set_window()` | Set drawing window |
| `ili9341_send_pixels()` | Smart transfer (auto DMA/blocking) |
| `ili9341_send_pixels_dma()` | Force DMA transfer |

### Configuration

| Function | Description |
| -------- | ----------- |
| `ili9341_set_pixel_transfer_mode()` | Set 8/16-bit mode |
| `ili9341_get_pixel_transfer_mode()` | Get current mode |
| `ili9341_get_dma_channel()` | Get DMA channel number |

## Example Projects

### 1. Simple Color Fill

```c
#include "drivers/display/ili9341.h"

int main() {
    ili9341_t display;
    ili9341_config_t config = {
        .spi = spi0,
        .spi_baudrate = 62500000,
        .pin_cs = 17, .pin_dc = 20, .pin_rst = 21,
        .pin_led = 22, .pin_sck = 18, .pin_mosi = 19,
        .use_16bit_pixel_transfer = false
    };
    
    ili9341_init(&display, &config);
    while (!ili9341_init_tick(&display)) sleep_ms(10);
    
    ili9341_set_backlight(&display, 255);
    
    // Cycle through colors
    uint16_t colors[] = {0xF800, 0x07E0, 0x001F};
    while (true) {
        for (int i = 0; i < 3; i++) {
            ili9341_fill_screen(&display, colors[i]);
            sleep_ms(1000);
        }
    }
}
```

### 2. Async Background Update

```c
void update_display_background() {
    static uint8_t hue = 0;
    
    // Convert HSV to RGB565
    uint16_t color = hsv_to_rgb565(hue, 255, 128);
    
    // Launch async fill (non-blocking)
    if (ili9341_fill_screen_async(&display, color)) {
        hue += 5;
    }
}

void main_loop() {
    while (true) {
        // Start background update
        update_display_background();
        
        // Do critical work while DMA runs
        read_adc();
        process_signal();
        
        // Wait for display before next iteration
        while (!ili9341_dma_is_idle(&display)) {}
        
        sleep_ms(50);
    }
}
```

### 3. Real-time Waveform Display

```c
#define WAVE_WIDTH 240
#define WAVE_HEIGHT 100

static uint16_t waveform_buffer[WAVE_WIDTH * WAVE_HEIGHT];

void render_waveform(float *samples, int count) {
    // Clear buffer (black)
    memset(waveform_buffer, 0, sizeof(waveform_buffer));
    
    // Draw waveform (green)
    for (int x = 0; x < count && x < WAVE_WIDTH; x++) {
        int y = (int)(samples[x] * WAVE_HEIGHT / 2) + WAVE_HEIGHT / 2;
        if (y >= 0 && y < WAVE_HEIGHT) {
            // Byte swap if in 8-bit mode
            uint16_t color = 0x07E0;  // Green
            if (!ili9341_get_pixel_transfer_mode(&display)) {
                color = __builtin_bswap16(color);
            }
            waveform_buffer[y * WAVE_WIDTH + x] = color;
        }
    }
    
    // Draw to display
    ili9341_set_window(&display, 0, 0, WAVE_WIDTH - 1, WAVE_HEIGHT - 1);
    ili9341_send_pixels(&display, waveform_buffer, WAVE_WIDTH * WAVE_HEIGHT);
}
```

## Architecture Notes

### Clock Configuration

Driver requires `clk_sys` and `clk_peri` at 150 MHz for optimal performance:

```c
// In your main.c before driver init
set_sys_clock_khz(150000, true);  // 150 MHz system clock
```

SPI clock is derived from `clk_peri`:

- Formula: `SPI_freq = clk_peri / (CPSR × (1 + SCR))`
- Driver sets: `CPSR=2, SCR=0` → `SPI_freq = 150MHz / 2 = 75MHz`

### DMA Architecture

- **Single channel**: One DMA channel claimed during init
- **IRQ-driven completion**: DMA_IRQ_0 automatically signals completion
- **Repeat mode**: For solid fills, single color value repeated via DMA ring buffer
- **Transfer modes**:
  - 8-bit: Ring buffer with 2-byte color (byte-swapped RGB565)
  - 16-bit: Native 16-bit transfers (MSB first)

### Memory Safety

- **No dynamic allocation**: All structures stack/static allocated
- **Buffer ownership**: Caller owns buffers, driver doesn't copy
- **DMA safety**: Buffers must remain valid until `dma_is_idle()` returns true
