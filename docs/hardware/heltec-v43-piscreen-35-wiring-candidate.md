# Heltec V4.3 S3R2 ↔ PiScreen 3.5-inch: provisional wiring map (NO POWER YET)

Status: **pin planning only**. No confirmed hardware connection, no software build, no PR.

## Sources of truth
- User's tested Raspberry `piscreen` overlay (ILI9486-compatible at 16 MHz, touch ADS7846 at 2 MHz).
- Heltec schematic: https://resource.heltec.cn/download/WiFi_LoRa_32_V4/Schematic/HTIT-WB32LAF_V4.3.pdf (the **V4.3 S3R2** schematic, not V4-R8).
- WadaMesh `platformio.ini` base `heltec_v4_tft_companion_radio_usb_tcp_touch` contains ST7789/CHSC6x pins for an *expansion display*; those are not the standalone board's peripheral layout.
- User's DarkMesh boot log confirms 2 MB PSRAM, LoRa SPI (9,11,10,8), OLED on I2C 17/18.

## Reserved, no-touch MCU pins
- RF: GPIO8–14 and GPIO2/5/7 plus RF PA control GPIO46 (firmware-specific).
- Integrated OLED: GPIO17/18 I2C, GPIO21 reset; power/user system: GPIO0, 1, 34, 36, 37; board I2C GPIO3/4.
- GPIO19/20 USB; GPIO26–32 flash interface; GPIO45 strapping.
- GNSS-related GPIO38–42 are exposed but may be used by an optional external GNSS; the map below sacrifices GPIO41 (PPS), only if **no external GNSS**.
- GPIO35 is also LED; GPIO6 is an optional buzzer in WadaMesh; they must be disabled for the external TFT profile.
- JTAG/UART usage for chosen pins must be checked at runtime.

## Corrected physical-header wiring proposal — S3R2 only

**The earlier GPIO15/GPIO16 suggestion was invalid for plug-in Dupont wiring**: those pins are not on the standard side headers J2/J3; they are listed as additional pins / 32 kHz oscillator connections in Heltec's documentation. Do **not** use the earlier table. Source: Heltec V4 datasheet, J2/J3 definitions, cross-checked with current V4.3 pinout.

The connector-position values previously written as J2/J3 numbers were not grounded against the user's physical board; they are **removed**. On the photographed PCB the top/bottom rows are the reliable physical reference; verify silkscreen alignment before plugging wires. The TFT pins refer to **physical Raspberry 40-pin numbering**, **not BCM GPIO numbers**.

| RPi LCD pin | Signal | Heltec S3R2 GPIO | Heltec side-header position | Notes |
|---|---|---|---|---|
| 23 | shared LCD/Touch SCLK | 47 | bottom header, GPIO47 | Independent SPI, preserve SX1262 bus |
| 19 | shared LCD/Touch MOSI | 33 | bottom header, GPIO33 | S3R2 QSPI memory variant only |
| 21 | touch MISO | 48 | bottom header, GPIO48 | LCD is write-only |
| 24 | LCD CS | 35 | bottom header, GPIO35 | GPIO35 on-board LED may also flash; isolate in code |
| 18 | LCD D/C | 40 | top header, GPIO40 | GNSS_WAKE function unavailable if reassigned |
| 22 | LCD RESET | 41 | top header, GPIO41 | GNSS_PPS / JTAG TDI repurposed |
| 26 | touch CS | 6 | top header, GPIO6 | Disable WadaMesh optional buzzer |
| 11 | touch IRQ | *unconnected initially* | — | Polling mode first; may add GPIO38/J3 pin 11 only after testing |
| 6 | GND | GND | GND pad, verify with continuity | Common ground |
| 2 or 4 | 5V | *no connection until measured* | — | External regulated 5V; verify module levels and supply scheme |
| 1 and 17 | 3V3 | *no connection until measured* | — | Never join multiple power outputs without checking module design |

The **LCD backlight cannot be controlled over a dedicated signal** from the published 40-pin PiScreen interface; GPIO22 on the Raspberry overlay is a `led-gpios` output and is *not* established as a physical LCD header BL input. Begin with the screen's verified power wiring and default backlight state.

Important possible resource clashes: GPIO35 is connected to the on-board LED, GPIO40/41 to GNSS/JTAG, GPIO6 to the optional buzzer. These must be explicitly disabled/remapped for the new build. Physical header pin access **does not imply unused silicon resource**.

References:
- Heltec V4.3 product/pinout: https://heltec.org/project/wifi-lora-32-v4/
- V4 header map: https://s.heltec.cn/download/WiFi_LoRa_32_V4/datasheet/WiFi_LoRa_32_V4.2.0.pdf
- Screen *candidate* module interface: https://lcdwiki.com/3.5inch_RPi_Display

**Status: candidate pending power validation, no wiring approved.** The PiScreen module must be fully detached from the Raspberry before connecting a separate host; do not plug it onto the Heltec pins directly.

## Software changes strictly isolated in new environment
- Existing base WadaMesh environment has ST7789 240×320 LCD on GPIO17/33/15/16/18/21 and CHSC6x *I2C* on 47/48. **DO NOT FLASH this existing environment onto the external LCD**.
- New environment should inherit common MeshCore/Companion settings but override display/touch paths. Avoid cumulative compiler flags for ST7789 and ILI9486; consider refactoring the board-display flags into shared/common + exclusive subsets.
- XPT2046 touch uses SPI, needs driver independent of `HeltecV4CapTouch`/CHSC6x and calibration in landscape.
- Driver must port the known working PiScreen custom `init`, 16-bit register-write convention, and set a cautious 16 MHz SPI ceiling.
- WadaMesh UI has fixed 240×320 assumptions (e.g. `HeltecV4CapTouch.cpp` uses W=240, H=320); adapt to 480×320 without global regression.
- Physical backlight power/control is unverified; **do not** drive 5V LED supply directly from MCU GPIO.

## Stop/go gates
1. Validate S3R2 GPIO33/47/48 usable as external bus *on this physical board*; inspect physical J headers before wiring.
2. Validate 5V and 3.3V rail interface of screen with multimeter; arrange common ground, no dual 5V host backfeed.
3. Disconnect Raspberry completely and power off all hardware before trial wiring.
4. Build a stand-alone ESP32-S3 test (color bars + raw touch, radio pins untouched).
5. Only then create WadaMesh external display profile, compile and test MeshCore radio.

Keep PRs closed; this document is planning, not a flashed or field-tested schematic.

## Photo verification (user-supplied HTIT-WB32LAF V4.3 backside)

The photographed **top row** is silk-labelled (left-to-right, image orientation): 7, 6, 5, 4, 3, 2, 1, 38, 39, 40, 41, 42, 45, 46, 37, [power pads], GND. The **bottom row** is labelled 19, 20, 21, 26, 48, 47, 33, 34, 35, 0, RST, 43, 44, Ve, Ve, 5V, GND. Some small labels can be obscured; inspect on the actual PCB and confirm GND with a meter.

The candidate signals are accessible, but **SCLK=GPIO47/MOSI=GPIO33/MISO=GPIO48, CS=GPIO35, DC=GPIO40, RESET=GPIO41, TOUCH_CS=GPIO6 remain unpowered/unwired** pending voltage/current and electrical validation. GPIO35 is on-board LED, GPIO40/41 assigned to GPS-related uses in firmware, GPIO6 optional buzzer. No touch IRQ in phase 1. Do not confuse physical Raspberry header positions with Heltec silkscreen GPIOs.
