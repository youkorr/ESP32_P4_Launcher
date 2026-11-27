# Experimental external Ortho integration

This branch starts from upstream launcher 0.33.0 (de2df2a7889502b537be7a09be6c4d009a111e8c).
It is not a release. No changes to the PAPP ABI, board wiring, app binaries or
the existing compositor in the preflight stage.

## Stage 1 — compile compatibility

Run the Ortho preflight workflow: seven board targets, three isolated variants
(original LVGL 9.2.2, LVGL 9.6.0, LVGL 9.6.0 with the external registry
dependency youkorr/lv_draw_ortho 0.2.0). It also runs the existing PAPP
relocation regression. The dependency variant does not activate the draw unit:
a library build does not demonstrate a working accelerated launcher.

## Stage 2 — integration design and gates

Keep the original pipeline as default. Only enable the new renderer with an
explicit CONFIG_LX_USE_ORTHO option. Start with the existing private home buffer
in native panel orientation, not scanout buffers, and retain compositor
ownership. Check LVGL native stride and RGB565, use a conservative internal
tile budget, reserve memory for networking and PAPP FAST code, and keep the
first trial single-core. Coordinate PPA clients and bounded waits before
enabling the engine. Map already-landscape BSP touch coordinates back to
native coordinates before LVGL's input rotation. Preserve full redraw on
return from an app and retry failed presentation outside the flush callback.
Non-rotated and SPI boards retain their original path until validated.

Only after that stage passes compilation and host tests, consider direct
panel buffers: ownership/retirement fences must cover DevLink starts, app
surface creation/teardown, menus/overlays, screenshots, scanout-off, timeouts
and late callbacks. No write to a scanned or pending buffer. Do not add a third
buffer without measuring memory and the actual panel refresh rate.

Keep the engine external, pinned to its registry version. Do not duplicate
its code or change upstream PAPP binaries. Games already use PPA SRM and
asynchronous presents; game acceleration is a separate measured change.

## Required hardware acceptance (by a board owner)

Use a spare/test device; keep the original full firmware and SD backup.
Compare identical board/chip/panel, clock, PSRAM, scene and firmware settings.
Measure delivered FPS, CPU, PPA/flip timings, heap free/min/largest block,
PAPP FAST allocation, audio underruns, touch corners and screenshot orientation.
Test cold boot, scrolling, Store, Wi-Fi, repeated home/app/home, forced exits,
overlays, suspend/scanout and long-running applications. Do not publish OTA
or claim FPS improvements from compilation alone.

## Stage 2 prototype (ortho-renderer branch)

LVGL 9.6 and the external 0.2.0 registry package are pinned. The optional
CONFIG_LX_USE_ORTHO path is restricted to Tab5 with a native 90/270 degree
panel, two framebuffers, compatible stride and conservative heap gates.
One core and 32 KiB tile blocks; the original renderer is the default.
A private native buffer prevents LVGL from drawing into app scanout buffers;
its full frame is still copied by the compositor, not zero-copy. PPA init
failure leaves the original path; DMA stall stops Ortho presentation and
requires reset. This prototype does not change the original BSP's retirement
semantics, which still require board validation. The game PPA path and ABI
are unchanged. No speedup or hardware reliability is claimed. CI builds
both configurations on all seven boards and tests coordinate round trips.

## Stage 3 — performance safeguards and measurement (ortho-performance)

The Tab5 BSP now counts DMA completions using on_frame_buf_complete and
tracks the retirement epoch of each of the two buffers. It requires two
later callbacks after successful submission, conservatively avoiding a
stale binary semaphore or callback racing with submission. This is a
model based on ESP-IDF 5.5.6's DPI driver, not proof of physical timing.
It may increase flip wait and decrease FPS; measure before tuning it.
All compositor retirement waits now check their result; no write after
a timeout. Count successful submissions rather than ignored show errors.
Other BSP retirement semantics remain unchanged. DevLink screenshot
ownership and all hardware behavior still need validation.

Diagnostics every ten seconds: successful home/app/overlay submissions,
measured PPA and flip time, explicitly instrumented copies and avoided
self-copies, timeout/error counts, free/min/largest SRAM and PSRAM, and
Ortho tile/core counters. CPU busy is approximate when FreeRTOS runtime
stats use esp_timer; these are enabled by CI. Submission rates are not
optically measured FPS. Second core is a separate opt-in with a conservative
SRAM check. The PAPP ABI and external engine version are unchanged.

Zero-copy native LVGL, third framebuffer, shared PPA scheduling/bounded
SRM for game scaling, and eliminating external-frame copies with changed
ownership contracts are intentionally not implemented here. In particular,
existing blocking game PPA operations remain; a stuck PPA requires reset.
Do not share LVGL buffers with games until a full render lease and app
handoff (including DevLink starts, overlays, screenshots and errors) have
been tested. No OTA release or performance claims before hardware A/B.
