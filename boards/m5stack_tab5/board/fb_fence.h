/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 youkorr
 * Experimental two-buffer retirement model; serialized by BSP spinlock.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t epoch, retired_at[2];
    bool pending[2];
    int visible;
} lx_fb_fence_t;

static inline void lx_fb_completed(lx_fb_fence_t *f)
{
    ++f->epoch;
}

static inline bool lx_fb_writable(const lx_fb_fence_t *f, int index)
{
    return index >= 0 && index < 2 && index != f->visible &&
           (!f->pending[index] || (uint32_t)(f->epoch - f->retired_at[index]) >= 2);
}

/* Call only after a successful native buffer submission. Count two later
 * DMA callbacks conservatively, including a callback racing with submission.
 * No assumptions about which buffer a callback refers to. */
static inline void lx_fb_submitted(lx_fb_fence_t *f, int index)
{
    if (index == f->visible) return;
    f->pending[f->visible] = true;
    f->retired_at[f->visible] = f->epoch;
    f->pending[index] = false;
    f->visible = index;
}
