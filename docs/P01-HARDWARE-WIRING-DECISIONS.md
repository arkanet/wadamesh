# P01 — Hardware wiring decisions

Confirmed by hardware owner (2026-11-21): MSP3520 microSD pads are accessible; soldered wires and direct wiring are acceptable, with preference for low-profile routing behind the display.

## Decisions
- Use soldered flexible wires and strain relief rather than bulky rear connectors.
- TFT ILI9488 runs portrait (320x480).
- SD_SCK, SD_MISO, SD_MOSI, SD_CS pads may be used directly.
- Prefer a shared *external* SPI bus for LCD and SD with separate chip selects, subject to electrical validation. The radio SPI remains untouched.
- A shared SD/LCD SPI bus can require only one extra CS GPIO beyond the TFT signals, but check LCD SDO tri-state and SD voltage compatibility; do not assume this before measurement.
- Use short wires, common ground, verified 3.3V logic, suitable backlight supply and local decoupling.

## Pending before soldering
1. Audit Heltec V4.3 R2 pinout and existing WadaMesh assignments.
2. Confirm MSP3520 PCB revision, LCD SD interface and 3.3V/5V regulator arrangement.
3. Produce explicit MSP3520 pin/pad ↔ Heltec GPIO table, marking reserved signals.
4. Confirm SPI chip-select idle levels and bus arbitration.
5. Execute TFT-only, SD-only and concurrent LoRa stress tests.

Do not treat this document as an approved wiring diagram.
