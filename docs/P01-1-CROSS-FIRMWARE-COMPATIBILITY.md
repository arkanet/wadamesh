# P01.1 — Single wiring harness, three firmware families

Requirement confirmed 2026-11-21: the Heltec V4.3 R2 + KMRTM35018-SPI 3.5-inch ILI9488 + microSD + touch wiring must remain unchanged when flashing WadaMesh/MeshCore, Meshtastic, or DarkMesh. Firmware may be adapted; physical rewiring should not be necessary.

## Repository discovery
- WadaMesh: arkanet/wadamesh, branch TFT-SPI-480×320
- DarkMesh firmware: arkanet/DarkMesh-Firmware (default branch 2.5.20-darkmesh_ghost)
- DarkMesh app: arkanet/DarkMesh
- No unambiguously named arkanet/firmware Meshtastic fork identified by repository-name search. Confirm exact Meshtastic firmware fork before editing.

## Compatibility matrix — not yet certified
| Check | WadaMesh | Meshtastic | DarkMesh |
|---|---|---|---|
| R2 LoRa/SX1262 GPIO ownership | Partially audited | Pending | Pending |
| GNSS and USB preserved | Pending hardware validation | Pending | Pending |
| External TFT SPI pin allocation | Pending | Pending | Pending |
| ILI9488 320x480 driver | To implement | To implement | To implement |
| Touch SPI controller identification | Pending | Pending | Pending |
| SD SPI and filesystem | Existing map stack, board adaptation pending | Pending | Pending |
| Offline maps | Existing PNG/PMTiles stack | Requires architecture assessment | Requires architecture assessment |
| Shared bus concurrency and watchdog | Pending | Pending | Pending |

## Hard constraints
- One immutable physical wiring map for all three firmware targets.
- No conflicting reuse of GPIOs reserved by any target's LoRa, GNSS, USB, power, PSRAM, boot straps or board functions.
- Display/SD/touch bus chip-select and SPI transaction ownership explicitly managed; never assume independent drivers arbitrate safely.
- TFT/SD failures must not interrupt LoRa RX/TX.
- Do not claim a common wiring is validated until all three target pin configurations and physical tests pass.
- Different software feature levels (e.g. maps) are permissible, but the wiring stays identical.

## Next actions
1. Identify exact Meshtastic firmware repository and target branch.
2. Audit DarkMesh-Firmware board pin definitions and Meshtastic pin definitions alongside WadaMesh.
3. Produce a single pin collision matrix and approved LCD/touch/SD ↔ Heltec GPIO solder table.
4. Implement independent firmware-specific board variants against the same wiring definition.
5. Build and flash-test each firmware separately, then exercise RF/GPS/USB/SD/touch simultaneously.
