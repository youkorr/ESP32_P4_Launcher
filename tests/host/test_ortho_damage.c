/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../components/lx_comp/include/lx_damage.h"
#include "../../boards/m5stack_tab5/board/fb_fence.h"

static uint16_t fb[3][LX_DMG_W*LX_DMG_H], reference[LX_DMG_W*LX_DMG_H];
static uint32_t rng=0x7a8d2e31;
static uint32_t rnd(void) { rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng; }
static void paint(uint16_t *p,int x,int y,int w,int h,uint16_t v)
{
    for(int yy=y;yy<y+h;++yy)
        for(int xx=x;xx<x+w;++xx) p[(size_t)yy*LX_DMG_W+xx]=v;
}
int main(void)
{
    lx_damage_t d={0};
    lx_fb_fence_t f={.count=3,.epoch=UINT32_MAX-3};
    lx_damage_reset(&d);
    assert(!d.valid);
    uint64_t copied=0,full_bytes=0,checks=0;
    unsigned partial=0,full=0,cancel=0,foreign=0,blocked=0;
    for(unsigned step=0;step<6000;++step) {
        unsigned events=rnd()%3;
        for(unsigned i=0;i<events;++i) lx_fb_completed(&f);
        int back=lx_fb_find_back(&f);
        if(back<0) {++blocked;continue;}
        assert(lx_fb_writable(&f,back));
        if(step%71==0) { /* app, overlay, or failed foreign presentation */
            memset(fb[back],0xa5,sizeof(fb[back]));
            if(step%2==0) lx_fb_submitted(&f,back);
            lx_damage_reset(&d);
            ++foreign;
            continue;
        }
        bool bootstrap=!d.valid;
        lx_damage_map_t changed;
        lx_damage_clear(&changed);
        if(bootstrap) {
            for(size_t i=0;i<sizeof(reference)/2;++i) reference[i]=(uint16_t)(i^step);
            memcpy(fb[back],reference,sizeof(reference)); /* complete render */
            lx_damage_all(&changed);
        } else {
            assert(d.latest==f.visible && d.latest!=back);
            size_t n=lx_damage_sync(fb[back],fb[d.latest],&d.stale[back]);
            copied+=n;
            full_bytes+=sizeof(reference);
            assert(n<=sizeof(reference));
            assert(memcmp(fb[back],reference,sizeof(reference))==0);
            for(unsigned r=0;r<1+step%5;++r) {
                int x=(int)(rnd()%LX_DMG_W),y=(int)(rnd()%LX_DMG_H);
                int w=1+(int)(rnd()%87),h=1+(int)(rnd()%43);
                if(x+w>LX_DMG_W) w=LX_DMG_W-x;
                if(y+h>LX_DMG_H) h=LX_DMG_H-y;
                uint16_t v=(uint16_t)rnd();
                paint(fb[back],x,y,w,h,v);
                paint(reference,x,y,w,h,v);
                assert(lx_damage_mark(&changed,x,y,x+w-1,y+h-1));
            }
        }
        assert(memcmp(fb[back],reference,sizeof(reference))==0);
        checks+=sizeof(reference)/2;
        if(step%97==0) { /* aborted lease: never publish it */
            lx_damage_reset(&d);
            ++cancel;
            continue;
        }
        lx_fb_submitted(&f,back);
        lx_damage_commit(&d,back,&changed,bootstrap);
        assert(d.latest==f.visible);
        if(bootstrap) ++full; else ++partial;
    }
    assert(!lx_damage_mark(&(lx_damage_map_t){0},-1,0,1,1));
    assert(!lx_damage_mark(&(lx_damage_map_t){0},0,0,LX_DMG_W,LX_DMG_H));
    assert(partial>1000 && full && cancel && foreign && blocked && copied<full_bytes);
    printf("OK %u partial, %u full, %u cancelled, %u foreign, %u blocked; %llu pixel checks; sync %llu/%llu bytes\n",
        partial,full,cancel,foreign,blocked,(unsigned long long)checks,
        (unsigned long long)copied,(unsigned long long)full_bytes);
    return 0;
}
