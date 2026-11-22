// SPDX-License-Identifier: GPL-3.0-or-later
// P03: optional KMRTM35018-SPI resistive touch via XPT2046-compatible IC.
// This target remains compile-only pending actual touch IC verification.
// All touch transfers are handled by the display's LovyanGFX SPI2 bus;
// NEVER instantiate a second SPIClass on the same GPIOs.
#if defined(HELTEC_LORA_V4_R2_ILI9488) && defined(P03_XPT2046_TOUCH) && defined(ESP32)

#include <Arduino.h>
#include <ILI9488LCDDisplay.h>
#include "HeltecV4CapTouch.h"
#include <helpers/ui/MomentaryButton.h>

#ifndef P03_TOUCH_X_MIN
#define P03_TOUCH_X_MIN 300
#endif
#ifndef P03_TOUCH_X_MAX
#define P03_TOUCH_X_MAX 3900
#endif
#ifndef P03_TOUCH_Y_MIN
#define P03_TOUCH_Y_MIN 400
#endif
#ifndef P03_TOUCH_Y_MAX
#define P03_TOUCH_Y_MAX 3900
#endif
#ifndef P03_TOUCH_SWAP_XY
#define P03_TOUCH_SWAP_XY 0
#endif
#ifndef P03_TOUCH_FLIP_X
#define P03_TOUCH_FLIP_X 0
#endif
#ifndef P03_TOUCH_FLIP_Y
#define P03_TOUCH_FLIP_Y 0
#endif
#ifndef P03_TOUCH_SWIPE_PX
#define P03_TOUCH_SWIPE_PX 36
#endif

static_assert(P03_TOUCH_X_MIN < P03_TOUCH_X_MAX &&
              P03_TOUCH_Y_MIN < P03_TOUCH_Y_MAX,
              "P03 XPT2046 calibration min/max bounds invalid");
static_assert(P03_TOUCH_X_MAX <= 4095 && P03_TOUCH_Y_MAX <= 4095,
              "XPT2046 ADC is 12-bit");
static_assert(P03_TOUCH_SWAP_XY == 0 || P03_TOUCH_SWAP_XY == 1, "swapXY is 0 or 1");

extern ILI9488LCDDisplay display;

namespace {
bool s_ready = false;
bool s_pressed = false;
bool s_seen_live = false;
bool s_tap_pending = false;
bool s_swipe_pending = false;
bool s_is_swiping = false;
uint8_t s_point_rotation = 0;
uint16_t s_x = 0, s_y = 0;
uint16_t s_start_x = 0, s_start_y = 0;
uint16_t s_raw_x = 0, s_raw_y = 0;
uint16_t s_tap_x = 0, s_tap_y = 0;
int8_t s_swipe_x = 0, s_swipe_y = 0;
uint8_t s_misses = 0;

uint16_t scale(int v, int lo, int hi, int maxOut) {
  if (v < lo) v = lo;
  if (v > hi) v = hi;
  return static_cast<uint16_t>((static_cast<int32_t>(v - lo) * maxOut) / (hi - lo));
}

void transform(uint16_t rawX, uint16_t rawY, uint16_t& x, uint16_t& y) {
  // First map both channels to a normalized range; SWAP then preserves the
  // full physical screen dimensions (320x480).
  uint16_t nx = scale(rawX, P03_TOUCH_X_MIN, P03_TOUCH_X_MAX, 4095);
  uint16_t ny = scale(rawY, P03_TOUCH_Y_MIN, P03_TOUCH_Y_MAX, 4095);
#if P03_TOUCH_SWAP_XY
  uint16_t t = nx; nx = ny; ny = t;
#endif
#if P03_TOUCH_FLIP_X
  nx = 4095 - nx;
#endif
#if P03_TOUCH_FLIP_Y
  ny = 4095 - ny;
#endif
  const uint16_t px = (static_cast<uint32_t>(nx) * 319) / 4095;
  const uint16_t py = (static_cast<uint32_t>(ny) * 479) / 4095;

  // Hardware display rotation; LVGL receives already-rotated coordinates.
  switch (s_point_rotation & 3) {
    case 1: x = py;       y = 319 - px; break; // 480x320
    case 2: x = 319 - px; y = 479 - py; break; // 320x480
    case 3: x = 479 - py; y = px;       break; // 480x320
    default: x = px; y = py; break;
  }
}
} // namespace

