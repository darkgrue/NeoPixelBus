# Design: Issue 921 — Configurable RMT Memory Block Size for BLE/WiFi Compatibility

**Date:** 2026-06-06  
**Branch:** `issue-921-ESP32-+-BLE/WiFi-mem_block_symbols`  
**Target:** PR against `Makuna/NeoPixelBus` master

---

## Problem

On ESP32 with an active BLE or WiFi connection, sparse LED patterns produce ghost pixels at index N+8 for every addressed pixel N. The defect is introduced during RMT transmission, not in the software color buffer.

**Root cause:** `config.mem_block_num = 1` in `NeoEsp32RmtMethod.h:Initialize()` allocates a single 64-symbol RMT memory block. With ping-pong DMA, each half-buffer holds 32 symbols. At WS2812x 800 Kbps (1.25 µs/symbol), the RMT refill ISR must be serviced within **~40 µs**. NimBLE and WiFi event handlers run at higher interrupt priority than the RMT refill ISR and routinely preempt for longer than 40 µs, leaving the data line idle past the ~300 µs LED latch threshold, which the LEDs interpret as an end-of-frame and produce false frame boundaries at every 8-pixel interval.

---

## Design

### Approach: User-overridable `NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS` macro

Add a single preprocessor macro that users can override at build time. The macro expresses the buffer size in **symbols** (the natural unit for both IDF v3/v4 and IDF v5 RMT APIs), ensuring consistent naming and user experience across both driver generations.

**Default:** 64 symbols = 1 memory block (backward-compatible — preserves current behavior for all existing users).

**BLE/WiFi fix:** Users add `-D NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS=512` to their build flags. This allocates 8 memory blocks; the half-buffer then holds 256 symbols × 1.25 µs = **320 µs**, which exceeds the 300 µs latch threshold and eliminates false latches.

### File changed

**`src/internal/methods/NeoEsp32RmtMethod.h`** (legacy IDF v3/v4 path, the path in `master`)

#### Change 1 — Add macro definition (near existing `NEOPIXELBUS_RMT_INT_FLAGS`)

```cpp
#ifndef NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS
#define NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS 64
#endif
```

Placement: immediately after the `NEOPIXELBUS_RMT_INT_FLAGS` block (~line 57), before the speed classes.

#### Change 2 — Replace hardcoded `mem_block_num = 1` in `Initialize()`

```cpp
// before
config.mem_block_num = 1;

// after
config.mem_block_num = (NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS + 63) / 64;
```

The integer division `(symbols + 63) / 64` rounds up to the nearest full block and is evaluated at compile time — zero runtime cost.

---

## Hardware Constraints

`mem_block_num > 1` consumes subsequent RMT channels' memory blocks on the original ESP32 (8 channels × 64 symbols = 512 total). The library already exposes explicit per-channel method types (`NeoEsp32Rmt0…`, `NeoEsp32Rmt1…`), so users who configure a large block size must ensure they do not instantiate strips on the channels whose memory blocks are consumed:

| `NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS` | `mem_block_num` | Blocks consumed (from ch N) | Half-buffer time @800Kbps |
|---|---|---|---|
| 64 (default) | 1 | 1 | ~40 µs |
| 128 | 2 | 2 | ~80 µs |
| 256 | 4 | 4 | ~160 µs |
| 384 | 6 | 6 | ~240 µs |
| 512 | 8 | 8 | ~320 µs ✓ (>300 µs) |

ESP32-S2, S3, and C3 have fewer channels/blocks, so users should be aware of variant-specific limits. For safety, the PR documentation will recommend 512 for original ESP32 + BLE/WiFi; smaller values provide partial improvement.

---

## Scope

**In scope:**
- Add `NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS` macro to `NeoEsp32RmtMethod.h`
- Replace `config.mem_block_num = 1` with derived value
- Update PR description with usage guidance

**Out of scope:**
- Fixing the same issue in `ESP-IDF-RMT-driver-migration` branch (separate WIP branch with its own PR lifecycle)
- Changing interrupt priority via `NEOPIXELBUS_RMT_INT_FLAGS` (different approach; Approach C)
- Any changes to I2S, LCD, or other method files

---

## Testing

This is a hardware-timing bug. Functional verification requires physical hardware:
1. ESP32 + WS2812x strip, NimBLE server advertising with a connected client
2. Set single pixel to red, all others black, call `Show()` in a loop
3. Without fix: ghost pixel at index 8 visible
4. With `-D NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS=512`: only pixel 0 lights

Compile-time check (no hardware needed): the library must build cleanly for all ESP32 variants (ESP32, S2, S3, C3) with and without the override macro set.

---

## PR Target

- From: `origin/issue-921-ESP32-+-BLE/WiFi-mem_block_symbols`
- To: `upstream/master` (Makuna/NeoPixelBus)
