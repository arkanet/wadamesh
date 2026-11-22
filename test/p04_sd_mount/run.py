#!/usr/bin/env python3
"""Exercise the production P04 mount code against a failing/removable fake SD.

No copied mount algorithm: extract the functions from UITask.cpp, compile their
real P04 preprocessor path, and run it with deterministic Arduino/SD doubles.
"""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/ui-touch/UITask.cpp").read_text()


def function(signature):
    start = SOURCE.index(signature)
    brace = SOURCE.index("{", start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (SOURCE[end] == "{") - (SOURCE[end] == "}")
        end += 1
    return SOURCE[start:end]


PREAMBLE = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#define P04_ILI9488_SD 1
#define CAP_SD 1
#define PIN_SD_CS 43
#define CARD_NONE 0
struct SPIClass {};
static SPIClass bus;
static SPIClass* shared_bus = &bus;
static SPIClass* sdSharedSPI() { return shared_bus; }
static uint32_t now_ms;
static uint32_t millis() { return now_ms; }
static bool busy, s_sd_mounted;
static uint64_t s_sd_size;
static unsigned long s_sd_retry_after_ms;
static uint32_t g_sd_operating_hz;
static int diag_begins, io_marks;
static uint32_t diag_hz;
static bool diag_mounted;
static void sdMountDiagBegin() { ++diag_begins; }
static void sdMountDiagAttempt(uint32_t hz, bool, bool) { diag_hz = hz; }
static void sdMountDiagSetMounted(bool mounted, uint32_t hz) {
  diag_mounted = mounted;
  diag_hz = hz;
}
static void markSdIo() { ++io_marks; }
static bool sdRuntimeLifecycleBusy() { return busy; }
struct FakeSD {
  int begins = 0, ends = 0, type = CARD_NONE;
  bool begin_ok = false, card_ready = false;
  uint32_t hz = 0;
  bool begin(int cs, SPIClass& spi, uint32_t clock, const char*, int files) {
    assert(cs == PIN_SD_CS && &spi == &bus && files == 6);
    ++begins;
    hz = clock;
    type = begin_ok && card_ready ? 1 : CARD_NONE;
    return begin_ok;
  }
  int cardType() { return type; }
  uint64_t cardSize() { return 8ULL * 1024 * 1024 * 1024; }
  void end() { ++ends; type = CARD_NONE; }
  void mkdir(const char*) { assert(false && "map-only recovery must not migrate history"); }
} SD;
static FakeSD internal_storage;
static FakeSD* s_ui_data_fs;
static FakeSD* s_tile_fs;
static bool s_tiles_from_sd, s_tiles_fs_ready = true;
static bool tileFetchWorkerActive() { return false; }
static void uiDataFsReady() { s_ui_data_fs = &internal_storage; }
static void uiDataEnsureDirs() { assert(false); }
static bool sleeping;
namespace touchSleep { static bool isSleeping() { return sleeping; } }
static void* s_fm_list;
static uint32_t s_sd_fail_note_ms;
static bool s_sd_format_pending;
static void tileBackendSwapTryBegin() {}
static void tileBackendSwapFinish() {}
static bool sdProbeAlive() { return SD.card_ready; }
static int storage_changes;
static void mapNoteStorageChanged() { ++storage_changes; }
#define TR(text) text
struct FakeTask {
  void showAlert(const char*, int) {}
  void flushHistorySoon() { assert(false); }
};
static struct { FakeTask* task = nullptr; } g_lv;
static void reset() {
  SD = FakeSD{};
  now_ms = 100;
  busy = s_sd_mounted = diag_mounted = false;
  s_sd_size = s_sd_retry_after_ms = g_sd_operating_hz = 0;
  diag_begins = io_marks = 0;
  diag_hz = 0;
  shared_bus = &bus;
  sleeping = false;
  s_fm_list = nullptr;
  s_sd_fail_note_ms = 0;
  s_sd_format_pending = false;
  storage_changes = 0;
  s_ui_data_fs = s_tile_fs = nullptr;
  s_tiles_from_sd = false;
  s_tiles_fs_ready = true;
}
'''

TESTS = r'''
int main() {
  reset();
  busy = true;
  assert(!fmSdTryMount());
  assert(SD.begins == 0 && SD.ends == 0 && diag_begins == 0);

  reset();
  shared_bus = nullptr;
  assert(!fmSdTryMount());
  assert(SD.begins == 0 && SD.ends == 0);

  reset();
  assert(!fmSdTryMount()); // absent card: one attempt, cleanup, ten-second backoff
  assert(SD.begins == 1 && SD.ends == 1 && !diag_mounted);
  now_ms += 9999;
  assert(!fmSdTryMount() && SD.begins == 1);
  ++now_ms;
  SD.begin_ok = SD.card_ready = true;
  assert(fmSdTryMount() && SD.begins == 2 && SD.hz == P04_SD_SPI_HZ);
  assert(s_sd_size == 8ULL * 1024 * 1024 * 1024);
  assert(diag_mounted && diag_hz == P04_SD_SPI_HZ);
  assert(g_sd_operating_hz == P04_SD_SPI_HZ && !s_sd_retry_after_ms);
  busy = true; // adopting a live mount does not tear down workers' open handles
  assert(fmSdTryMount() && SD.begins == 2 && SD.ends == 1);
  busy = false;
  fmSdUnmount();
  assert(!s_sd_mounted && !s_sd_size && !diag_mounted && SD.ends == 2);

  reset();
  SD.begin_ok = true; // begin success alone is not a ready card
  assert(!fmSdTryMount() && SD.ends == 1 && !s_sd_mounted);
  s_sd_retry_after_ms = 0; // explicit retry bypasses backoff
  assert(!fmSdTryMount() && SD.begins == 2);

  reset();
  now_ms = UINT32_MAX - 4999; // deadline crosses wrap
  assert(!fmSdTryMount());
  now_ms = UINT32_MAX - 1;
  assert(!fmSdTryMount() && SD.begins == 1);
  now_ms = 4999;
  assert(!fmSdTryMount() && SD.begins == 1);
  now_ms = 5000;
  assert(!fmSdTryMount() && SD.begins == 2);

  reset();
  now_ms = UINT32_MAX - 9999; // zero deadline must not disable backoff
  assert(!fmSdTryMount());
  assert(s_sd_retry_after_ms != 0);
  now_ms = UINT32_MAX;
  assert(!fmSdTryMount() && SD.begins == 1);
  now_ms = 1;
  assert(!fmSdTryMount() && SD.begins == 2);

  reset();
  now_ms = 100000;
  sdHealthTick(); // background recovery uses the same configured clock
  assert(SD.begins == 1 && SD.hz == P04_SD_SPI_HZ && !s_sd_mounted);
  now_ms = 129999;
  sdHealthTick();
  assert(SD.begins == 1);
  now_ms = 130000;
  busy = true;
  sdHealthTick();
  assert(SD.begins == 1); // lifecycle busy also blocks background mounts
  now_ms = 160000;
  busy = false;
  SD.begin_ok = SD.card_ready = true;
  sdHealthTick();
  assert(SD.begins == 2 && s_sd_mounted && storage_changes == 1);
  assert(diag_hz == P04_SD_SPI_HZ && g_sd_operating_hz == P04_SD_SPI_HZ);
  puts("P04 SD mount: PASS");
}
'''

functions = "\n".join(function(signature) for signature in (
    "static bool sdMountRetryPending(uint32_t now)",
    "static bool sdAdoptLiveMount()",
    "static bool sdBeginTracked(SPIClass* spi",
    "static bool fmSdTryMount() {",
    "static void fmSdUnmount() {",
    "static void sdHealthTick() {",
))

with tempfile.TemporaryDirectory(prefix="wada-p04-") as tmp:
    source = Path(tmp) / "mount.cpp"
    source.write_text(PREAMBLE + functions + TESTS)
    for hz in (4_000_000, 1_000_000):
        binary = Path(tmp) / f"mount-{hz}"
        subprocess.run([os.environ.get("CXX", "g++"), "-std=c++17", "-Wall",
                        "-Wextra", "-Werror", f"-DP04_SD_SPI_HZ={hz}",
                        str(source), "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