bool heltecV4CapTouchBegin() {
  // Display::begin installs Touch_XPT2046 into LGFX for the P03 build.
  // The controller cannot be safely identified from a touch read alone.
  s_ready = display.isOn();
  return s_ready;
}

int heltecV4CapTouchCheck() {
  if (!s_ready) return BUTTON_EVENT_NONE;
  uint16_t rawX = 0, rawY = 0;
  const bool touched = display.sampleTouchRaw(&rawX, &rawY);
  if (!touched) {
    // A single dropped read must not split a continuous finger drag.
    if (s_pressed && ++s_misses < 2) return BUTTON_EVENT_NONE;
    s_misses = 0;
    if (!s_pressed) return BUTTON_EVENT_NONE;
    s_pressed = false;
    s_is_swiping = false;
    if (!s_seen_live && !s_swipe_pending) {
      s_tap_pending = true; s_tap_x = s_x; s_tap_y = s_y;
    }
    return s_swipe_pending ? BUTTON_EVENT_NONE : BUTTON_EVENT_CLICK;
  }
  s_misses = 0;
  s_raw_x = rawX; s_raw_y = rawY;
  uint16_t x, y;
  transform(rawX, rawY, x, y);
  s_x = x; s_y = y;
  if (!s_pressed) {
    s_pressed = true;
    s_start_x = x; s_start_y = y;
    s_seen_live = false;
    s_is_swiping = false;
    s_swipe_pending = false;
    s_tap_pending = false;
  }
  const int dx = static_cast<int>(x) - s_start_x;
  const int dy = static_cast<int>(y) - s_start_y;
  if (!s_is_swiping && abs(dx) >= P03_TOUCH_SWIPE_PX &&
      abs(dx) > abs(dy)) {
    s_is_swiping = true;
    s_swipe_pending = true;
    s_swipe_x = dx > 0 ? 1 : -1;
    s_swipe_y = 0;
  }
  return BUTTON_EVENT_NONE;
}

bool heltecV4CapTouchGetLive(uint16_t* x, uint16_t* y) {
  if (!s_pressed) return false;
  if (x) *x = s_x;
  if (y) *y = s_y;
  s_seen_live = true;
  return true;
}

bool heltecV4CapTouchPopTap(uint16_t* x, uint16_t* y) {
  if (!s_tap_pending) return false;
  s_tap_pending = false;
  if (x) *x = s_tap_x;
  if (y) *y = s_tap_y;
  return true;
}

bool heltecV4CapTouchPopSwipe(int8_t* x, int8_t* y) {
  if (!s_swipe_pending || s_pressed) return false;
  s_swipe_pending = false;
  if (x) *x = s_swipe_x;
  if (y) *y = s_swipe_y;
  return true;
}

// P03 deliberately polls inline from lvglTouchRead. Background polling
// requires separate hardware validation of SPI2 bus contention/latency.
bool heltecV4CapTouchStartBackgroundPoll(uint32_t) { return false; }
bool heltecV4CapTouchIsAsyncPolling() { return false; }
bool heltecV4CapTouchIsSwiping() { return s_is_swiping; }
void heltecV4CapTouchSetRotation(uint8_t) {}
void heltecV4CapTouchSetPointRotation(uint8_t r) { s_point_rotation = r & 3; }
void heltecV4CapTouchSetSlowPoll(bool) {}
const char* heltecV4CapTouchDebug() {
  return "P03 XPT2046 provisional; inspect raw ADC and calibrate axes";
}
void heltecV4CapTouchGetRaw(uint16_t* x, uint16_t* y) {
  if (x) *x = s_raw_x;
  if (y) *y = s_raw_y;
}

#endif // P03_XPT2046_TOUCH
