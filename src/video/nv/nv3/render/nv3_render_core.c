/*
* 86Box    A hypervisor and IBM PC system emulator that specializes in
*          running old operating systems and software designed for IBM
*          PC systems and compatibles from 1981 through fairly recent
*          system designs based on the PCI bus.
*
*          This file is part of the 86Box distribution.
*
*          NV3 Core rendering code (Software version)
*
* 
* 
* Authors: Connor Hyde, <mario64crashed@gmail.com> I need a better email address ;^)
*
*          Copyright 2024-2025 Connor Hyde
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <86box/86box.h>
#include <86box/device.h>
#include <86box/mem.h>
#include <86box/pci.h>
#include <86box/plat.h>
#include <86box/rom.h>
#include <86box/video.h>
#include <86box/nv/vid_nv.h>
#include <86box/nv/vid_nv3.h>
#include <86box/utils/video_stdlib.h>

/* Expand a colour.
   NOTE: THE GPU INTERNALLY OPERATES ON RGB10!!!!!!!!!!!
*/
nv3_color_expanded_t nv3_render_expand_color(uint32_t color, nv3_grobj_t grobj)
{
    // grobj0 = seems to share the format of PGRAPH_CONTEXT_SWITCH register.

    uint8_t format = (grobj.grobj_0 & 0x07);
    bool alpha_enabled = (grobj.grobj_0 >> NV3_PGRAPH_CONTEXT_SWITCH_ALPHA) & 0x01;

    nv3_color_expanded_t color_final; 
    // set the pixel format
    color_final.pixel_format = format;

    nv_log_verbose_only("Expanding Colour 0x%08x using pgraph_pixel_format 0x%x alpha enabled=%d\n", color, format, alpha_enabled);

    
    // default to fully opaque in case alpha is disabled
    color_final.a = 0xFF;

    switch (format)
    {
        // ALL OF THESE TYPES ARE 32 BITS IN SIZE

        // 555
        case nv3_pgraph_pixel_format_r5g5b5:
            // "stretch out" the colour

            color_final.a = (color >> 15) & 0x01;       // will be ignored if alpha_enabled isn't used 
            color_final.r = ((color >> 10) & 0x1F) << 5;
            color_final.g = ((color >> 5) & 0x1F) << 5;
            color_final.b = (color & 0x1F) << 5;

            break;
        // 888 (standard colour + 8-bit alpha)
        case nv3_pgraph_pixel_format_r8g8b8:
            if (alpha_enabled)
                color_final.a = ((color >> 24) & 0xFF) * 4;

            color_final.r = ((color >> 16) & 0xFF) * 4;
            color_final.g = ((color >> 8) & 0xFF) * 4;
            color_final.b = (color & 0xFF) * 4;

            break;
        case nv3_pgraph_pixel_format_r10g10b10:
            color_final.a = (color << 31) & 0x01;
            color_final.r = (color << 30) & 0x3FF;
            color_final.g = (color << 20) & 0x1FF;
            color_final.b = (color << 10);

            break;
        case nv3_pgraph_pixel_format_y8:
            color_final.a = (color >> 8) & 0xFF;

            // yuv
            color_final.r = color_final.g = color_final.b = (color & 0xFF) * 4; // convert to rgb10
            break;
        case nv3_pgraph_pixel_format_y16:
            color_final.a = (color >> 16) & 0xFFFF;

            // yuv
            color_final.r = color_final.g = color_final.b = (color & 0xFFFF) * 4; // convert to rgb10
            break;
        default:
            warning("nv3_render_expand_color unknown format %d", format);
            break;
        
    }

    // i8 is a union under i16
    color_final.i16 = (color & 0xFFFF);

    return color_final;
}

