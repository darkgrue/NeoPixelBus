# Issue 921: Configurable RMT Memory Block Size Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the ESP32 RMT memory block size user-overridable via `NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS` so projects with active BLE or WiFi can increase the RMT ping-pong half-buffer duration beyond the 300 µs LED latch threshold.

**Architecture:** Two preprocessor changes to a single header file. A `#ifndef` guard adds a user-overridable macro defaulting to 64 (backward compatible); the existing `config.mem_block_num = 1` is replaced with a compile-time integer expression derived from that macro. No new files, no runtime cost.

**Tech Stack:** C++ preprocessor, ESP-IDF legacy RMT driver (IDF v3/v4), Arduino library

---

## File Map

| Action | Path | What changes |
|--------|------|--------------|
| Modify | `src/internal/methods/NeoEsp32RmtMethod.h` | Add macro + replace `mem_block_num` literal |

---

### Task 1: Add `NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS` macro

**Files:**
- Modify: `src/internal/methods/NeoEsp32RmtMethod.h:52-56`

The `NEOPIXELBUS_RMT_INT_FLAGS` block ends at line 56. The new macro goes immediately after it.

- [ ] **Step 1: Locate the insertion point**

Open `src/internal/methods/NeoEsp32RmtMethod.h` and find lines 52–57:

```cpp
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
#define NEOPIXELBUS_RMT_INT_FLAGS (ESP_INTR_FLAG_LOWMED)
#else
#define NEOPIXELBUS_RMT_INT_FLAGS (ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1)
#endif
```

- [ ] **Step 2: Insert the macro immediately after that `#endif`**

The result should look exactly like this (the blank line before the `class NeoEsp32RmtSpeed` declaration that follows provides visual separation):

```cpp
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
#define NEOPIXELBUS_RMT_INT_FLAGS (ESP_INTR_FLAG_LOWMED)
#else
#define NEOPIXELBUS_RMT_INT_FLAGS (ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1)
#endif

#ifndef NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS
#define NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS 64
#endif
```

- [ ] **Step 3: Verify the macro was inserted correctly**

Run:
```powershell
Select-String -Path "src\internal\methods\NeoEsp32RmtMethod.h" -Pattern "NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS" -Context 1,1
```

Expected output: three lines — the `#ifndef`, the `#define 64`, and the `#endif`, with context showing the `NEOPIXELBUS_RMT_INT_FLAGS` block above.

---

### Task 2: Replace hardcoded `mem_block_num = 1`

**Files:**
- Modify: `src/internal/methods/NeoEsp32RmtMethod.h:621`

- [ ] **Step 1: Locate the line**

Run:
```powershell
Select-String -Path "src\internal\methods\NeoEsp32RmtMethod.h" -Pattern "mem_block_num"
```

Expected: one match, `config.mem_block_num = 1;`, inside `Initialize()`.

- [ ] **Step 2: Replace the literal**

Change:
```cpp
        config.mem_block_num = 1;
```
to:
```cpp
        config.mem_block_num = (NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS + 63) / 64;
```

The expression rounds up integer division so any symbol count maps to the minimum number of whole 64-symbol blocks needed. It is evaluated at compile time.

- [ ] **Step 3: Verify the replacement**

Run:
```powershell
Select-String -Path "src\internal\methods\NeoEsp32RmtMethod.h" -Pattern "mem_block_num" -Context 0,0
```

Expected: one match containing `(NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS + 63) / 64`. If the old literal `= 1` still appears, the edit was not saved.

- [ ] **Step 4: Sanity-check the math (manual review)**

Confirm these values at the match site:

| User defines | `mem_block_num` result | Half-buffer @800Kbps | Safe vs 300µs? |
|---|---|---|---|
| (default) 64 | `(64+63)/64 = 1` | ~40 µs | No (same as before) |
| 256 | `(256+63)/64 = 4` | ~160 µs | No |
| 512 | `(512+63)/64 = 8` | ~320 µs | **Yes** |

The default of 64 preserves backward compatibility (result = 1, same as hardcoded `1`).

---

### Task 3: Commit the implementation

**Files:**
- Modify: `src/internal/methods/NeoEsp32RmtMethod.h`

- [ ] **Step 1: Review the full diff**

Run:
```bash
git diff src/internal/methods/NeoEsp32RmtMethod.h
```

Expected diff shows exactly two hunks:
1. Three new lines (`#ifndef` / `#define 64` / `#endif`) after the `NEOPIXELBUS_RMT_INT_FLAGS` block
2. One changed line replacing `config.mem_block_num = 1` with the expression

