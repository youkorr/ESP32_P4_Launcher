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
