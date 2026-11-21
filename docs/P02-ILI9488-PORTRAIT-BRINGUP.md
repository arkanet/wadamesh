# P02 — Heltec V4.3 R2, external 3.5-inch ILI9488, portrait 320×480

Status: **compile-only prototype**. This is **not** an electrical validation, a
firmware-to-flash approval, or a claim that the pictured KMRTM35018-SPI panel
has been verified to contain an ILI9488 controller.

## New build target

```sh
pio run -e heltec_v4_r2_ili9488_p02
```

The pre-existing, CI-green
`heltec_v4_tft_companion_radio_usb_tcp_touch` ST7789 target is preserved.

P02 uses `variants/heltec_v4/ILI9488LCDDisplay.{h,cpp}`, with:
- LovyanGFX `Panel_ILI9488`, SPI2 (FSPI), 16 MHz bring-up clock.
- 320 wide × 480 tall portrait at hardware rotation 0.
- RGB565 application buffer converted by LovyanGFX to the panel's SPI format.
- Synchronous per-band transfers (no reuse of ST7789's RGB565 DMA fast path).
- Display `bus_shared` and separate LoRa SPI bus, so P03/P04 can later
  integrate touch and SD without reworking SX1262 ownership.

## Candidate GPIO table — **NOT solder approved**

| TFT / peripheral signal | Heltec V4.3 R2 GPIO |
|---|---:|
| LCD SDI / future microSD MOSI / touch DIN | 33 |
| LCD SCK / future microSD SCK / touch CLK | 47 |
| LCD SDO / future microSD MISO / touch DOUT | 48 |
| LCD CS | 21 |
| LCD D/C | 16 |
| LCD RESET | 35 |
| LCD LED / backlight | 6 |
| microSD CS (future P04; idle HIGH in P02) | 43 |
| resistive touch CS (future P03; idle HIGH in P02) | 44 |
| touch IRQ | Unconnected for P02 |
| VCC / GND | Require measured and verified power compatibility |

**This provisional mapping is for a compile-only comparison across all three
firmwares.** Before wiring, confirm R2 schematic, TFT board revision/controller,
logic levels, power, inactive MISO tri-state, and repurposing of GPIO35/6/43/44.
GPIO47/48 are no longer CHSC6x I²C on this prototype. Buzzer, board expansion
IO and original I²C touch are disabled. SPI pins 8–14 for SX1262, GPS pins,
GNSS control, native USB pins and power control remain reserved as in P01.

The code uses LoRa on **HSPI/SPI3** and external LCD on **FSPI/SPI2**;
radio and LCD never share an SPI peripheral. The microSD and resistive touch
chip-selects are parked HIGH but those devices are **not yet initialized**.
Do not load SD maps, expect touch events, or upload this binary to the device.

## Next gates

1. Compile P02 separately in CI. If red, fix only build integration errors.
2. Inspect the actual panel/touch-controller markings; approve the wiring.
3. P03: resistive SPI touch driver and calibration, without CHSC6x assumptions.
4. P04: microSD shared-SPI arbitration and existing PNG / PMTiles map stack.
5. P05: LVGL portrait geometry, contact list and map usability, hardware tests.
6. Adapt the same approved GPIO mapping to Meshtastic and DarkMesh variants.
