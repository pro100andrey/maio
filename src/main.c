#include "hardware/clocks.h"
#include "hardware/structs/scb.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "pico/util/queue.h"
#include <pico/stdio.h>
#include <stdio.h>

#define DWT_CONTROL (*((volatile uint32_t *)0xE0001000))
#define DWT_CYCCNT (*((volatile uint32_t *)0xE0001004))
#define DEMCR (*((volatile uint32_t *)0xE000EDFC))

typedef enum {
  EV_LED_TOGGLE,
  EV_BUTTON_PRESSED,
} event_type_t;

queue_t event_queue;

void init_cpu_perf_counter() {
  DEMCR |= 0x01000000;
  DWT_CYCCNT = 0;
  DWT_CONTROL |= 1;
}

bool timer_callback(struct repeating_timer *t) {
  event_type_t ev = EV_LED_TOGGLE;
  queue_try_add(&event_queue, &ev);

  return true; // keep repeating
}

int main() {
  stdio_init_all();

  init_cpu_perf_counter();

  if (cyw43_arch_init()) {
    printf("WiFi init failed\n");
    return -1;
  }

  queue_init(&event_queue, sizeof(event_type_t), 10);

  struct repeating_timer timer;
  add_repeating_timer_ms(1000, timer_callback, NULL, &timer);

  event_type_t ev;
  static bool led_state = false;

  uint32_t last_report_time = to_ms_since_boot(get_absolute_time());
  uint64_t total_active_cycles = 0;
  uint32_t cycle_start_work;

  while (true) {
    queue_remove_blocking(&event_queue, &ev);

    cycle_start_work = DWT_CYCCNT;

    if (ev == EV_LED_TOGGLE) {
      led_state = !led_state;
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);

      printf("LED is now %s\n", led_state ? "ON" : "OFF");
    }

    uint32_t cycle_end_work = DWT_CYCCNT;
    total_active_cycles += (cycle_end_work - cycle_start_work);

    // Отчет раз в секунду
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_report_time >= 1000) {

      float freq = (float)clock_get_hz(clk_sys);
      float load = ((float)total_active_cycles / freq) * 100.0f;

      // Формат для расширения Teleplot (если установлено)
      printf(">cpu_load:%.2f|g\n", load);
      // Обычный вывод
      printf("CPU Load: %.4f %% | Cycles: %llu\n", load, total_active_cycles);

      total_active_cycles = 0;
      last_report_time = now;
    }
  }
}
