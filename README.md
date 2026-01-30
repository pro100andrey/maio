# Maio - Multimeter All-in-One

## Architecture Overview (PREVIEW)

Event-driven firmware for Raspberry Pi Pico 2W with Strategy pattern for application state management.

### Core Components

``` txt
┌─────────────────┐
│   Main Loop     │ ◄── Blocking wait on event queue
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Event Manager   │ ◄── Queue-based (queue_t), configurable size
└────────┬────────┘
         │
         ├─► EV_ENCODER_ROTATE
         ├─► EV_ENCODER_BUTTON
         └─► EV_TIMER_TICK
         │
         ▼
┌─────────────────┐
│   Strategy      │ ◄── Pluggable state handlers
└─────────────────┘
```

### Event System

- **Queue**: FIFO event queue (size: `EVENT_QUEUE_SIZE` from CMake)
- **Blocking Wait**: CPU enters WFI (Wait For Interrupt) when queue is empty
- **Non-blocking Post**: `queue_try_add()` for interrupt-safe posting

**Event Types:**

- `EV_ENCODER_ROTATE` - Rotary encoder increment/decrement (payload: delta)
- `EV_ENCODER_BUTTON` - Push button press detected
- `EV_TIMER_TICK` - Periodic tick (every `SYSTEM_TICK_MS`)

### Strategy Pattern

Strategies implement application modes/states with consistent interface:

```c
typedef struct strategy_s {
  void (*on_enter)(void);           // Called when strategy becomes active
  void (*on_exit)(void);            // Called when strategy is replaced
  void (*on_event)(const event_t*); // Event handler
} strategy_t;
```

**Current Strategies:**

- `idle_strategy` - Test/demo strategy showing encoder position

## Hardware Layer

### Encoder Driver (`drivers/encoder/`)

**Algorithm**: Polling-based half-step detection

- Samples phases A/B every 1ms via hardware timer
- Detects falling edge of phase A with phase B state determining direction
- State machine tracks: `IDLE → ROTATING → DEBOUNCE → IDLE`
- One detent = one increment (no multiplier)

**Button Handling**:

- Edge detection: HIGH→LOW transition (pull-up to VCC, button to GND)
- Debouncing: 50ms for button, 2ms for rotation
- State tracking prevents repeated triggers

**Hardware Configuration**:

```c
PIN_ENC_A  = GPIO10  // Phase A, no internal pull (module has external pull-up)
PIN_ENC_B  = GPIO11  // Phase B, no internal pull (module has external pull-up)
PIN_ENC_SW = GPIO12  // Push button, internal pull-up (button to GND)
```

**Module Wiring**: VCC/GND encoder module with integrated 10kΩ pull-ups for phases A/B

### Encoder Manager (`src/managers/encoder_manager.c`)

Adapter layer between encoder driver and event system:

- Polls encoder state via `encoder_get_delta()` and `encoder_button_pressed()`
- Converts hardware state to events
- Posts to event queue with interrupt protection

## Timing Configuration

All timing controlled via CMakeLists.txt:

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `ENCODER_POLL_MS` | 1ms | Hardware timer interval for encoder polling |
| `SYSTEM_TICK_MS` | 10ms | Periodic tick event interval |
| `EVENT_QUEUE_SIZE` | 10 | Maximum events in queue |

**Timer Callback** (1ms):

- Polls encoder on every tick
- Posts `EV_TIMER_TICK` every `SYSTEM_TICK_MS` ticks (not every 1ms to avoid flooding)

## Project Structure

``` txt
maio/
├── board_config.h           # GPIO pin assignments
├── CMakeLists.txt           # Build configuration and timing constants
│
├── drivers/
│   └── encoder/             # Low-level encoder driver
│       ├── encoder.c        # Half-step algorithm, debouncing
│       └── encoder.h        # Driver interface
│
├── src/
│   ├── main.c               # Entry point, timer setup, event loop
│   │
│   ├── core/                # Framework core
│   │   ├── event_manager.*  # Event queue management
│   │   └── strategy.*       # Strategy pattern implementation
│   │
│   ├── managers/            # Hardware adapters
│   │   └── encoder_manager.* # Encoder → Event converter
│   │
│   ├── shared/
│   │   └── events.h         # Event type definitions
│   │
│   └── strategies/          # Application states
│       └── idle_strategy.*  # Test strategy
│
└── build/                   # CMake build artifacts
```

## Memory Model

- **Static Allocation**: No dynamic memory (no malloc/free)
- **Stack-based Events**: Events copied by value into queue
- **Global State**: Single strategy pointer, single event queue
- **Interrupt Safety**: Critical sections protected with `save_and_disable_interrupts()`

## Build System

CMake-based with Pico SDK integration:

- Toolchain: ARM GCC 14.2
- SDK: Pico SDK 2.2.0
- Target: RP2040/RP2350 (Pico 2W)
- Binary Format: UF2 (no_flash - runs from RAM)

**Feature Flags**:

- `ENABLE_ENCODER` - Encoder support
- `ENABLE_LED` - LED control via CYW43
- `DEBUG_VERBOSE` - Extended logging
- `ENABLE_ASSERTS` - Runtime assertions

## Debug Output

UART0 @ 115200 baud:

- Pin 0 (TX), Pin 1 (RX)
- Initialization logs from main.c, event_manager, encoder_manager
- Strategy-specific output (e.g., position updates in idle_strategy)

## Performance Characteristics

- **Latency**: 1ms max encoder response time (polling interval)
- **CPU Usage**: Minimal - WFI sleep between events
- **Queue Depth**: Configurable, default 10 events
- **Debounce**: 2ms rotation, 50ms button (prevents bouncing artifacts)

## Future Extensions

To add new functionality:

1. Create new strategy in `src/strategies/`
2. Implement `on_enter()`, `on_exit()`, `on_event()` callbacks
3. Add new event types to `src/shared/events.h` if needed
4. Switch strategy via `strategy_switch(&new_strategy)`

For new hardware:

1. Add pin definitions to `board_config.h`
2. Create driver in `drivers/`
3. Create manager in `src/managers/`
4. Post events from manager to event queue