/* Used for chroma test */
uint32_t nv3_render_downconvert_color(nv3_grobj_t grobj, nv3_color_expanded_t color)
{
    uint8_t format = (grobj.grobj_0 & 0x07);
    bool alpha_enabled = (grobj.grobj_0 >> NV3_PGRAPH_CONTEXT_SWITCH_ALPHA) & 0x01;

    nv_log_verbose_only("Downconverting Colour 0x%08x using pgraph_pixel_format 0x%x alpha enabled=%d\n", color, format, alpha_enabled);

    uint32_t packed_color = 0x00;

    switch (format)
    {
        case nv3_pgraph_pixel_format_r5g5b5:
            packed_color = (color.r >> 5) << 10 | 
                           (color.g >> 5) << 5 |
                           (color.b >> 5);

            break;
        case nv3_pgraph_pixel_format_r8g8b8:
            packed_color = (color.a) << 24 | // is this a thing?
                           (color.r >> 2) << 16 |
                           (color.g >> 2) << 8 |
                           color.b;
            break;
        case nv3_pgraph_pixel_format_r10g10b10:
            /* sometimes alpha isn't used but we should incorporate it anyway */
            if (color.a > 0x00) packed_color | (1 << 31);

            packed_color |= (color.r << 30);
            packed_color |= (color.g << 20);
            packed_color |= (color.b << 10);
            break;
        case nv3_pgraph_pixel_format_y8:
            warning("nv3_render_downconvert: Y8 not implemented");
            break;
        case nv3_pgraph_pixel_format_y16:
            warning("nv3_render_downconvert: Y16 not implemented");
            break;
        default:
            warning("nv3_render_downconvert_color unknown format %d", format);
            break;

    }

    return packed_color;
}

/* Runs the chroma key/color key test */
bool nv3_render_chroma_test(uint32_t color, nv3_grobj_t grobj)
{
    bool chroma_enabled = ((grobj.grobj_0 >> NV3_PGRAPH_CONTEXT_SWITCH_CHROMA_KEY) & 0x01);

    if (!chroma_enabled)
        return true;
    
    bool alpha = ((nv3->pgraph.chroma_key >> 31) & 0x01);

    if (!alpha)
        return true;

    /* this is dumb but i'm lazy, if it kills perf, fix it later - we need to do some format shuffling */
    nv3_grobj_t grobj_fake = {0};
    grobj_fake.grobj_0 = 0x02; /* we don't care about any other bits */

    nv3_color_expanded_t chroma_expanded = nv3_render_expand_color(nv3->pgraph.chroma_key, grobj_fake);
   
    uint32_t chroma_downconverted = nv3_render_downconvert_color(grobj, chroma_expanded);

    return !(chroma_downconverted == color);
}

/* Convert expanded colour format to chroma key format */
uint32_t nv3_render_to_chroma(nv3_color_expanded_t expanded)
{
    // convert the alpha to 1 bit. then return packed rgb10
    return !!expanded.a | (expanded.r << 30) | (expanded.b << 20) | (expanded.a << 10);
}

/* Convert a rgb10 colour to a pattern colour */
uint32_t nv3_render_set_pattern_color(nv3_color_expanded_t pattern_colour, bool use_color1)
{
    /* reset the colour */
    if (!use_color1)
        nv3->pgraph.pattern_color_0_rgb.r = nv3->pgraph.pattern_color_0_rgb.g = nv3->pgraph.pattern_color_0_rgb.b = 0x00;
    else 
        nv3->pgraph.pattern_color_1_rgb.r = nv3->pgraph.pattern_color_1_rgb.g = nv3->pgraph.pattern_color_1_rgb.b = 0x00;
   
    /* select the right pattern colour, _rgb is already in RGB10 format, so we don't need to do any conversion */

    if (!use_color1)
    {
        nv3->pgraph.pattern_color_0_alpha = (pattern_colour.a) & 0xFF;
        nv3->pgraph.pattern_color_0_rgb.r = pattern_colour.r;
        nv3->pgraph.pattern_color_0_rgb.g = pattern_colour.g;
        nv3->pgraph.pattern_color_0_rgb.b = pattern_colour.b;

    }
    else 
    {
        nv3->pgraph.pattern_color_1_alpha = (pattern_colour.a) & 0xFF;
        nv3->pgraph.pattern_color_1_rgb.r = pattern_colour.r;
        nv3->pgraph.pattern_color_1_rgb.g = pattern_colour.g;
        nv3->pgraph.pattern_color_1_rgb.b = pattern_colour.b;
    }
    
}

