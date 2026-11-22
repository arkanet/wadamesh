// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// P02 compile-only external 3.5" ILI9488 SPI LCD, Heltec V4.3 R2.
// Do not flash or solder until the P01 shared-harness electrical gate passes.
// Touch and microSD will be added in separate follow-up steps.
#include <helpers/ui/DisplayDriver.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class ILI9488LCDDisplay : public DisplayDriver {
  lgfx::Bus_SPI _bus;
  lgfx::Panel_ILI9488 _panel;
#if defined(P03_XPT2046_TOUCH)
  // Optional shared SPI2 touch controller (actual IC marking must be verified).
  lgfx::Touch_XPT2046 _touch;
#endif
  lgfx::LGFX_Device _lcd;
  bool _isOn = false;
  uint16_t _color = 0xFFFF;

public:
  ILI9488LCDDisplay();
  bool begin();
  bool isOn() override { return _isOn; }
  void turnOn() override;
  void turnOff() override;
  void clear() override;
  void startFrame(ColorVal bkg = UIColor::window_bkg) override;
  void setTextSize(int sz) override;
  void setColor(ColorVal c) override;
  void setCursor(int x, int y) override;
  void print(const char* str) override;
  void fillRect(int x, int y, int w, int h) override;
  void drawRect(int x, int y, int w, int h) override;
  void drawXbm(int x, int y, const uint8_t* bits, int w, int h) override;
  uint16_t getTextWidth(const char* str) override;
  void endFrame() override;
  void writePixelsRGB565(int x, int y, int w, int h, const uint16_t* pixels);
  void setDisplayRotation(uint8_t rotation);
  void panelSleep(bool sleep);
  // Uncalibrated 12-bit measurements for on-device calibration (P03 only).
  bool sampleTouchRaw(uint16_t* rawX, uint16_t* rawY);
};
