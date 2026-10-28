# Heltec V4.3 + Raspberry Pi 3.5-inch SPI display — hardware bring-up

Status: **research / no wiring / no firmware flash**  
Work branch: `feat/heltec-v43-ili9486-xpt2046` (no PR).

## Fixed project scope

- Fixed repeater: Heltec T096, MeshCore Repeater (separate from this branch).
- Portable companion: Heltec WiFi LoRa 32 **V4.3** + recycled 3.5-inch Raspberry Pi TFT/touch.
- Preserve the existing upstream WadaMesh Heltec V4 TFT target. New board profile only; do not modify `main`.

## Observed hardware, verified on Raspberry Pi 4

Reference supplied by user in `meshcore-display-reference.tar.gz`, kept outside GitHub until a review of sensitive config content.

- Display marking: `3.5inch RPi Display`, 480×320, XPT2046 Touch Controller.
- Raspberry: Debian 13, kernel 6.18.33+rpt-rpi-v8.
- Known-working overlay: `dtoverlay=piscreen,speed=16000000,rotate=90`.
- Linux: `fb_ili9486`, `regwidth=16`, `buswidth=8`, 480×320 fullscreen.
- SPI panel on Raspberry SPI0 CE0; touch ADS7846-compatible on SPI0 CE1, maximum 2 MHz.
- Raspberry GPIOs in overlay: reset=25; D/C=24; backlight=22; touch interrupt=17.
- Touch input `/dev/input/event0`: varying X/Y, pressure and release demonstrated.
- Actual LCD silicon ID **not read**: ILI9486-*compatible* is demonstrated, exact IC remains unconfirmed.
- PiScreen overlay contains a custom `init` property: Linux's generic ILI9486 initialization was **not** sufficient (partial screen/garbled text). A port must reproduce and validate the panel-specific sequence, including its unusual 16-bit register convention.

## WadaMesh source inspection (fork main at 8a2646b5bdbe42b8fd790967677ede8e9a8829c1)

`platformio.ini` existing environment `heltec_v4_tft_companion_radio_usb_tcp_touch`:

- Radio: SX1262 SPI NSS=8 SCK=9 MOSI=10 MISO=11, RESET=12 BUSY=13 DIO1=14.
- RF frontend controls: 2, 5, 7, **46** are used; never appropriate for TFT.
- TFT in existing expansion kit: ST7789, 240×320, SCLK=17 MOSI=33 CS=15 DC=16 RESET=18 BL=21.
- Touch in existing expansion kit: **CHSC6x over I2C**, SDA=47 SCL=48 RST=44, not compatible with XPT2046 SPI without a new driver.
- Other allocated peripherals include GPS (38,39,42,34), power / ADC (36,37,1), user button=0, board I2C=4/3, buzzer=6, expansion button=35.
- Separate target `heltec_v4_r8_tft_companion_radio_usb_tcp_touch` changes several pins **and** uses octal PSRAM that occupies GPIO33–37. This project must identify the exact chip/module **before** locking the pin map. WadaMesh board JSONs differentiate 2 MB QSPI PSRAM vs 8 MB OPI PSRAM.

## Electrical and software gates before any wiring

1. Identify the exact V4.3 physical module: S3R2 vs S3R8; verify exposed GPIOs against the *same* variant's Heltec schematic and PCB connectors.
2. Verify the TFT module's power supply (Raspberry 5 V header input), current draw, ground, logic level compatibility, and any on-board level conversion; **do not** feed 5 V into ESP32-S3 GPIOs.
3. Need display SCK, MOSI, CS, DC, RESET, backlight; touch MISO, CS, IRQ, plus common SPI SCK/MOSI (if actual breakout follows PiScreen overlay). Choose independent SPI bus if feasible, and explicitly resolve all pin conflicts.
4. Preserve the MeshCore radio SPI and RF frontend controls; no changes to their mapping.
5. Implement LCD controller init and touch as a separate compile-time target/profile, not by redefining ST7789 or CHSC6x globally. Add a minimal test (color bars, touch raw/calibration) before enabling LVGL or transmitting.
6. Check WadaMesh UI layout assumptions for 240×320 before changing to 480×320; avoid assuming the existing touch coordinate transforms scale automatically.
7. Flash only after valid electrical wiring and successful build; user has not yet tested on V4.3.

## Suggested follow-on commits (not yet implemented)

- hardware wiring decision + documented schematic and supply checks;
- isolated ILI9486-compatible driver with known PiScreen init;
- XPT2046 touch driver and calibration;
- WadaMesh 480×320 LVGL integration;
- repeatable PlatformIO build, hardware test checklist, radio regression.

No pull requests. No main-branch changes.


## Hardware bring-up results — 27–28 October 2026

These supersede the earlier unpowered/unwired research status above.

- **Physical board**: Heltec V4.3 S3R2, ESP32-S3; runtime confirmed PSRAM **2,097,152 bytes** and flash **16,777,216 bytes**.
- **Power**: TFT Raspberry header physical 1 -> 3V3, 2 -> 5V, 6 -> GND; backlight illuminated. Continuity: 2↔4, 1↔17, 6↔9. Unpowered resistance: 2↔1 OL, 2↔6 ~10MOhm, 1↔6 OL. Power budget and long-term thermal stability still require validation.
- **Tested signal map**: Raspberry physical pin 18 -> GPIO40 (LCD D/C), 19 -> GPIO33 (MOSI), 21 -> GPIO48 (touch MISO), 22 -> GPIO41 (RESET), 23 -> GPIO47 (SCLK), 24 -> GPIO35 (LCD CS), 26 -> GPIO6 (touch CS); touch IRQ pin 11 unconnected.
- **Panel proven**: Standalone ESP32-S3 Arduino HSPI driver, SPI mode 0 at 2 MHz; PiScreen register-byte convention is two SPI bytes `00 xx`; pixel data is RGB565 two bytes. Init sequence tested: B0/11/3A/36/C2/C5/E0/E1/E2/11/29. `MADCTL=0x28` and GRAM window **480×320** completely fill the visible display with changing colours. A 320×480 GRAM window only filled part of the panel.
- **TFT_eSPI caveat**: Its initial ILI9486 configuration reset with watchdog; later it ran but did not draw despite wiring continuity. Do not substitute it for the tested PiScreen-specific 16-bit command transport.
- **Touch**: XPT2046 SPI polling code was supplied for testing, but real calibration samples and five-point confirmation were not recorded yet. Treat touch as **unverified**, not proven.
- **Initial integration commit**: `include/WadaPiScreen35.h` adds an **opt-in** `WADA_PISCREEN_35` class, preserving tested initialization and fillRect. It is not yet attached to WadaMesh DISPLAY_CLASS / LVGL; existing ST7789/CHSC6x environment stays unchanged.
- **Critical conflicts**: The existing Heltec expansion target uses GPIO35 for expansion key, GPIO6 for buzzer, GPIO40/41 in GNSS-related functionality. A new dedicated environment must disable conflicting uses instead of inheriting them silently.
- **Next gates**: integrate panel with WadaMesh display abstraction; then XPT2046 polling, raw data verification, 5-point landscape calibration persisted with recovery/reset; finally LVGL 480×320 and SX1262 regression tests. No PR, never modify main.

