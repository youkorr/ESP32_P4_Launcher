/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "../../boards/m5stack_tab5/board/fb_fence.h"

int main(void)
{
    unsigned long cases = 0;
    /* Enumerate stale/pre-submit and post-submit events, repeated flips,
     * timeout (no event), and wraparound. Exercise the actual BSP model. */
    for (unsigned offset = 0; offset < 128; ++offset)
    for (unsigned before = 0; before < 6; ++before)
    for (unsigned after = 0; after < 6; ++after) {
        lx_fb_fence_t f = {.epoch = UINT32_MAX - offset};
        for (unsigned i = 0; i < before; ++i) lx_fb_completed(&f);
        for (unsigned round = 0; round < 64; ++round) {
            int old = f.visible, next = old ^ 1;
            assert(lx_fb_writable(&f, next));
            assert(!lx_fb_writable(&f, old));
            lx_fb_submitted(&f, next);
            assert(!lx_fb_writable(&f, next));
            assert(!lx_fb_writable(&f, old)); /* timeout / stale wake */
            for (unsigned n = 0; n < after; ++n) {
                lx_fb_completed(&f);
                assert(lx_fb_writable(&f, old) == (n >= 1));
                assert(!lx_fb_writable(&f, next));
            }
            for (unsigned n = after; n < 2; ++n) lx_fb_completed(&f);
            assert(lx_fb_writable(&f, old));
            assert(!lx_fb_writable(&f, next));
            ++cases;
        }
    }
    printf("OK %lu retirement sequences\n", cases);
    return 0;
}
