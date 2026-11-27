/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include "../../launcher/main/ortho_coords.h"
int main(void)
{
    const int rotations[] = {0,90,180,270};
    const int pw = 720, ph = 1280;
    unsigned long count = 0;
    for (unsigned i=0; i<4; ++i) {
        int r=rotations[i], w=(r==90||r==270)?ph:pw, h=(r==90||r==270)?pw:ph;
        for (int y=0;y<h;y++) for(int x=0;x<w;x++) {
            lx_ortho_point_t p=lx_ortho_native_point(r,pw,ph,x,y);
            assert(p.x>=0 && p.x<pw && p.y>=0 && p.y<ph);
            /* LVGL 9.6 lv_display_rotate_point, default clockwise input. */
            int xx=p.x, yy=p.y;
            if(r==90) {xx=ph-p.y-1; yy=p.x;}
            if(r==180) {xx=pw-p.x-1; yy=ph-p.y-1;}
            if(r==270) {xx=p.y; yy=pw-p.x-1;}
            assert(xx==x && yy==y);
            count++;
        }
    }
    printf("OK %lu coordinate round trips\n",count);
}
