/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "../../boards/m5stack_tab5/board/fb_fence.h"

/* Exhaustive bounded model: submit, cancel, completion, read ownership.
 * Cancellation means no submission and leaves visible/retirement unchanged.
 * The compositor mutex serializes leases, apps, overlays and snapshots;
 * this test exercises the shared selection/retirement logic, not FreeRTOS. */
static unsigned long leaves, submissions, cancels, unavailable;
static void explore(lx_fb_fence_t f, int depth)
{
    if (!depth) { ++leaves; return; }
    for (int i = 0; i < f.count; ++i)
        if (lx_fb_writable(&f,i)) {
            assert(i != f.visible);
            assert(!f.pending[i] || (uint32_t)(f.epoch-f.retired_at[i]) >= 2);
        }
    int back = lx_fb_find_back(&f);
    if (back >= 0) {
        assert(lx_fb_writable(&f,back));
        lx_fb_fence_t next = f;
        int old = next.visible;
        lx_fb_submitted(&next,back);
        assert(next.visible == back);
        assert(!lx_fb_writable(&next,old));
        assert(!lx_fb_writable(&next,back));
        ++submissions;
        explore(next,depth-1);
        ++cancels;
        explore(f,depth-1);
    } else {
        ++unavailable;
        explore(f,depth-1);
    }
    lx_fb_completed(&f);
    explore(f,depth-1);
}
int main(void)
{
    for (int count = 2; count <= 3; ++count)
        for (unsigned i=0;i<3;++i) {
            lx_fb_fence_t f = {.count=count,.epoch=UINT32_MAX-i};
            explore(f,12);
        }
    assert(submissions && cancels && unavailable);
    printf("OK %lu leaves, %lu submits, %lu cancels, %lu unavailable\n",
           leaves,submissions,cancels,unavailable);
}
