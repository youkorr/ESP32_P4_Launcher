/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 youkorr
 * Experimental two/three-buffer retirement model; serialized by BSP spinlock.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t epoch, retired_at[3];
    bool pending[3];
    int visible, count;
} lx_fb_fence_t;

static inline void lx_fb_completed(lx_fb_fence_t *f)
{
    ++f->epoch;
}

static inline bool lx_fb_writable(const lx_fb_fence_t *f, int index)
{
    return index >= 0 && index < f->count && index != f->visible &&
           (!f->pending[index] || (uint32_t)(f->epoch - f->retired_at[index]) >= 2);
}

static inline int lx_fb_find_back(const lx_fb_fence_t *f)
{
    for (int n = 1; n < f->count; ++n) {
        int i = (f->visible + n) % f->count;
        if (lx_fb_writable(f, i)) return i;
    }
    return -1;
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
