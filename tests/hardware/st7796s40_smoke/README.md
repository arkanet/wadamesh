# LAFVIN 4.0-inch ST7796S / XPT2046 — Heltec V4.3 S3R2

**Status: driver and standalone test committed; no LAFVIN hardware verification, no LVGL/WadaMesh production target yet.** This display is **not** interchangeable at firmware level with the Raspberry Pi 3.5-inch PiScreen ILI9486. It uses standard SPI byte commands, whereas the tested PiScreen needs 16-bit command framing.

## LAFVIN 14-position header -> Heltec V4.3 S3R2

| LAFVIN pin label | Heltec pin | Notes |
| --- | --- | --- |
| VCC | 3V3 **or** 5V (one supply only) | Choose after confirming hardware supply budget; do NOT connect both |
| GND | GND | Common ground |
| CS | GPIO35 | LCD chip select; conflicts with expansion IO/button |
| RESET | GPIO41 | LCD reset; conflicts with GNSS/JTAG role |
| DC/RS | GPIO40 | LCD D/C; conflicts with GNSS role |
| SDI(MOSI) | GPIO33 | Shared SPI MOSI |
| SCK | GPIO47 | Shared SPI clock |
| LED | 3V3 if always-on and load is permitted | Check module backlight control circuit before attaching to GPIO |
| SDO(MISO) | unconnected initially | LCD read not used |
| T_CLK | GPIO47 | Same SPI SCK |
| T_CS | GPIO6 | Touch CS; disables WadaMesh buzzer function |
| T_DIN | GPIO33 | Touch SPI MOSI |
| T_DO | GPIO48 | Touch SPI MISO |
| T_IRQ | unconnected initially | Polling, optional future IRQ |

The manufacturer-style product specification states ST7796S, 480x320,
resistive touch controller XPT2046, VCC 3.3–5 V. This mapping is a
**candidate** until verified with the physical panel and supply.

**Do not use the Raspberry Pi 40-position female pinout for this module.**
Do not attach PiScreen and LAFVIN simultaneously to the Heltec.
Radio's separate SX1262 bus remains unmodified.

## Build, upload and inspect

From `tests/hardware/st7796s40_smoke`:

```sh
pio run
pio run -t upload
pio device monitor -b 115200
```

The standalone target exercises red/green/blue/black full-screen updates and
raw XPT2046 readings every 250ms. Avoid inferring that touch works from
unfiltered idle values; confirm while touching corners and centre.

The LAFVIN setup uses a conservative 2 MHz SPI clock, RGB565, SPI mode 0,
landscape MADCTL=0x28, and a minimal ST7796S unlock/sleep-out/display-on
sequence. Module revision can require additional panel-specific power, porch,
gamma, rotation or inversion settings. Serial output means the commands were
issued, not that a physical screen accepted them.

## WadaMesh integration status

The branch contains *two independent hardware LCD transports*:
- `WadaPiScreen35`: ILI9486-compatible Raspberry Pi adapter, whose SPI
  command framing and 480x320 window have been tested on actual hardware.
- `WadaST7796S40`: conventional SPI ST7796S LAFVIN candidate.
- `WadaXPT2046`: shared raw SPI touch transport; calibration is not yet validated.

Remaining work: integrate each driver into WadaMesh display abstraction and LVGL
flush callback; implement 480x320 UI, rotation-aware five-point calibration,
storage/recovery and radio/companion regression tests. Existing WadaMesh ST7789
and CHSC6x configurations must remain unaffected.

No PR, no main changes.
