// SPDX-License-Identifier: GPL-3.0-or-later
#if defined(HELTEC_LORA_V4_R2_ILI9488)

#include "ILI9488LCDDisplay.h"
#include <Arduino.h>
#include <driver/spi_master.h>

#ifndef ILI9488_SPI_WRITE_HZ
#define ILI9488_SPI_WRITE_HZ 16000000
#endif

ILI9488LCDDisplay::ILI9488LCDDisplay() : DisplayDriver(320, 480) {
  // External SPI bus on FSPI/SPI2; SX1262 radio uses independent HSPI/SPI3.
  // Reserve separate chip-selects for LCD, touch and microSD.
  auto bus = _bus.config();
  bus.spi_host = SPI2_HOST;
  bus.spi_mode = 0;
  bus.freq_write = ILI9488_SPI_WRITE_HZ; // conservative bring-up clock
  bus.freq_read = 4000000;
  bus.spi_3wire = false;
  bus.use_lock = true;
  bus.dma_channel = SPI_DMA_CH_AUTO;
  bus.pin_sclk = PIN_TFT_SCL;
  bus.pin_mosi = PIN_TFT_SDA;
  bus.pin_miso = PIN_TFT_MISO;
  bus.pin_dc = PIN_TFT_DC;
  _bus.config(bus);

  _panel.setBus(&_bus);
  auto panel = _panel.config();
  panel.pin_cs = PIN_TFT_CS;
  panel.pin_rst = PIN_TFT_RST;
  panel.pin_busy = -1;
  panel.panel_width = 320;
  panel.panel_height = 480;
  panel.offset_x = 0;
  panel.offset_y = 0;
  panel.offset_rotation = 0;
  panel.readable = false;
  panel.invert = false;
  panel.rgb_order = false;
  panel.dlen_16bit = false; // panel takes 18-bit SPI pixels; LGFX converts RGB565
  panel.bus_shared = true; // required for later SD/touch SPI integration
  _panel.config(panel);
  _lcd.setPanel(&_panel);
}

bool ILI9488LCDDisplay::begin() {
  if (_isOn) return true;
  // LVGL's RGB565 buffer is little-endian; LGFX handles the pixel conversion.
  _lcd.init();
  _lcd.setSwapBytes(true);
  _lcd.setRotation(0); // physically portrait: 320 wide x 480 tall
  setLogicalSize(_lcd.width(), _lcd.height());
  _lcd.fillScreen(0);
#if PIN_TFT_LEDA_CTL >= 0
  pinMode(PIN_TFT_LEDA_CTL, OUTPUT);
  digitalWrite(PIN_TFT_LEDA_CTL, HIGH);
#endif
  _isOn = true;
  Serial.printf("[P02] ILI9488 external LCD: %dx%d portrait, SPI=%u Hz (touch/SD disabled)\\n",
                width(), height(), (unsigned)ILI9488_SPI_WRITE_HZ);
  return true;
}

void ILI9488LCDDisplay::turnOn() {
  if (!_isOn) begin();
}

void ILI9488LCDDisplay::turnOff() {
  if (!_isOn) return;
#if PIN_TFT_LEDA_CTL >= 0
  digitalWrite(PIN_TFT_LEDA_CTL, LOW);
#endif
  _isOn = false;
}

void ILI9488LCDDisplay::clear() { _lcd.fillScreen(0); }
void ILI9488LCDDisplay::startFrame(ColorVal bg) { _lcd.fillScreen(bg); }
void ILI9488LCDDisplay::setTextSize(int sz) { _lcd.setTextSize(sz); }
void ILI9488LCDDisplay::setColor(ColorVal c) { _color = c; _lcd.setTextColor(c); }
void ILI9488LCDDisplay::setCursor(int x, int y) { _lcd.setCursor(x, y); }
void ILI9488LCDDisplay::print(const char* s) { _lcd.print(s); }
void ILI9488LCDDisplay::fillRect(int x, int y, int w, int h) { _lcd.fillRect(x, y, w, h, _color); }
void ILI9488LCDDisplay::drawRect(int x, int y, int w, int h) { _lcd.drawRect(x, y, w, h, _color); }
void ILI9488LCDDisplay::drawXbm(int x, int y, const uint8_t* bitmap, int w, int h) {
  _lcd.drawXBitmap(x, y, bitmap, w, h, _color);
}
uint16_t ILI9488LCDDisplay::getTextWidth(const char* s) { return (uint16_t)_lcd.textWidth(s); }
void ILI9488LCDDisplay::endFrame() {}

void ILI9488LCDDisplay::writePixelsRGB565(int x, int y, int w, int h, const uint16_t* pixels) {
  if (!_isOn || !pixels || w <= 0 || h <= 0) return;
  // Keep transfers synchronous: the 18-bit ILI9488 conversion cannot reuse
  // the R8/ST7789 driver 16-bit DMA fast-path without an RGB888 staging buffer.
  _lcd.startWrite();
  _lcd.setAddrWindow(x, y, w, h);
  _lcd.writePixels(const_cast<uint16_t*>(pixels), (uint32_t)w * h);
  _lcd.endWrite(); // releases bus for future microSD/touch transactions
}

void ILI9488LCDDisplay::setDisplayRotation(uint8_t rotation) {
  _lcd.setRotation(rotation & 3);
  setLogicalSize(_lcd.width(), _lcd.height());
}

void ILI9488LCDDisplay::panelSleep(bool sleep) {
  if (sleep) { _lcd.sleep(); delay(5); }
  else       { _lcd.wakeup(); delay(6); }
}
#endif // HELTEC_LORA_V4_R2_ILI9488
