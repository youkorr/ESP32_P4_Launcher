/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 youkorr
 */
#pragma once
typedef struct { int x, y; } lx_ortho_point_t;
/* Logical landscape -> physical panel. LVGL applies the inverse to touch. */
static inline lx_ortho_point_t lx_ortho_native_point(int rot, int pw, int ph, int x, int y)
{
    switch (rot) {
    case 90: return (lx_ortho_point_t){y, ph - 1 - x};
    case 180: return (lx_ortho_point_t){pw - 1 - x, ph - 1 - y};
    case 270: return (lx_ortho_point_t){pw - 1 - y, x};
    default: return (lx_ortho_point_t){x, y};
    }
}
