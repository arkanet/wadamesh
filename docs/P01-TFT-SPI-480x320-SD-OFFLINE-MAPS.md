# P01 — TFT SPI 480x320 + microSD offline maps

Target: Heltec WiFi LoRa 32 V4.3 **R2** (ESP32-S3, 2 MB PSRAM), LCDWiki MSP3520 ILI9488 3.5-inch, 480x320 rotated to portrait 320x480.

## Goals
1. Preserve existing LoRa radio and firmware operation.
2. Bring up display with a nonblocking renderer, no full-screen framebuffer requirement.
3. Mount microSD over SPI, verify read-only access and show a test tile.
4. Display an offline raster map centered on a node GPS position, with zoom/pan and node markers.
5. Degrade gracefully when SD is missing/corrupt; never block radio processing.

## Electrical constraints / verification gates
- Heltec R2 is NOT interchangeable with R8: audit the exact R2 schematic, firmware pin definitions, PSRAM/flash reserved pins, GNSS and RF FEM controls before assigning GPIO.
- LoRa uses GPIO8..14 (NSS=8, SCK=9, MOSI=10, MISO=11, RESET=12, BUSY=13, DIO1=14); do not repurpose them.
- GPIO19/20 are native USB; GPIO26 is PSRAM-related; GPIO1 battery sensing; GPIO2/5/7 FEM control. Avoid without a documented design change.
- LCD header: VCC, GND, CS, RESET, DC/RS, SDI(MOSI), SCK, LED, SDO(MISO); optional resistive touch: T_CLK, T_CS, T_DIN, T_DO, T_IRQ.
- microSD contacts are separate PCB pads SD_SCK, SD_MISO, SD_MOSI, SD_CS. **The LCD's 9-pin header alone does not connect the SD card.** Inspect actual PCB revision, continuity and voltage before wiring.
- Never feed 5V logic into ESP32-S3. Size supply for TFT backlight and SD transient current.
- Prefer independent external SPI bus for display + SD (shared SCK/MOSI/MISO with independent CS, serialized transactions); LoRa stays on its board-specific bus. SD SPI frequency begins conservatively (e.g. 4 MHz), increase only after tests.
- Pin assignment remains TBD pending full existing wiring audit and PCB photo/continuity. No unverified GPIO mapping should be committed as a production pinout.

## Map architecture
- Phase 1: pre-generated raster JPEG tiles in an indexed folder tree /maps/{region}/{z}/{x}/{y}.jpg; optional PNG for overlay, no SQLite dependency.
- Use standard Web Mercator slippy-map z/x/y indexing, longitude wrapping, latitude clamped to +/-85.05112878 degrees.
- Prefer 128x128 tiles for constrained decoded buffers; 256x256 accepted with strip/scanline decoder. Avoid full RGB framebuffer.
- Draw only dirty rectangles and visible tiles; cap cache by PSRAM budget, reserve RAM for mesh stack. Do not hold FS mutex during radio callbacks.
- GPS node positions are drawn as overlays, not baked into tiles.
- Optional later MBTiles adapter with SQLite after measuring RAM, CPU and seek latency.
- Map assets must have compatible redistribution terms and attribution (e.g. OpenStreetMap contributors); do not bulk-download from OSM's public tile server.

## Modules proposed
- display/ili9488_port.{h,cpp}: init, rotation, DMA/partial drawing, backlight.
- storage/sd_spi.{h,cpp}: mount, status, safe reads, absent-card handling.
- maps/tile_store.{h,cpp}: path lookup, tile cache and fallbacks.
- maps/mercator.{h,cpp}: lat/lon to pixel and viewport transforms.
- ui/map_screen.{h,cpp}: viewport, node overlay and refresh budget.
- tests/: coordinate transforms, path traversal rejection, absent SD, invalid tile, zoom bounds, radio responsiveness.

## Acceptance criteria
- Portrait 320x480 verified on hardware.
- SD mount, directory scan, read test and card removal recovery.
- Offline map loads with Wi-Fi/BLE disabled.
- Node marker alignment at several latitudes and zoom levels.
- No LoRa RX/TX regression, no watchdog resets during scrolling and SD failures.
- Memory high-water mark and rendering latency documented.

## First execution steps
1. Audit current fork's firmware structure and any prior TFT branch; choose minimal integration point.
2. Confirm MSP3520 PCB SD pads and safe R2 GPIO allocation; record TFT-pin <-> Heltec GPIO table.
3. Implement independent SD smoke test, then LCD test, then combined SPI arbitration test.
4. Implement Mercator coordinate tests and a static pre-generated map package.
5. Integrate renderer into existing mesh UI without blocking the radio.

References:
- https://www.lcdwiki.com/3.5inch_SPI_Module_ILI9488_SKU:MSP3520
- https://www.lcdwiki.com/res/MSP3520/3.5inch_SPI_Module_MSP3520_User_Manual_EN.pdf
- https://heltec.org/project/wifi-lora-32-v4/
