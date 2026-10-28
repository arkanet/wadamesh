#include <Arduino.h>
#include <SPI.h>
#include "WadaST7796S40.h"
#include "WadaXPT2046.h"

static constexpr int LCD_CS=35,LCD_DC=40,LCD_RST=41,TOUCH_CS=6;
static SPIClass bus(HSPI);
static WadaST7796S40 lcd(bus,LCD_CS,LCD_DC,LCD_RST,TOUCH_CS);
static WadaXPT2046 touch(bus,LCD_CS,TOUCH_CS);

void setup() {
  Serial.begin(115200);
  delay(1200);
  Serial.println("\n[LAFVIN] ST7796S 480x320 / XPT2046 diagnostic");
  pinMode(LCD_CS,OUTPUT);
  pinMode(TOUCH_CS,OUTPUT);
  digitalWrite(LCD_CS,HIGH);
  digitalWrite(TOUCH_CS,HIGH);
  bus.begin(47,48,33,LCD_CS);
  lcd.begin(2000000);
  touch.begin();
  Serial.println("[LAFVIN] LCD init returned");
}

void loop() {
  static const uint16_t colors[] = {0xF800,0x07E0,0x001F,0x0000};
  static uint32_t lastDraw=0,lastTouch=0;
  static uint8_t index=0;
  const uint32_t now=millis();
  if (now-lastDraw>=3000) {
    lastDraw=now;
    lcd.fillRect(0,0,480,320,colors[index]);
    Serial.printf("[LAFVIN] fill %u\n",unsigned(index));
    index=(index+1)%4;
  }
  if (now-lastTouch>=250) {
    lastTouch=now;
    const WadaTouchRaw r=touch.read();
    Serial.printf("[TOUCH] X=%u Y=%u Z1=%u Z2=%u P=%ld\n",
                  r.x,r.y,r.z1,r.z2,(long)r.pressure);
  }
  delay(10);
}