/* Combine the current buffer with the pitch to get the address in the framebuffer to draw from for a given position. */
uint32_t nv3_render_get_vram_address(nv3_position_16_t position, nv3_grobj_t grobj, bool use_destination)
{
    uint32_t vram_x = position.x;
    uint32_t vram_y = position.y;
    uint32_t current_buffer = (grobj.grobj_0 >> NV3_PGRAPH_CONTEXT_SWITCH_SRC_BUFFER) & 0x03; 

    /* test DST_BUFFER code
    I assume for 2d at least only one is allowed at a time
    */

    if (use_destination)
    {
        uint32_t destination_buffer = 5; // 5 = just use the source buffer

        if ((grobj.grobj_0 >> NV3_PGRAPH_CONTEXT_SWITCH_DST_BUFFER0_ENABLED) & 0x01) destination_buffer = 0;
        if ((grobj.grobj_0 >> NV3_PGRAPH_CONTEXT_SWITCH_DST_BUFFER1_ENABLED) & 0x01) destination_buffer = 1;
        if ((grobj.grobj_0 >> NV3_PGRAPH_CONTEXT_SWITCH_DST_BUFFER2_ENABLED) & 0x01) destination_buffer = 2;
        if ((grobj.grobj_0 >> NV3_PGRAPH_CONTEXT_SWITCH_DST_BUFFER3_ENABLED) & 0x01) destination_buffer = 3;
    
        if (destination_buffer != current_buffer
        && destination_buffer != 5)
            current_buffer = destination_buffer;
    
    }

    uint32_t framebuffer_bpp = nv3->nvbase.svga.bpp;

    // we have to multiply the x position by the number of bytes per pixel
    switch (framebuffer_bpp)
    {
        case 8:
            break;
        case 15:
        case 16:
            vram_x = position.x << 1;
            break;
        case 32:
            vram_x = position.x << 2;
            break;
    }

    uint32_t pixel_addr_vram = vram_x + (nv3->pgraph.bpitch[current_buffer] * vram_y) + nv3->pgraph.boffset[current_buffer];

    pixel_addr_vram &= nv3->nvbase.svga.vram_mask;

    return pixel_addr_vram;
}

/* Read an 8bpp pixel from the framebuffer. */
uint8_t nv3_render_read_pixel_8(nv3_position_16_t position, nv3_grobj_t grobj, bool use_destination)
{ 
    // hope you call it with the right bit
    uint32_t vram_address = nv3_render_get_vram_address(position, grobj, use_destination);

    return nv3->nvbase.svga.vram[vram_address];
}

/* Read an 16bpp pixel from the framebuffer. */
uint16_t nv3_render_read_pixel_16(nv3_position_16_t position, nv3_grobj_t grobj, bool use_destination)
{ 
    // hope you call it with the right bit
    uint32_t vram_address = nv3_render_get_vram_address(position, grobj, use_destination);

    uint16_t* vram_16 = (uint16_t*)(nv3->nvbase.svga.vram);
    vram_address >>= 1; //convert to 16bit pointer

    return vram_16[vram_address];
}

/* Read an 16bpp pixel from the framebuffer. */
uint32_t nv3_render_read_pixel_32(nv3_position_16_t position, nv3_grobj_t grobj, bool use_destination)
{ 
    // hope you call it with the right bit
    uint32_t vram_address = nv3_render_get_vram_address(position, grobj, use_destination);

    uint32_t* vram_32 = (uint32_t*)(nv3->nvbase.svga.vram);
    vram_address >>= 2; //convert to 32bit pointer

    return vram_32[vram_address];
}

