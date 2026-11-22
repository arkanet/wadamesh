#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""P02-only source compatibility adjustment for the large vendored LVGL UI.

The upstream UITask.cpp directly chooses ST7789LCDDisplay in the fallback
case and hardcodes 240x320. This transform is deliberately narrow, idempotent
and strict: a mismatched upstream source fails the build rather than producing
a silently incorrect LCD image.

TEMPORARY P02 BRIDGE: upstream this small set of conditional guards into
UITask.cpp when the 4 MB UI translation unit can be committed directly.
"""
from pathlib import Path

Import("env")  # noqa: F821  -- PlatformIO/SCons runtime

if env.get("PIOENV") not in (
    "heltec_v4_r2_ili9488_p02",
    "heltec_v4_r2_ili9488_p03",
    "heltec_v4_r2_ili9488_p04",
):
    raise RuntimeError("External ILI9488 UITask patch executed in an unrelated build")

source = Path(env.subst("$PROJECT_DIR")) / "src/ui-touch/UITask.cpp"
s = source.read_text(encoding="utf-8")
changes = [
    (
        '  #else\n    #include <helpers/ui/ST7789LCDDisplay.h>\n  #endif\n  #include <helpers/AdvertDataHelpers.h>',
        '  #elif defined(HELTEC_LORA_V4_R2_ILI9488)\n'
        '    #include <ILI9488LCDDisplay.h>\n'
        '  #else\n    #include <helpers/ui/ST7789LCDDisplay.h>\n'
        '  #endif\n  #include <helpers/AdvertDataHelpers.h>',
    ),
    (
        '  #else\n    extern ST7789LCDDisplay display;\n  #endif\n#endif\n\nconstexpr unsigned long UI_REFRESH_MS',
        '  #elif defined(HELTEC_LORA_V4_R2_ILI9488)\n'
        '    extern ILI9488LCDDisplay display;\n'
        '  #else\n    extern ST7789LCDDisplay display;\n'
        '  #endif\n#endif\n\nconstexpr unsigned long UI_REFRESH_MS',
    ),
    (
        '#if defined(HELTEC_LORA_V4_R8)\n// ---- V4-R8 panel sleep (anti burn-in)',
        '#if defined(HELTEC_LORA_V4_R8) || defined(HELTEC_LORA_V4_R2_ILI9488)\n'
        '// ---- LovyanGFX panel sleep (anti burn-in)',
    ),
    (
        '#elif defined(HAS_WIO_TRACKER_L2)\n'
        '  // Native 320x240 landscape (UI rotation 3), or 240x320 in the portraits (#698).\n'
        '  g_lv.disp_drv.hor_res',
        '#elif defined(HELTEC_LORA_V4_R2_ILI9488)\n'
        '  // External ILI9488: native portrait 320x480; landscape 480x320.\n'
        '  g_lv.disp_drv.hor_res = s_remote_mode ? (s_remote_landscape ? 400 : 240) : (ui_landscape ? 480 : 320);\n'
        '  g_lv.disp_drv.ver_res = s_remote_mode ? (s_remote_landscape ? 240 : 400) : (ui_landscape ? 320 : 480);\n'
        '#elif defined(HAS_WIO_TRACKER_L2)\n'
        '  // Native 320x240 landscape (UI rotation 3), or 240x320 in the portraits (#698).\n'
        '  g_lv.disp_drv.hor_res',
    ),
]
# P04 adds ONLY its own selective SD paths to the existing, already-tested UI.
# Do NOT enable the original R8 board-specific power/backlight/boot behavior
# and do NOT expose the destructive in-app FAT32 formatter.
if env.get("PIOENV") == "heltec_v4_r2_ili9488_p04":
    changes.extend([
        (
            '#elif defined(HAS_TDECK_GT911) || defined(HAS_TDECK_PRO) || defined(HAS_THINKNODE_M9) || defined(HELTEC_LORA_V4_R8)\n  #include <SD.h>',
            '#elif defined(HAS_TDECK_GT911) || defined(HAS_TDECK_PRO) || defined(HAS_THINKNODE_M9) || defined(HELTEC_LORA_V4_R8) || defined(P04_ILI9488_SD)\n  #include <SD.h>',
        ),
        (
            '#elif defined(HELTEC_LORA_V4_R8)\nstatic inline SPIClass* sdSharedSPI() { return heltecV4R8SharedSPI(); }   // V4-R8: micro-SD shares the TFT FSPI bus (CS=3)',
            '#elif defined(HELTEC_LORA_V4_R8)\nstatic inline SPIClass* sdSharedSPI() { return heltecV4R8SharedSPI(); }   // V4-R8: micro-SD shares the TFT FSPI bus (CS=3)\n'
            '#elif defined(P04_ILI9488_SD)\n'
            'static inline SPIClass* sdSharedSPI() { return heltecV4R2SharedSPI(); } // R2 LCD+touch+SD SPI2; LoRa SPI3',
        ),
        (
            'const bool mounted = sdPagerBeginWithPowerRecovery(spi, &begin_ok);\n  const uint32_t mounted_hz = 4000000;\n#else',
            'const bool mounted = sdPagerBeginWithPowerRecovery(spi, &begin_ok);\n  const uint32_t mounted_hz = 4000000;\n'
            '#elif defined(P04_ILI9488_SD)\n'
            '  // Single conservative 4 MHz attempt; never reset shared TFT/touch bus.\n'
            '  const uint32_t mounted_hz = 4000000;\n'
            '  const bool mounted = sdBeginTracked(spi, mounted_hz);\n#else',
        ),
        (
            '#if defined(TLORA_PAGER) || defined(HAS_CROWPANEL_35)\n// These ports offer only a non-destructive mount retry.',
            '#if defined(TLORA_PAGER) || defined(HAS_CROWPANEL_35) || defined(P04_ILI9488_SD)\n'
            '// These ports offer only a non-destructive mount retry.',
        ),
        (
            '#elif defined(HAS_CROWPANEL_35)\n  if ((s_sd_mounted || millis() >= s_sd_retry_after_ms) && fmSdTryMount()) {',
            '#elif defined(HAS_CROWPANEL_35) || defined(P04_ILI9488_SD)\n'
            '  if ((s_sd_mounted || millis() >= s_sd_retry_after_ms) && fmSdTryMount()) {',
        ),
    ])

count = 0
for previous, replacement in changes:
    if replacement in s:
        continue  # already applied in current checkout
    matches = s.count(previous)
    if matches != 1:
        raise RuntimeError(
            f"P02 UITask compatibility source mismatch: expected 1 occurrence, found {matches}"
        )
    s = s.replace(previous, replacement, 1)
    count += 1
if count:
    source.write_text(s, encoding="utf-8")
print(f"[P02] UITask ILI9488 compatibility: {count} substitutions, portrait 320x480")
