#ifndef PICO_BUTTON_ASYNC_H
#define PICO_BUTTON_ASYNC_H

#include "pico/stdlib.h"
#include "pico/async_context_threadsafe_background.h"
#include <string.h>

#ifndef DEBOUNCE_SAMPLES
#define DEBOUNCE_SAMPLES 8
#endif

class DebounceManager {
private:
  async_context_t *_context;
  async_at_time_worker_t _worker;
  uint32_t _history[DEBOUNCE_SAMPLES];
  uint8_t _index = 0;
  uint32_t _mask = 0;
  uint32_t _interval_ms;

  static void _worker_callback(async_context_t *context, async_at_time_worker_t *worker) {
    DebounceManager *instance = (DebounceManager *)worker->user_data;
    instance->tick();
    async_context_add_at_time_worker_in_ms(context, worker, instance->_interval_ms);
  }

  static async_context_t *_get_default_context() {
    static async_context_threadsafe_background_t shared_bg_context;
    static bool initialized = false;

    if (!initialized) {
#if LIB_PICO_CYW43_ARCH
      async_context_t *wifi_ctx = cyw43_arch_async_context();
      if (wifi_ctx) return wifi_ctx;
#endif
      async_context_threadsafe_background_init_with_defaults(&shared_bg_context);
      initialized = true;
    }
    return &shared_bg_context.core;
  }
public:
  volatile uint32_t currentState = 0xFFFFFFFF;

  DebounceManager(uint32_t interval_ms = 5, async_context_t *ctx = nullptr)
    : _interval_ms(interval_ms) {
    _context = (ctx != nullptr) ? ctx : _get_default_context();
    memset(_history, 0xFF, sizeof(_history));
    _worker.do_work = _worker_callback;
    _worker.user_data = this;
  }

  void begin() {
    async_context_add_at_time_worker_in_ms(_context, &_worker, _interval_ms);
  }

  void addPin(int pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);

    async_context_acquire_lock_blocking(_context);
    _mask |= (1UL << pin);
    async_context_release_lock(_context);
  }

  void tick() {
    _history[_index] = gpio_get_all() & _mask;
    _index = (_index + 1) % DEBOUNCE_SAMPLES;
    uint32_t stableHigh = 0xFFFFFFFF, stableLow = 0;

    for (uint8_t i = 0; i < DEBOUNCE_SAMPLES; i++) {
      stableHigh &= _history[i];
      stableLow |= _history[i];
    }
    currentState = (currentState | (stableHigh & _mask)) & (stableLow | ~_mask);
  }

  uint32_t getSafeState() {
    async_context_acquire_lock_blocking(_context);
    uint32_t state = currentState;
    async_context_release_lock(_context);
    return state;
  }

  bool checkChord(uint32_t chordMask) {
    return (~getSafeState() & chordMask) == chordMask;
  }
};

class PicoButton {
protected:
  DebounceManager &_mgr;
  uint32_t _pinMask;
  uint32_t _pressStartTime = 0;
  uint32_t _lastRepeatTime = 0;

  /*
  Separate trackers so that the press and release
  functions don't "steal" the edge from each other.
  */

  bool _lastPressCheck = false;
  bool _lastReleaseCheck = false;
  bool _longPressTriggered = false;

  inline uint32_t _now() {
    return to_ms_since_boot(get_absolute_time());
  }

public:
  PicoButton(DebounceManager &mgr, int pin)
    : _mgr(mgr), _pinMask(1UL << pin) {
    _mgr.addPin(pin);
  }
  virtual bool isPressed() {
    return !(_mgr.getSafeState() & _pinMask);
  }

  /** @brief Rising edge: True only the moment the button is pressed. */
  bool justPressed() {
    bool current = isPressed();
    bool result = (current && !_lastPressCheck);
    _lastPressCheck = current;

    if (result) {
      _pressStartTime = _now();
      _lastRepeatTime = _now();
      _longPressTriggered = false;
    }
    return result;
  }

  /** @brief Falling edge: True only the moment the button is released. */
  bool justReleased() {
    bool current = isPressed();
    bool result = (!current && _lastReleaseCheck);
    _lastReleaseCheck = current;
    if (result) {
      _pressStartTime = 0;  // Essential reset
    }
    return result;
  }

  /** @brief Time-based: True once when held for 'ms'. */
  bool justLongPressed(uint32_t ms = 1000) {
    bool current = isPressed();
    // Internal sync: ensure we have a start time even if justPressed wasn't called
    if (current && _pressStartTime == 0) {
      _pressStartTime = _now();
      _longPressTriggered = false;
    }

    if (current && !_longPressTriggered && _pressStartTime > 0) {
      if (_now() - _pressStartTime >= ms) {
        _longPressTriggered = true;
        return true;
      }
    }
    // Reset timer if button is physically up
    if (!current) {
      _pressStartTime = 0;
      _longPressTriggered = false;
    }
    return false;
  }

  /**
  * @brief Checks if the button is currently held down.
  * This is a non-blocking check of the last debounced state.
  * Simulates repeated triggers every interval_ms.
  * @return true if the button is pressed (logic low) for delay_ms amount of time.
  */
  bool repeat(uint32_t delay_ms = 500, uint32_t interval_ms = 100) {
    bool current = isPressed();
    if (current && _pressStartTime > 0 && (_now() - _pressStartTime >= delay_ms)) {
      if (_now() - _lastRepeatTime >= interval_ms) {
        _lastRepeatTime = _now();
        return true;
      }
    }
    return false;
  }
};

#endif