/* Plots a pixel. */
void nv3_render_write_pixel(nv3_position_16_t position, uint32_t color, nv3_grobj_t grobj)
{
    uint8_t alpha = 0xFF;

    // PFB_0 is always set to hardcoded "NO_TILING" value of 0x1114.
    // It seems, you are meant to 

    /* put this here for debugging + it may be needed later. */
    #ifdef DEBUG
    uint8_t color_format_object = (grobj.grobj_0 >> NV3_PGRAPH_CONTEXT_SWITCH_COLOR_FORMAT) & 0x07;
    #endif
    bool alpha_enabled = (grobj.grobj_0 >> NV3_PGRAPH_CONTEXT_SWITCH_ALPHA) & 0x01;

    uint32_t framebuffer_bpp = nv3->nvbase.svga.bpp; // maybe y16 too?z

    /* doesn't seem*/
    nv3_color_argb_t color_data = *(nv3_color_argb_t*)&color;

    if (framebuffer_bpp == 32)
        alpha = color_data.a;
    
    int32_t clip_end_x = nv3->pgraph.clip_start.x + nv3->pgraph.clip_size.x;
    int32_t clip_end_y = nv3->pgraph.clip_start.y + nv3->pgraph.clip_size.y;
    
    /* First, check our current buffer. */
    /* Then do the clip. */
    if (position.x < nv3->pgraph.clip_start.x
        || position.y < nv3->pgraph.clip_start.y
        || position.x > clip_end_x
        || position.y > clip_end_y)
        {
            // DO NOT DRAW THE PIXEL
            return;
        }

    /* TODO: Plane Mask...*/
    if (!nv3_render_chroma_test(color, grobj))
        return;

    uint32_t pixel_addr_vram = nv3_render_get_vram_address(position, grobj, true);

    uint32_t rop_src = 0, rop_dst = 0, rop_pattern = 0;
    uint8_t bit = 0x00; 

    /* Get our pattern data, may move to another function */
    switch (nv3->pgraph.pattern.shape)
    {

        /* This logic is from NV1 envytoos docs, but seems to be same on NV3*/
        case NV3_PATTERN_SHAPE_8X8:
            bit = (position.x & 7) | (position.y & 7) << 3;
            break; 
        case NV3_PATTERN_SHAPE_1X64:
            bit = (position.x & 0x3f);
            break;
        case NV3_PATTERN_SHAPE_64X1:
            bit = (position.y & 0x3f);
            break;
    }

    // pull out the actual bit and see which colour we need to use

    bool use_color1 = (nv3->pgraph.pattern_bitmap >> bit) & 0x01;

    if (!use_color1)
    {
        if (!nv3->pgraph.pattern_color_0_alpha)
            return; 

        /* This is stupid */
        rop_pattern = nv3_render_downconvert_color(grobj, nv3->pgraph.pattern_color_0_rgb);
    }
    else
    {
        if (!nv3->pgraph.pattern_color_1_alpha)
            return;

        rop_pattern = nv3_render_downconvert_color(grobj, nv3->pgraph.pattern_color_1_rgb);
    }
    

    /*  Go to vram and do the final ROP for a basic bitblit.
        It seems we can skip the downconversion step *for now*, since (framebuffer bits per pixel) == (object bits per pixel) 
        I'm not sure how games will react. But it depends on how the D3D drivers operate, we may need ro convert texture formats to the current bpp internally.

        TODO: MOVE TO BPIXEL DEPTH or GROBJ0 to determine this, once we figure out how to get the bpixel depth.
    */

    switch (framebuffer_bpp)
    {
        case 8:
            rop_src = color & 0xFF;
            rop_dst = nv3->nvbase.svga.vram[pixel_addr_vram];
            nv3->nvbase.svga.vram[pixel_addr_vram] = video_rop_gdi_ternary(nv3->pgraph.rop, rop_src, rop_dst, rop_pattern) & 0xFF;

            nv3->nvbase.svga.changedvram[pixel_addr_vram >> 12] = changeframecount;

            break;
        case 15:
        case 16:
            uint16_t* vram_16 = (uint16_t*)(nv3->nvbase.svga.vram);
            pixel_addr_vram >>= 1; 
  
            // mask to 16bit 

            rop_src = color & 0xFFFF; 

            /* if alpha is turned on and we aren't in 565 mode, reject transparent pixels */
            bool is_16bpp = (nv3->pramdac.general_control >> NV3_PRAMDAC_GENERAL_CONTROL_565_MODE) & 0x01;

            // a1r5g5b5
            if (!is_16bpp)
            {
                if (alpha_enabled && 
                    !(color & 0x8000))
                    return;
            }

            // convert to 16bpp
            // forcing it to render in 15bpp fixes it, 

            rop_dst = vram_16[pixel_addr_vram];

            vram_16[pixel_addr_vram] = video_rop_gdi_ternary(nv3->pgraph.rop, rop_src, rop_dst, rop_pattern) & 0xFFFF;

            nv3->nvbase.svga.changedvram[pixel_addr_vram >> 11] = changeframecount;

            break;
        case 32:
            uint32_t* vram_32 = (uint32_t*)(nv3->nvbase.svga.vram);
            pixel_addr_vram >>= 2; 

            rop_src = color;
            rop_dst = vram_32[pixel_addr_vram];
            vram_32[pixel_addr_vram] = video_rop_gdi_ternary(nv3->pgraph.rop, rop_src, rop_dst, rop_pattern);

            nv3->nvbase.svga.changedvram[pixel_addr_vram >> 10] = changeframecount;

            break;
    }
}


