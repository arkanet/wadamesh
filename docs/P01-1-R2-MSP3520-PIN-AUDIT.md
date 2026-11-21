# P01.1 — R2 pin audit and MSP3520 integration gate

Date: 2026-11-21. Branch: TFT-SPI-480×320.

## Repository findings (verified)
The existing `platformio.ini` target `heltec_v4_tft_companion_radio_usb_tcp_touch` is **not** an MSP3520 target. It configures ST7789 240x320 and TFT_eSPI with MOSI=33, SCLK=17, CS=15, DC=16, RST=18, BL=21. It also has CHSC6x capacitive touch (SDA=47, SCL=48, RST=44). Radio pins are NSS=8, SCLK=9, MOSI=10, MISO=11, RESET=12, BUSY=13, DIO1=14. GPS RX=38, TX=39, RESET=42, EN=34; board I2C=4/3. GPIO35 is expansion IO. Other board functions include GPIO1 battery, 2/5/7 RF controls, 36 VEXT and 37 ADC control.

The repository README already documents an offline map pipeline: XYZ PNG tiles, read-only PNG PMTiles v3, folder fallback, authenticated Wi-Fi transfer and map settings. **Reuse it**; do not create a second incompatible map engine. Existing documentation says JPEG folder fallback is supported but JPEG PMTiles are not.

## User-confirmed accessible external header labels
Top row, left to right: 7 6 5 4 3 2 1 38 39 40 41 42 45 16 37 3V3 3V3 GND.
Partial bottom row previously supplied: 19 20 21 26 ... (remaining pins not established in current record).

## Electrical availability — do not mistake firmware assignment for free external GPIO
- GPIO8–14: reserved SX1262. DO NOT reuse.
- GPIO1,2,5,7,37,38,39,42: board/radio/GPS/battery controls; DO NOT reuse.
- GPIO3/4: board I2C; avoid unless I2C intentionally disabled and checked.
- GPIO16: external header, but current TFT DC assignment; candidate for revised display only after audit.
- GPIO40/41/45: externally labeled and candidates for investigation, **not approved**; GPIO45 is a strapping pin on ESP32-S3, so external pull levels at reset require scrutiny.
- GPIO19/20: native USB; preserve USB flashing/serial.
- GPIO21: current TFT backlight in old target; confirm header wiring and any on-board loading.
- GPIO26: do not assign before checking flash/PSRAM wiring.
- GPIO6: existing buzzer. GPIO35 expansion IO. GPIO34 GPS enable, GPIO36 VEXT.
- GPIO15,17,18,33: existing TFT wiring uses these but they are not present in the user's *reported top row*. Do not assume they are available or accessible until full bottom header and schematic are audited.
- GPIO47/48/44: existing capacitive touch, not necessarily suitable or present on this external module.

## MSP3520 required signals
TFT: VCC, GND, LCD_CS, LCD_RST, LCD_DC/RS, LCD_SDI/MOSI, LCD_SCK, LCD_LED, optionally LCD_SDO/MISO.
SD pads: SD_SCK, SD_MISO, SD_MOSI, SD_CS.
Resistive touch if fitted: T_CLK, T_CS, T_DIN, T_DO, T_IRQ; optional, NOT to be confused with CHSC6x.
A shared TFT+SD SPI bus requires SCK/MOSI and possibly MISO shared, separate CS lines, validated inactive tri-state behavior and serial access; minimum 5 control/data GPIOs for write-only LCD+SD (SCK, MOSI, MISO, LCD_CS, SD_CS) plus LCD_DC, usually LCD_RST and controllable BL. This is **at least 7–8 GPIOs**, unless hardware reset/backlight are tied appropriately and safe. Therefore there is no confirmed complete pin allocation from the available external header information. Never publish a speculative solder table.

## Implementation plan
1. Read full R2 schematic, full bottom header legend and existing board use; perform continuity checks on target board.
2. Decide whether shared SPI with the existing radio is feasible with separate CS and bus arbitration or whether a small SPI GPIO expander/alternate interface is required. Preserve radio responsiveness.
3. Introduce a separate `heltec_v4_r2_msp3520_sd` PlatformIO environment, not a change to the stable ST7789 environment.
4. Configure ILI9488 320x480 portrait, appropriate pixel format and LVGL flush buffer; select touch driver only if actual resistive controller is present and wired.
5. Reuse existing SD initialization, PMTiles PNG reader and map UI; only adapt board SD HAL and screen geometry.
6. Build and test, then hardware test; no flashing until electrical mapping is approved.

## Gate
P01.1 is **partially complete** (firmware inventory); the wiring table is BLOCKED on full bottom-row header and board schematic/electrical confirmation. The current branch contains documentation only, not tested firmware changes.
