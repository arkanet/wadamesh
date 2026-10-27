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

## Candidate direct wiring, *pending header continuity and 3.3 V levels*

| Module RPi 40-pin physical | Purpose | Heltec MCU GPIO candidate | Notes |
|---|---|---|---|
| 23 | SPI SCLK LCD+touch | 47 | *External display-only* SPI bus |
| 19 | SPI MOSI LCD+touch | 33 | Native ESP32-S3R2 PSRAM QSPI assumed; verify pin availability on S3R2 |
| 21 | SPI MISO touch | 48 | LCD does not require MISO |
| 24 | LCD_CS | 15 | Separate from LoRa NSS=8 |
| 18 | LCD DC/RS | 16 | GPIO output |
| 22 | LCD RESET | 41 | **GNSS PPS repurposed**; requires GNSS disabled |
| 26 | TOUCH_CS | 6 | Disable WadaMesh buzzer on GPIO6 |
| 11 | TOUCH IRQ | 35 | Disable built-in LED use; *optional*: poll XPT2046 with IRQ disconnected |
| 6 (also 9/14/20/25) | GND | GND | Common ground required |
| 2 or 4 | 5 V supply | **externally current-rated regulated 5 V** | Verify module 5V rail; avoid USB backfeed into Heltec |
| 1/17 | 3.3 V rail pins | **do not connect before checking actual module schematic** | LCD board may derive 3.3V from 5V; avoid rail contention |

The above is a **candidate**, *not an approved harness*. Do not connect until module GPIO header mapping, V4.3 PCB revision and electrical levels are checked. Pin # numbering on Raspberry is **physical pin position**, not BCM GPIO.

LCDwiki MPI3501 (candidate identification, not uniquely proven) states 5V ~0.13 A and 3.3V signals: https://lcdwiki.com/3.5inch_RPi_Display. Module photograph and PiScreen test confirm board family, not internal regulator topology or input tolerance. Use meter/supply current limiting before wiring.

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