/* 
    Modified SVGA Core functions 
    Done, to facilitate the buffer changing around stuff.

    TODO: Do we need a custom 8bpp function?
*/

void nv3_render_15bpp(svga_t *svga)
{
    int       x;
    uint32_t *p;
    uint32_t  dat;
    uint32_t  changed_addr;
    uint32_t  addr;

    /* "force_old_addr" code path removed, since we don't need it */

    if ((svga->displine + svga->y_add) < 0)
        return;

    changed_addr = svga->remap_func(svga, svga->ma);

    if (svga->changedvram[changed_addr >> 12] || svga->changedvram[(changed_addr >> 12) + 1] || svga->fullchange) {
        p = &svga->monitor->target_buffer->line[svga->displine + svga->y_add][svga->x_add];

        if (svga->firstline_draw == 2000)
            svga->firstline_draw = svga->displine;
        svga->lastline_draw = svga->displine;

        if (!svga->remap_required) {
            for (x = 0; x <= (svga->hdisp + svga->scrollcache); x += 8) {
                dat  = *(uint32_t *) (&svga->vram[(svga->ma + (x << 1)) & svga->vram_display_mask]);
                *p++ = svga->conv_16to32(svga, dat & 0xffff, 15);
                *p++ = svga->conv_16to32(svga, dat >> 16, 15);

                dat  = *(uint32_t *) (&svga->vram[(svga->ma + (x << 1) + 4) & svga->vram_display_mask]);
                *p++ = svga->conv_16to32(svga, dat & 0xffff, 15);
                *p++ = svga->conv_16to32(svga, dat >> 16, 15);

                dat  = *(uint32_t *) (&svga->vram[(svga->ma + (x << 1) + 8) & svga->vram_display_mask]);
                *p++ = svga->conv_16to32(svga, dat & 0xffff, 15);
                *p++ = svga->conv_16to32(svga, dat >> 16, 15);

                dat  = *(uint32_t *) (&svga->vram[(svga->ma + (x << 1) + 12) & svga->vram_display_mask]);
                *p++ = svga->conv_16to32(svga, dat & 0xffff, 15);
                *p++ = svga->conv_16to32(svga, dat >> 16, 15);
            }
            svga->ma += x << 1;
        } else {
            for (x = 0; x <= (svga->hdisp + svga->scrollcache); x += 2) {
                addr = svga->remap_func(svga, svga->ma);
                dat  = *(uint32_t *) (&svga->vram[(addr) & svga->vram_display_mask]);

                *p++ = svga->conv_16to32(svga, dat & 0xffff, 15);
                *p++ = svga->conv_16to32(svga, dat >> 16, 15);
                svga->ma += 4;
            }
        }
        svga->ma &= svga->vram_display_mask;
    }
}


