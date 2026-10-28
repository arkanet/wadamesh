# Heltec V4.3 S3R2 PiScreen 480x320: isolated hardware smoke test

This is intentionally **not** the WadaMesh firmware. It validates the newly
isolated ILI9486 PiScreen and raw XPT2046 transports without any WadaMesh
LVGL, MeshCore, SX1262, GPS, expansion-button or buzzer initialization.

The LCD's full-screen colour cycling already worked using this pinout in a
separate standalone sketch. The integrated `WadaPiScreen35` class and its
`fillRect` API need their own on-device confirmation; XPT2046 raw readings
have not been provided by the tester yet.

## Connections

| Raspberry TFT physical pin | Heltec V4.3 GPIO/rail |
| --- | --- |
| 1 | 3V3 |
| 2 | 5V |
| 6 | GND |
| 18 LCD DC | 40 |
| 19 MOSI | 33 |
| 21 MISO | 48 |
| 22 LCD reset | 41 |
| 23 clock | 47 |
| 24 LCD CS | 35 |
| 26 touch CS | 6 |
| 11 touch IRQ | disconnected |

Do not connect the Raspberry and Heltec to the TFT together.
The 3V3 rail power budget and logic levels should be validated under load.

## Build and test

From this directory:
```sh
pio run
pio run -t upload
pio device monitor -b 115200
```

The serial monitor reports `[PISCREEN]` and `[TOUCH]`. Compare raw touch
samples at rest, center, and each corner before adding calibration.
No touch-pressure threshold, rotation transform or UI calibration can yet be
claimed verified. An SPI response may be unstable until the physical panel's
touch CS and ADC sample settling are validated.

## Integration still pending

- Attach an ILI9486-compatible display implementation to WadaMesh DISPLAY_CLASS
  and LVGL flush pipeline without inheriting the ST7789 240x320 constructor.
- Use a dedicated S3R2 environment; inherited Heltec expansion pin assignments
  conflict with LCD CS GPIO35 and touch CS GPIO6. Preserve the existing target.
- Add XPT2046 LVGL input adapter, raw sample filtering, five-point calibration,
  calibration persistence/recovery UI and rotation-aware coordinate mapping.
- Validate RAM, task scheduling, startup, USB companion, SX1262 and normal GUI.
- Do not flash existing WadaMesh ST7789 target for this external PiScreen.

No PR, no main-branch changes.
