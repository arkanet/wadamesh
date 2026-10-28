#include <Arduino.h>
#include <SPI.h>
#include "WadaPiScreen35.h"
#include "WadaXPT2046.h"

// This standalone test intentionally does not initialize the SX1262 radio.
static constexpr int LCD_CS=35, LCD_DC=40, LCD_RST=41, TOUCH_CS=6;
static SPIClass bus(HSPI);
static WadaPiScreen35 lcd(bus,LCD_CS,LCD_DC,LCD_RST,TOUCH_CS);
static WadaXPT2046 touch(bus,LCD_CS,TOUCH_CS);

void setup() {
  Serial.begin(115200);
  delay(1200);
  Serial.println("\n[PISCREEN] Boot 480x320 / XPT2046 raw");
  pinMode(LCD_CS, OUTPUT);
  pinMode(TOUCH_CS, OUTPUT);
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(TOUCH_CS, HIGH);
  bus.begin(47,48,33,LCD_CS);
  lcd.begin(2000000);
  touch.begin();
  Serial.println("[PISCREEN] LCD initialization complete");
}

void loop() {
  static const uint16_t colors[] = {0xF800,0x07E0,0x001F,0x0000};
  static uint8_t colorIdx=0;
  static uint32_t lastColor=0;
  static uint32_t lastTouch=0;
  const uint32_t now=millis();
  if (now-lastColor>=3000) {
    lastColor=now;
    lcd.fillRect(0,0,480,320,colors[colorIdx]);
    Serial.printf("[PISCREEN] color index %u\n",unsigned(colorIdx));
    colorIdx=(colorIdx+1)%4;
  }
  if (now-lastTouch>=250) {
    lastTouch=now;
    WadaTouchRaw raw=touch.read();
    Serial.printf("[TOUCH] X=%u Y=%u Z1=%u Z2=%u P=%ld\n",
                  raw.x,raw.y,raw.z1,raw.z2,(long)raw.pressure);
  }
  delay(10);
}