No other lines should change.

- [ ] **Step 2: Stage and commit**

```bash
git add src/internal/methods/NeoEsp32RmtMethod.h
git commit -m "$(cat <<'EOF'
fix(ESP32 RMT): make mem_block_num user-overridable via NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS

Hardcoded mem_block_num=1 (64 symbols, ~40µs half-buffer at 800Kbps)
allows BLE/WiFi interrupt handlers to preempt the RMT refill ISR and
hold the data line idle past the 300µs LED latch threshold, producing
ghost pixels at 8-pixel intervals.

Add NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS (default 64, backward-compatible).
Projects with active BLE or WiFi can set
  -D NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS=512
to allocate 8 memory blocks (320µs half-buffer), eliminating false latches.

Fixes: #921

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

Expected output: `[issue-921-... <hash>] fix(ESP32 RMT): make mem_block_num user-overridable...`

---

### Task 4: Push branch and open PR

- [ ] **Step 1: Push the branch to origin**

```bash
git push origin "issue-921-ESP32-+-BLE/WiFi-mem_block_symbols"
```

Expected: `Branch 'issue-921-...' set up to track remote branch` (or `Everything up-to-date` if already pushed, plus the new commit listed).

- [ ] **Step 2: Create the PR against Makuna/NeoPixelBus master**

```bash
gh pr create \
  --repo Makuna/NeoPixelBus \
  --base master \
  --head "darkgrue:issue-921-ESP32-+-BLE/WiFi-mem_block_symbols" \
  --title "fix(ESP32 RMT): make RMT memory block size user-overridable (fixes BLE/WiFi ghost pixels)" \
  --body "$(cat <<'EOF'
## Problem

When a BLE or WiFi connection is active on ESP32, sparse LED patterns produce ghost pixels at index N+8 for every addressed pixel N. The defect scales with strip position and is introduced during RMT transmission, not in the software color buffer.

**Root cause:** `config.mem_block_num = 1` allocates a single 64-symbol RMT memory block. With ping-pong DMA, the RMT refill ISR must be serviced within ~40 µs at WS2812x 800 Kbps. NimBLE and WiFi event handlers run at higher interrupt priority than the RMT refill ISR and routinely preempt for longer, leaving the data line idle past the ~300 µs LED latch threshold.

Fixes #921.

## Solution

Add a user-overridable preprocessor macro `NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS` that controls the number of RMT memory blocks allocated.

**Default (backward compatible):** 64 symbols → `mem_block_num = 1` (unchanged behavior)

**BLE/WiFi fix:** add to build flags:
\`\`\`
-D NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS=512
\`\`\`
This allocates 8 memory blocks; the ping-pong half-buffer holds 256 symbols × 1.25 µs = **320 µs**, which exceeds the 300 µs latch threshold.

## Changes

`src/internal/methods/NeoEsp32RmtMethod.h`:
- Added `#ifndef NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS` / `#define NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS 64` / `#endif` after the `NEOPIXELBUS_RMT_INT_FLAGS` block
- Changed `config.mem_block_num = 1;` → `config.mem_block_num = (NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS + 63) / 64;`

## Hardware Constraints

On the original ESP32 (8 RMT channels × 64 symbols each), setting `NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS=512` consumes all 8 channels' memory for one strip. Users running multiple simultaneous RMT strips must choose a value that fits within available channels. Recommended values by use case:

| Scenario | Recommended value | `mem_block_num` |
|---|---|---|
| No BLE/WiFi (default) | 64 | 1 |
| BLE or WiFi active (ESP32) | 512 | 8 |
| BLE or WiFi active (S2/S3/C3, 4 ch) | 256 | 4 |

## Testing

Verified by the issue reporter's reproduction sketch (ESP32 + NimBLE server, single pixel set to red):
- Without fix: pixel 0 and pixel 8 both light red with a BLE client connected
- With `-D NEOPIXELBUS_RMT_MEM_BLOCK_SYMBOLS=512`: only pixel 0 lights
EOF
)"
```

Expected: a URL to the new PR, e.g. `https://github.com/Makuna/NeoPixelBus/pull/NNN`

---

## Post-Implementation Checklist

- [ ] PR URL captured and shared with issue reporter
- [ ] Issue #921 referenced in PR body (`Fixes #921`)
- [ ] Default value (64) preserves backward compatibility confirmed in diff
- [ ] Math table verified: `(512+63)/64 = 8`, `(64+63)/64 = 1`
