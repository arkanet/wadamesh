#pragma once
// XPT2046 on the tested external PiScreen SPI wiring (Heltec V4.3 S3R2).
// Hardware touch readings still require physical validation.
#if defined(WADA_PISCREEN_35)
#include <Arduino.h>
#include <SPI.h>

struct WadaTouchRaw {
  uint16_t x = 0, y = 0, z1 = 0, z2 = 0;
  int32_t pressure = 0;
};

class WadaXPT2046 {
public:
  WadaXPT2046(SPIClass& spi, int lcdCS, int touchCS)
    : spi_(spi), lcdCS_(lcdCS), touchCS_(touchCS) {}
  void begin() {
    pinMode(lcdCS_, OUTPUT); pinMode(touchCS_, OUTPUT);
    digitalWrite(lcdCS_, HIGH); digitalWrite(touchCS_, HIGH);
  }
  WadaTouchRaw read(uint32_t hz = 1000000) {
    spi_.beginTransaction(SPISettings(hz, MSBFIRST, SPI_MODE0));
    digitalWrite(lcdCS_, HIGH);
    digitalWrite(touchCS_, LOW);
    readADC(0xD0); // discard first conversion after selecting the controller
    WadaTouchRaw out;
    out.x = readADC(0xD0);
    out.y = readADC(0x90);
    out.z1 = readADC(0xB0);
    out.z2 = readADC(0xC0);
    digitalWrite(touchCS_, HIGH);
    spi_.endTransaction();
    out.pressure = 4095L + int32_t(out.z1) - int32_t(out.z2);
    if (out.pressure < 0) out.pressure = 0;
    return out;
  }
private:
  uint16_t readADC(uint8_t cmd) {
    spi_.transfer(cmd);
    const uint16_t a = spi_.transfer(uint8_t(0));
    const uint16_t b = spi_.transfer(uint8_t(0));
    return ((a << 8) | b) >> 3 & 0x0FFF;
  }
  SPIClass& spi_;
  int lcdCS_, touchCS_;
};
#endif