void nv3_render_16bpp(svga_t *svga)
{
    int       x;
    uint32_t *p;
    uint32_t  dat;
    uint32_t  changed_addr;
    uint32_t  addr;

    if ((svga->displine + svga->y_add) < 0)
        return;

    changed_addr = svga->remap_func(svga, svga->ma);

    /* "force_old_addr" code path removed, since we don't need it */
    if (svga->changedvram[changed_addr >> 12] || svga->changedvram[(changed_addr >> 12) + 1] || svga->fullchange) {
        p = &svga->monitor->target_buffer->line[svga->displine + svga->y_add][svga->x_add];

        if (svga->firstline_draw == 2000)
            svga->firstline_draw = svga->displine;
        svga->lastline_draw = svga->displine;

        if (!svga->remap_required) {
            for (x = 0; x <= (svga->hdisp + svga->scrollcache); x += 8) {
                dat  = *(uint32_t *) (&svga->vram[(svga->ma + (x << 1)) & svga->vram_display_mask]);
                *p++ = svga->conv_16to32(svga, dat & 0xffff, 16);
                *p++ = svga->conv_16to32(svga, dat >> 16, 16);

                dat  = *(uint32_t *) (&svga->vram[(svga->ma + (x << 1) + 4) & svga->vram_display_mask]);
                *p++ = svga->conv_16to32(svga, dat & 0xffff, 16);
                *p++ = svga->conv_16to32(svga, dat >> 16, 16);

                dat  = *(uint32_t *) (&svga->vram[(svga->ma + (x << 1) + 8) & svga->vram_display_mask]);
                *p++ = svga->conv_16to32(svga, dat & 0xffff, 16);
                *p++ = svga->conv_16to32(svga, dat >> 16, 16);

                dat  = *(uint32_t *) (&svga->vram[(svga->ma + (x << 1) + 12) & svga->vram_display_mask]);
                *p++ = svga->conv_16to32(svga, dat & 0xffff, 16);
                *p++ = svga->conv_16to32(svga, dat >> 16, 16);
            }
            svga->ma += x << 1;
        } else {
            for (x = 0; x <= (svga->hdisp + svga->scrollcache); x += 2) {
                addr = svga->remap_func(svga, svga->ma);
                dat  = *(uint32_t *) (&svga->vram[(addr) & svga->vram_display_mask]);

                *p++ = svga->conv_16to32(svga, dat & 0xffff, 16);
                *p++ = svga->conv_16to32(svga, dat >> 16, 16);

                svga->ma += 4;
            }
        }
        svga->ma &= svga->vram_display_mask;
    }
}


void nv3_render_32bpp(svga_t *svga)
{
    int       x;
    uint32_t *p;
    uint32_t  dat;
    uint32_t  changed_addr;
    uint32_t  addr;

    if ((svga->displine + svga->y_add) < 0)
        return;

    /* "force_old_addr" code path removed, since we don't need it */

    changed_addr = svga->remap_func(svga, svga->ma);

    if (svga->changedvram[changed_addr >> 12] || svga->changedvram[(changed_addr >> 12) + 1] || svga->fullchange) {
        p = &svga->monitor->target_buffer->line[svga->displine + svga->y_add][svga->x_add];

        if (svga->firstline_draw == 2000)
            svga->firstline_draw = svga->displine;
        svga->lastline_draw = svga->displine;

        if (!svga->remap_required) {
            for (x = 0; x <= (svga->hdisp + svga->scrollcache); x++) {
                dat  = *(uint32_t *) (&svga->vram[(svga->ma + (x << 2)) & svga->vram_display_mask]);
                *p++ = svga_lookup_lut_ram(svga, dat & 0xffffff);
            }
            svga->ma += (x * 4);
        } else {
            for (x = 0; x <= (svga->hdisp + svga->scrollcache); x++) {
                addr = svga->remap_func(svga, svga->ma);
                dat  = *(uint32_t *) (&svga->vram[addr & svga->vram_display_mask]);
                *p++ = svga_lookup_lut_ram(svga, dat & 0xffffff);

                svga->ma += 4;
            }
        }
        svga->ma &= svga->vram_display_mask;
    }
}
