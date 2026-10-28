#pragma once
// LAFVIN 4.0 inch ST7796S (480x320) conventional 8-bit SPI command transport.
// Standalone opt-in driver. Does not change existing ST7789/PiScreen paths.
// Unverified on physical LAFVIN module; initial configuration is diagnostic.
#if defined(WADA_ST7796S_40)
#include <Arduino.h>
#include <SPI.h>

class WadaST7796S40 {
public:
  static constexpr uint16_t WIDTH = 480;
  static constexpr uint16_t HEIGHT = 320;

  WadaST7796S40(SPIClass& spi, int cs, int dc, int rst, int touchCs)
    : spi_(spi), cs_(cs), dc_(dc), rst_(rst), touchCs_(touchCs) {}

  void begin(uint32_t hz = 2000000) {
    hz_ = hz;
    pinMode(cs_, OUTPUT);
    pinMode(dc_, OUTPUT);
    pinMode(rst_, OUTPUT);
    pinMode(touchCs_, OUTPUT);
    digitalWrite(cs_, HIGH);
    digitalWrite(touchCs_, HIGH);
    digitalWrite(rst_, HIGH); delay(50);
    digitalWrite(rst_, LOW); delay(100);
    digitalWrite(rst_, HIGH); delay(200);

    command(0x01); delay(150);  // SWRESET
    command(0x11); delay(150);  // SLPOUT

    const uint8_t f0Enable[] = {0xC3};
    const uint8_t f0Enable2[] = {0x96};
    command(0xF0, f0Enable, sizeof(f0Enable));
    command(0xF0, f0Enable2, sizeof(f0Enable2));
    const uint8_t madctl[] = {0x28};  // landscape; requires on-panel verification
    command(0x36, madctl, sizeof(madctl));
    const uint8_t pixfmt[] = {0x55};  // RGB565
    command(0x3A, pixfmt, sizeof(pixfmt));
    const uint8_t f0Disable[] = {0x3C};
    const uint8_t f0Disable2[] = {0x69};
    command(0xF0, f0Disable, sizeof(f0Disable));
    command(0xF0, f0Disable2, sizeof(f0Disable2));
    command(0x29); delay(100);  // DISPON
  }

  void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t rgb565) {
    if (!w || !h || x >= WIDTH || y >= HEIGHT) return;
    if (w > WIDTH-x) w = WIDTH-x;
    if (h > HEIGHT-y) h = HEIGHT-y;
    const uint16_t x1=x+w-1, y1=y+h-1;
    const uint8_t col[] = {uint8_t(x>>8),uint8_t(x),uint8_t(x1>>8),uint8_t(x1)};
    const uint8_t row[] = {uint8_t(y>>8),uint8_t(y),uint8_t(y1>>8),uint8_t(y1)};
    command(0x2A,col,sizeof(col));
    command(0x2B,row,sizeof(row));
    spi_.beginTransaction(SPISettings(hz_, MSBFIRST, SPI_MODE0));
    digitalWrite(touchCs_, HIGH);
    digitalWrite(cs_, LOW);
    digitalWrite(dc_, LOW);
    spi_.transfer(uint8_t(0x2C)); // RAMWR: command and pixels in one CS assertion
    digitalWrite(dc_, HIGH);
    const uint32_t count=uint32_t(w)*uint32_t(h);
    for (uint32_t i=0; i<count; ++i) {
      spi_.transfer(uint8_t(rgb565>>8));
      spi_.transfer(uint8_t(rgb565));
      if ((i & 0x1FFFu)==0) yield();
    }
    digitalWrite(cs_, HIGH);
    spi_.endTransaction();
  }

private:
  void command(uint8_t c, const uint8_t* data=nullptr, size_t n=0) {
    spi_.beginTransaction(SPISettings(hz_,MSBFIRST,SPI_MODE0));
    digitalWrite(touchCs_,HIGH);
    digitalWrite(cs_,LOW);
    digitalWrite(dc_,LOW);
    spi_.transfer(c);
    digitalWrite(dc_,HIGH);
    for(size_t i=0;i<n;++i) spi_.transfer(data[i]);
    digitalWrite(cs_,HIGH);
    spi_.endTransaction();
  }

  SPIClass& spi_;
  int cs_,dc_,rst_,touchCs_;
  uint32_t hz_=2000000;
};
#endif
