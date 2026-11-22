// SPDX-License-Identifier: GPL-3.0-or-later
// P02: compile-only external ILI9488 LCD has no confirmed resistive touch IC.
// Preserve the existing LVGL touch API without probing the CHSC6x I2C bus.
// Replace this shim with a validated SPI touch driver during P03.
#if defined(HELTEC_LORA_V4_R2_ILI9488) && defined(ESP32)
#include "HeltecV4CapTouch.h"
#include <helpers/ui/MomentaryButton.h>

bool heltecV4CapTouchBegin() { return false; }
int heltecV4CapTouchCheck() { return BUTTON_EVENT_NONE; }
bool heltecV4CapTouchPopTap(uint16_t*, uint16_t*) { return false; }
bool heltecV4CapTouchGetLive(uint16_t*, uint16_t*) { return false; }
bool heltecV4CapTouchPopSwipe(int8_t*, int8_t*) { return false; }
bool heltecV4CapTouchStartBackgroundPoll(uint32_t) { return false; }
bool heltecV4CapTouchIsAsyncPolling() { return false; }
bool heltecV4CapTouchIsSwiping() { return false; }
void heltecV4CapTouchSetRotation(uint8_t) {}
void heltecV4CapTouchSetSlowPoll(bool) {}
void heltecV4CapTouchSetPointRotation(uint8_t) {}
const char* heltecV4CapTouchDebug() { return "P02: SPI touch not configured"; }
void heltecV4CapTouchGetRaw(uint16_t* x, uint16_t* y) {
  if (x) *x = 0;
  if (y) *y = 0;
}
#endif
