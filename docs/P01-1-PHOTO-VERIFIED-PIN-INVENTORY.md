# P01.1 photographed pin inventory — 2026-11-21

Hardware owner supplied three photographs and confirmed willingness to solder directly to PCB pads.

## Heltec V4.3 R2 header labels, left to right as photographed

Top: `7 6 5 4 3 2 1 38 39 40 41 42 45 16 37 3V3 3V3 GND`

Bottom: `19 20 21 26 48 47 33 34 35 36 0 RST 43 44 Ve Ve 5V GND`

These are *physical silkscreen labels*, not proof that all pins are free for use. Existing firmware uses GPIO33 for its older ST7789 TFT, 47/48 for CHSC6x touch, 21 for backlight, and GPIO34/35/36 for board functions. GPIO26 may be memory-reserved; GPIO19/20 USB; GPIO45 strapping. Verify the exact board schematic before reassignment.

## TFT photograph

Back silkscreen: `KMRTM35018-SPI`, `3.5" TFT SPI 480*320 V1.0`.

14-pin TFT/touch header: `VCC GND CS RESET D/C SDI SCK LED SDO(MISO) T_CLK T_CS T_DIN T_OUT T_IRQ`.

Four independent SD solder pads: `SD_CS SD_MOSI SD_MISO SD_SCK`.

Touch signals imply a separate SPI touch controller; determine controller identity (often XPT2046 but **not confirmed**) from IC marking/schematic before enabling any driver. Do not reuse the original CHSC6x touch configuration blindly.

## Candidate shared wiring topology (not yet solder-approved)

- TFT SDI, SD_MOSI, T_DIN: common SPI MOSI (after checking electrical compatibility)
- TFT SCK, SD_SCK, T_CLK: common SPI SCLK
- TFT SDO, SD_MISO, T_OUT: common SPI MISO **only if non-selected outputs tri-state properly**
- TFT CS, SD_CS, T_CS: three independent chip selects
- TFT D/C: independent GPIO
- TFT RESET: independent GPIO or validated reset circuit
- TFT LED: controlled backlight or validated always-on circuit
- T_IRQ: optional input
- VCC and GND: measured and properly rated supply

Shared TFT+SD+touch uses 3 SPI data/clock + 3 CS + D/C + RESET + BL = 9 GPIO if all controlled; optional IRQ is a tenth. Reduced GPIO configurations need electrical justification. Do not tie two CS signals together.

## Next engineering gates
1. Exact V4.3 R2 schematic and firmware pin collision matrix.
2. Identify actual TFT/touch ICs and signal voltage; confirm power consumption.
3. Determine GPIO allocation and verify that radio/GPS/USB remain operational.
4. Implement separate PlatformIO env and build/test after pin approval.
