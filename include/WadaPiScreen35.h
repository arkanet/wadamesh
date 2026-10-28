#pragma once
// Standalone PiScreen-compatible ILI9486 transport for Heltec V4.3 S3R2.
// Opt-in only; existing ST7789/CHSC6x WadaMesh environments are unaffected.
#if defined(WADA_PISCREEN_35)
#include <Arduino.h>
#include <SPI.h>

class WadaPiScreen35 {
public:
  static constexpr uint16_t WIDTH = 480;
  static constexpr uint16_t HEIGHT = 320;
  WadaPiScreen35(SPIClass& spi, int cs, int dc, int rst, int touchCs)
    : spi_(spi), cs_(cs), dc_(dc), rst_(rst), touchCs_(touchCs) {}

  void begin(uint32_t spiHz = 2000000) {
    hz_ = spiHz;
    pinMode(cs_, OUTPUT); pinMode(dc_, OUTPUT);
    pinMode(rst_, OUTPUT); pinMode(touchCs_, OUTPUT);
    digitalWrite(cs_, HIGH); digitalWrite(touchCs_, HIGH);
    digitalWrite(rst_, HIGH); delay(50);
    digitalWrite(rst_, LOW); delay(100);
    digitalWrite(rst_, HIGH); delay(200);
    cmd1(0xB0, 0x00);
    cmd(0x11); delay(250);
    cmd1(0x3A, 0x55);   // RGB565
    cmd1(0x36, 0x28);   // Tested landscape 480x320 window
    cmd1(0xC2, 0x44);
    static const uint8_t c5[] = {0, 0, 0, 0};
    cmd(0xC5, c5, sizeof(c5));
    static const uint8_t gp[] = {
      0x0F,0x1F,0x1C,0x0C,0x0F,0x08,0x48,0x98,0x37,0x0A,0x13,0x04,0x11,0x0D,0x00
    };
    static const uint8_t gn[] = {
      0x0F,0x32,0x2E,0x0B,0x0D,0x05,0x47,0x75,0x37,0x06,0x10,0x03,0x24,0x20,0x00
    };
    cmd(0xE0, gp, sizeof(gp));
    cmd(0xE1, gn, sizeof(gn));
    cmd(0xE2, gn, sizeof(gn));
    cmd(0x11); delay(120);
    cmd(0x29); delay(100);
  }

  // RGB565, single landscape rectangle, inclusive bounds. Keeps LCD CS asserted
  // through pixel transfer; SPI transactions must never overlap the XPT2046.
  void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t rgb565) {
    if (!w || !h || x >= WIDTH || y >= HEIGHT) return;
    if (w > WIDTH - x) w = WIDTH - x;
    if (h > HEIGHT - y) h = HEIGHT - y;
    const uint16_t x1 = x + w - 1, y1 = y + h - 1;
    const uint8_t col[] = {uint8_t(x >> 8), uint8_t(x), uint8_t(x1 >> 8), uint8_t(x1)};
    const uint8_t row[] = {uint8_t(y >> 8), uint8_t(y), uint8_t(y1 >> 8), uint8_t(y1)};
    cmd(0x2A, col, sizeof(col));
    cmd(0x2B, row, sizeof(row));
    cmd(0x2C);
    spi_.beginTransaction(SPISettings(hz_, MSBFIRST, SPI_MODE0));
    digitalWrite(touchCs_, HIGH);
    digitalWrite(cs_, LOW); digitalWrite(dc_, HIGH);
    for (uint32_t i = 0, n = uint32_t(w)*h; i < n; ++i) {
      spi_.transfer(uint8_t(rgb565 >> 8)); spi_.transfer(uint8_t(rgb565));
      if ((i % 7680) == 0) yield();
    }
    digitalWrite(cs_, HIGH);
    spi_.endTransaction();
  }

private:
  void word8(uint8_t v) { spi_.transfer(uint8_t(0)); spi_.transfer(v); }
  void cmd(uint8_t id, const uint8_t* data = nullptr, size_t len = 0) {
    spi_.beginTransaction(SPISettings(hz_, MSBFIRST, SPI_MODE0));
    digitalWrite(touchCs_, HIGH); digitalWrite(cs_, LOW);
    digitalWrite(dc_, LOW); word8(id);
    digitalWrite(dc_, HIGH);
    for (size_t i = 0; i < len; ++i) word8(data[i]);
    digitalWrite(cs_, HIGH); spi_.endTransaction();
  }
  void cmd1(uint8_t id, uint8_t value) { cmd(id, &value, 1); }
  SPIClass& spi_;
  int cs_, dc_, rst_, touchCs_;
  uint32_t hz_ = 2000000;
};
#endif
