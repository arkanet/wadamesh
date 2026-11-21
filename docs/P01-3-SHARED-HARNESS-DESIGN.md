# P01.3 — Shared harness engineering decision record (NO BUILD)

Target hardware: Heltec WiFi LoRa 32 V4.3 R2 and KMRTM35018-SPI V1.0 480x320 TFT with resistive SPI touch and independent microSD pads. Same harness required for WadaMesh, Meshtastic, DarkMesh.

## Hard reservations (cross-firmware)
- SX1262 GPIO8 CS, 9 SCK, 10 MOSI, 11 MISO, 12 RESET, 13 BUSY, 14 DIO1.
- FEM GPIO2,5,7,46.
- GNSS GPIO34 enable, 38/39 UART, 40 standby, 41 PPS, 42 reset.
- GPIO1 battery, 36 VEXT, 37 ADC; GPIO19/20 native USB.
- GPIO26 flash/PSRAM risk, GPIO45 ESP32-S3 strapping; GPIO0 boot strap.
- Existing ST7789 firmware pins (15/16/17/18/21/33) are NOT evidence that they are electrically available on the external header. External header provides GPIO16,21,33 but not 15/17/18.
- Optional functions already allocated in firmware: GPIO3/4 board I2C, GPIO6 buzzer, GPIO35 expansion IO, GPIO43/44 UART0/other touch reset, GPIO47/48 capacitive touch. These require explicit remapping/disable decisions before use.

## Shared bus signal plan (logical, not approved solder table)
| Physical MSP3520 signal | Shared net |
|---|---|
| SDI, SD_MOSI, T_DIN | SPI_MOSI |
| SCK, SD_SCK, T_CLK | SPI_SCK |
| SDO(MISO), SD_MISO, T_OUT | SPI_MISO, only after inactive tri-state validation |
| CS | LCD_CS |
| SD_CS | SD_CS (separate) |
| T_CS | TOUCH_CS (separate) |
| D/C | LCD_DC |
| RESET | LCD_RESET |
| LED | LCD_BACKLIGHT |
| T_IRQ | TOUCH_IRQ optional |
| VCC/GND | measured compatible power rail / common ground |

Minimum independently controlled signals: 9 GPIO (MOSI, MISO, SCK, 3x CS, D/C, RESET, BL), optionally IRQ = 10. RESET or BL may be managed by external circuitry after electrical validation.

## Candidate GPIO pool — conflicts not yet resolved
Potential external pins: 16,21,33,47,48,43,44,35,6,3,4. This is NOT an approved allocation: 47/48 are used by existing capacitive touch; 43/44 may be UART0, 35 expansion IO, 6 buzzer, 3/4 board I2C. To preserve all those functions simultaneously, a GPIO expander or alternate board-level arrangement may be needed.

Provisional **electrical design study only**, intentionally NOT solder-approved:
- SPI MOSI=33, SCK=47, MISO=48
- LCD DC=16, LCD CS=21
- SD CS=43, TOUCH CS=44
- LCD RESET=35, LCD BACKLIGHT=6
- TOUCH IRQ unconnected initially (polling)
This uses 43/44 (UART0), 35 (expansion IO), 6 (buzzer) and repurposes 47/48 (capacitive touch); each must be disabled/remapped in ALL THREE firmwares. It also assumes the exact R2 PCB and LCD/SD electrical compatibility, neither hardware-verified. This is a **candidate for comparative engineering**, not a wiring instruction.

## Firmware portability contract
- Keep the same signal names and numeric GPIOs in all targets.
- Separate display/touch/SD drivers from board-specific pin config.
- Serialize access to shared SPI; never share an SPI peripheral concurrently without explicit arbitration.
- LCD: ILI9488 portrait 320x480, strip-based drawing; resistive touch controller identity to be confirmed.
- SD: reuse WadaMesh existing PNG/PMTiles stack; assess Meshtastic/DarkMesh map feasibility independently.
- Maintain original ST7789 targets untouched.
- No flashing, compilation or physical soldering authorized by this document.

## Open gates
1. Verify exact R2 schematic, flash/PSRAM topology, GPIO output capability and serial debug alternatives.
2. Read marking on touch controller U2 and power interface; verify voltage at LCD and SD pads.
3. Verify MISO tri-state for ILI9488, touch controller and SD when CS high.
4. Confirm user accepts disabling/remapping buzzer, expansion IO and UART0 if provisional GPIO plan is selected; otherwise redesign with an expander.
5. Check DarkMesh V4.3 R2 support before asserting runtime compatibility.
