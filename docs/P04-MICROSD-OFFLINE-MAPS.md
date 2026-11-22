# P04 — External SPI microSD with offline maps, Heltec V4.3 R2

Status: **compile-only, unvalidated hardware**. This is not an electrical
pin approval, a flashed/field-tested image, nor a verified SD+touch+radio
concurrency implementation.

## Build

```sh
pio run -e heltec_v4_r2_ili9488_p04
```

P02 (LCD), P03 (touch) and the legacy ST7789 environment stay separate.

## Scope / reuse

* Reuses WadaMesh's **existing** map library: JPEG tile folders and
  uncompressed PNG **PMTiles v3** archives. It does not create a new map
  decoder or second storage schema.
* Uses `CAP_SD=1` and `CAP_MAP_PMTILES=1` only for this optional board
  variant. The built-in map source selector can browse SD `/maps` archives,
  and `/tiles/{z}/{x}/{y}.jpg` folder packs.
* Uses Arduino `SD` on the already-started **FSPI/SPI2** bus at **4 MHz**;
  the same host drives ILI9488 via LovyanGFX and provisional XPT2046.
  Every peripheral has its **own CS**. SX1262 continues on HSPI/SPI3.
* Mount-on-demand via the existing `fmSdTryMount()`; a missing/unreadable
  FAT16/FAT32 card is non-fatal and guarded with a retry backoff.
* The file-manager card row offers **mount/retry**, not **format**;
  the P04 compile path deliberately excludes the in-device FAT32 formatter.
* `src/main.cpp` is *not* changed to adopt the SD as identity/prefs/history
  storage. **No automatic data-store migration on first boot.** SPIFFS remains
  the default for device identity.
* Application-level map read paths reuse the existing file handling. This
  is **not** a block-level read-only mount: the legacy file manager and online
  tile-cache paths may still write to an SD filesystem if deliberately used.
  Do not test with your only copy of important data.

## Proposed external wiring (NOT approved for soldering)

| Panel/pad label | GPIO (Heltec V4.3 R2) | Bus |
| --- | ---: | --- |
| SD_SCK | 47 | SPI2 SCK shared with LCD SCK, touch T_CLK |
| SD_MOSI | 33 | SPI2 MOSI shared with LCD SDI, touch T_DIN |
| SD_MISO | 48 | SPI2 MISO shared with LCD SDO, touch T_OUT |
| SD_CS | 43 | independent SD chip select |
| LCD CS | 21 | independent LCD chip select |
| T_CS | 44 | independent resistive-touch chip select |
| LCD D/C | 16 | panel data/command |
| LCD RESET | 35 | tentative reset GPIO |
| LCD LED | 6 | tentative backlight GPIO |
| VCC/GND | not determined | verify physical panel board rail first |

No GPIO8–14 reallocation: they belong to SX1262. GPIO43/44 may conflict
with UART0, GPIO35 with expansion IO and GPIO6 with the buzzer. Neither
GPIO47/48 reassignment nor the module's **MISO tri-state** and supply
voltage have been measured. Power-up simultaneous chip select states and
bus sharing are **unproven**; do not connect or flash from this table alone.

## Offline map data (existing WadaMesh engine)

The map already accepts directories of raster tiles and supported PMTiles
archives. Use a FAT32 card with /maps for map packs. For direct folder tile
fallback the existing `/tiles/{z}/{x}/{y}.jpg` convention applies.
PNG PMTiles v3 support is for **PNG tile data without tile compression**,
not JPEG PMTiles or vector PBF. Names and region folder conventions must
follow the current WadaMesh map picker. Never bulk-scrape the public OSM
tile server; respect attribution and dataset licensing.

## Hardware acceptance gate

1. Photograph and identify the exact LCD and touch ICs and regulator/level
   shifter. Confirm module requires/accepts actual VCC rail and 3.3V logic.
2. Verify V4.3 R2 GPIO availability, boot state and any UART conflicts.
3. On an electrically verified harness, verify idle CS HIGH and all non-
   selected SDO/T_OUT/SD_MISO outputs high-impedance.
4. Test LCD + touch without an SD; ensure the radio remains functional.
5. Test SD mount at 4 MHz, directory listing and read of a known JPEG tile
   and PMTiles header, then remove/reinsert card. Test graceful no-card boot.
6. Exercise map zoom/pan and packet RX/TX concurrently and record missed RX,
   longest UI stall, watchdog resets, SPI errors and memory high-water marks.
7. Verify power budget including LCD backlight and SD transient current.
8. Only then approve hardware flashing and adapt validated wiring to the
   Meshtastic and DarkMesh builds.

Flash budget is tight: P03 used **3,886,381 / 4,063,232 bytes** (95.6%).
P04 build **passed** GitHub Actions compilation in run #45026910121:
**3,961,105 / 4,063,232 bytes** (97.5% flash), **123,552 bytes** reported
RAM (5.9% of 2,097,152). Available application flash is just
**102,127 bytes**. This is a compile/link result, not a field measurement.
Further UI/SD additions must be size-budgeted. Prefer removing duplicated
features over increasing partition sizes or silently disabling safety or
map functionality. P03/P02/ST7789 regression jobs execute after P04.
