/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 youkorr
 * Native Tab5 damage ledger, serialized by compositor ownership.
 * Tiles are conservative: extra pixels may be copied, none omitted.
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

enum { LX_DMG_W=720, LX_DMG_H=1280, LX_DMG_TW=32, LX_DMG_TH=16,
       LX_DMG_COLS=(LX_DMG_W+LX_DMG_TW-1)/LX_DMG_TW,
       LX_DMG_ROWS=(LX_DMG_H+LX_DMG_TH-1)/LX_DMG_TH,
       LX_DMG_WORDS=(LX_DMG_COLS*LX_DMG_ROWS+31)/32 };
typedef struct { uint32_t bits[LX_DMG_WORDS]; } lx_damage_map_t;
typedef struct {
    int latest;
    bool valid;
    lx_damage_map_t stale[3];
} lx_damage_t;
static inline void lx_damage_reset(lx_damage_t *d)
{
    d->valid = false;
    d->latest = -1;
    memset(d->stale, 0xff, sizeof(d->stale));
}
static inline void lx_damage_clear(lx_damage_map_t *m) { memset(m,0,sizeof(*m)); }
static inline void lx_damage_all(lx_damage_map_t *m) { memset(m,0xff,sizeof(*m)); }
static inline bool lx_damage_test(const lx_damage_map_t *m,int x,int y)
{
    unsigned i = (unsigned)(y*LX_DMG_COLS+x);
    return (m->bits[i/32] & (UINT32_C(1)<<(i%32))) != 0;
}
static inline bool lx_damage_mark(lx_damage_map_t *m,int x1,int y1,int x2,int y2)
{
    if(x1<0 || y1<0 || x2<x1 || y2<y1 || x2>=LX_DMG_W || y2>=LX_DMG_H) return false;
    for(int y=y1/LX_DMG_TH;y<=y2/LX_DMG_TH;++y)
        for(int x=x1/LX_DMG_TW;x<=x2/LX_DMG_TW;++x) {
            unsigned i=(unsigned)(y*LX_DMG_COLS+x);
            m->bits[i/32] |= UINT32_C(1)<<(i%32);
        }
    return true;
}
/* Only called after a successful submission of a complete, coherent frame.
 * Do not clear the old source's stale map until that buffer is synchronized. */
static inline void lx_damage_commit(lx_damage_t *d,int target,const lx_damage_map_t *changed,bool full)
{
    for(int i=0;i<3;++i) {
        if(i==target) lx_damage_clear(&d->stale[i]);
        else if(full || !d->valid) lx_damage_all(&d->stale[i]);
        else for(unsigned j=0;j<LX_DMG_WORDS;++j) d->stale[i].bits[j] |= changed->bits[j];
    }
    d->latest=target;
    d->valid=true;
}
/* Copy contiguous horizontal runs of stale tiles, including clipped edge.
 * Source is immutable (latest submitted home frame), destination retired.
 * The caller owns both and must perform target cache writeback before DMA. */
static inline size_t lx_damage_sync(uint16_t *dst,const uint16_t *src,const lx_damage_map_t *m)
{
    size_t bytes=0;
    for(int ty=0;ty<LX_DMG_ROWS;++ty) {
        int y1=ty*LX_DMG_TH,y2=y1+LX_DMG_TH;
        if(y2>LX_DMG_H) y2=LX_DMG_H;
        for(int tx=0;tx<LX_DMG_COLS;) {
            if(!lx_damage_test(m,tx,ty)) {++tx;continue;}
            int x1=tx*LX_DMG_TW;
            do {++tx;} while(tx<LX_DMG_COLS && lx_damage_test(m,tx,ty));
            int x2=tx*LX_DMG_TW;
            if(x2>LX_DMG_W) x2=LX_DMG_W;
            size_t n=(size_t)(x2-x1)*sizeof(uint16_t);
            for(int y=y1;y<y2;++y) {
                size_t off=(size_t)y*LX_DMG_W+x1;
                memcpy(dst+off,src+off,n);
                bytes+=n;
            }
        }
    }
    return bytes;
}
