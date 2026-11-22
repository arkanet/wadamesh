# P03 — Optional external resistive SPI touch (XPT2046 family)

Status: **compile-only; electrical and chip-identity validation outstanding**.

P02 remains the unchanged LCD-only target and passed the baseline CI.
The photographed module is marked `KMRTM35018-SPI V1.0`, with `T_CLK`,
`T_CS`, `T_DIN`, `T_OUT`, `T_IRQ` at the separate touch header.
Comparable boards commonly contain XPT2046; this **does not prove** the
exact chip fitted on the user's PCB.

## Build variants

```bash
pio run -e heltec_v4_r2_ili9488_p02  # LCD only; touch disabled
pio run -e heltec_v4_r2_ili9488_p03  # LCD + optional XPT2046 experiment
```

The original `heltec_v4_tft_companion_radio_usb_tcp_touch` ST7789
target remains independent. **Neither experimental binary is approved
for flashing or wiring yet.**

## Signal plan (candidate, NOT a solder map)

| TFT marking | Heltec V4.3 R2 GPIO | Electrical role |
|---|---:|---|
| T_CLK | 47 | shared FSPI/SPI2 SCLK |
| T_DIN | 33 | shared FSPI/SPI2 MOSI |
| T_OUT | 48 | shared FSPI/SPI2 MISO |
| T_CS | 44 | independent touch chip select |
| T_IRQ | disconnected | polled by LVGL (optional interrupt later) |
| CS (LCD) | 21 | independent LCD chip select |
| SD_CS (microSD) | 43 | parked HIGH, SD not mounted |

Additional candidate LCD GPIOs are recorded in `P02-ILI9488-PORTRAIT-BRINGUP.md`.

Before connecting: confirm chip marking, supply and logic levels, pin
multiplexing on R2, ownership of GPIO43/44 (UART0), GPIO47/48
(original capacitive I²C), shared SDO and MISO inactive tri-state.
The LoRa SX1262 uses another SPI host; its GPIO8..14 remain reserved.

## P03 driver

- Uses **LovyanGFX Touch_XPT2046** in the existing LCD driver, on the same
  SPI2 host with a 1 MHz touch clock and separate `T_CS`.
- Exposes the existing WadaMesh `heltecV4CapTouch*` API to LVGL; avoids
  CHSC6x and T-Deck GT911 I²C buses entirely.
- Polls inline on LVGL calls, without a second background task while SPI
  arbitration has not been characterized on real hardware.
- 12-bit raw touch position transformed to 320×480 portrait, supporting
  rotate 0/90/180/270 and swipe detection.
- Temporary four-endpoint calibration is build-time configurable:
  `P03_TOUCH_X_MIN/MAX`, `P03_TOUCH_Y_MIN/MAX`,
  `P03_TOUCH_SWAP_XY`, `P03_TOUCH_FLIP_X/Y`.
  Defaults (X 300–3900; Y 400–3900) are **placeholders**, not calibrated
  for the actual panel. Raw values are exposed through
  `heltecV4CapTouchGetRaw()` for future on-device corner capture.
- Tap/swipe code requires hardware testing to tune filtering, phantom
  touches, sampling latency, gesture thresholds and direction.
- Calibrating and saving parameters via the settings UI is **not yet
  implemented**; only compile-time calibration is present.

## Hardware validation gate

1. Photograph and read the actual touch IC marking and power-stage markings.
2. Verify T_OUT releases MISO while T_CS is HIGH, alongside LCD and SD.
3. Verify R2 GPIO ownership, 3.3V logic and safe backlight power.
4. Record 4–5 raw touches at screen corners and center at portrait rotation 0.
5. Set X/Y min/max and swap/flip, then recompile P03.
6. Check tap, long-press, scroll, swipe, map pan and rotation changes.
7. Repeat while SX1262 receives and transmits. Record RSSI, missed RX,
   watchdog resets, largest block duration and heap.
8. Only then begin P04 microSD mounting and concurrent traffic tests.

Expected acceptance: physical 320×480 touch coordinate error < 12 pixels
at 5 points, no repeated false presses, no SD/LoRa regression after
integrating P04. These thresholds are **test goals**, not measured results.
