/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          WELCOME TO NVIDIA DRIVER VERSION 2.0!
 *          nv1_pfb: Framebuffer Interface
 *
 * Authors: starfrost
 *
 *          Copyright 2024-2026 starfrost
 */

#include "../nv1.h"

uint32_t nv1_pfb_read(uint32_t addr)
{
    uint32_t ret = 0x00;
    
    switch (addr)
    {
        case NV_PFB_BOOT_0:
            ret |= (NV_PFB_BOOT_0_DAC_WIDTH_64_BIT << NV_PFB_BOOT_0_DAC_WIDTH);
            
            if (nv1->vram_amount == NV1_VRAM_SIZE_1MB)
                ret |= (NV_PFB_BOOT_0_RAM_AMOUNT_1MB << NV_PFB_BOOT_0_RAM_AMOUNT);
            else if (nv1->vram_amount == NV1_VRAM_SIZE_2MB)
                ret |= (NV_PFB_BOOT_0_RAM_AMOUNT_2MB << NV_PFB_BOOT_0_RAM_AMOUNT);
            else if (nv1->vram_amount == NV1_VRAM_SIZE_4MB) 
                ret |= (NV_PFB_BOOT_0_RAM_AMOUNT_4MB << NV_PFB_BOOT_0_RAM_AMOUNT);
                

            break;
        case NV_PFB_GREEN_0:
            ret = nv1->pfb.green_0;
            break;
        case NV_PFB_HOR_FRNT_PORCH:
            ret = nv1->pfb.hfrontporch;
            break;
        case NV_PFB_HOR_SYNC_WIDTH:
            ret = nv1->pfb.hsync_width;
            break;
        case NV_PFB_HOR_BACK_PORCH:
            ret = nv1->pfb.hbackporch;
            break;
        case NV_PFB_HOR_DISP_WIDTH:
            ret = nv1->pfb.hdisp;
            break;
        case NV_PFB_VER_FRNT_PORCH:
            ret = nv1->pfb.vfrontporch;
            break;
        case NV_PFB_VER_SYNC_WIDTH:
            ret = nv1->pfb.vsync_width;
            break;
        case NV_PFB_VER_BACK_PORCH:
            ret = nv1->pfb.vbackporch;
            break;
        case NV_PFB_VER_DISP_WIDTH:
            ret = nv1->pfb.vdisp;
            break; 
    }

    return ret; 
}

void nv1_pfb_write(uint32_t addr, uint32_t val)
{
    // deosn't do anything yet
    bool recalc_needed = false;

    // nv1 is not vga compatible but these are rough equivalents
    switch (addr)
    {
        case NV_PFB_GREEN_0:
            nv1->pfb.green_0 = val;
            break;        
        case NV_PFB_HOR_FRNT_PORCH:
            nv1->pfb.hfrontporch = val;
            recalc_needed = true;
            break;
        case NV_PFB_HOR_SYNC_WIDTH:
            nv1->pfb.hsync_width = val;
            recalc_needed = true;
            break;
        case NV_PFB_HOR_BACK_PORCH:
            nv1->pfb.hbackporch = val;
            recalc_needed = true;
            break;
        case NV_PFB_HOR_DISP_WIDTH:
            nv1->pfb.hdisp = val;
            recalc_needed = true;
            break;
        case NV_PFB_VER_FRNT_PORCH:
            nv1->pfb.vfrontporch = val;
            recalc_needed = true;
            break;
        case NV_PFB_VER_SYNC_WIDTH:
            nv1->pfb.vsync_width = val;
            recalc_needed = true;
            break;
        case NV_PFB_VER_BACK_PORCH:
            nv1->pfb.vbackporch = val;
            recalc_needed = true;
            break;
        case NV_PFB_VER_DISP_WIDTH:
            nv1->pfb.vdisp = val;
            recalc_needed = true;
            break; 
    }

    // set svga stuff based on recalcualted values of this

    if (recalc_needed)
    {        
        nv1->svga.hdisp = nv1->pfb.hdisp;
        nv1->svga.htotal = nv1->pfb.hdisp + nv1->pfb.hbackporch;
        nv1->svga.vdisp = nv1->pfb.vdisp;
        nv1->svga.vtotal = nv1->pfb.vdisp + nv1->pfb.vbackporch;

        nv1->svga.hblankstart = nv1->pfb.hdisp;
        nv1->svga.hblankend = nv1->pfb.hdisp + nv1->pfb.hsync_width;
        nv1->svga.vblankstart = nv1->pfb.vdisp;
        nv1->svga.vblankend = nv1->pfb.vdisp + nv1->pfb.vsync_width;

        svga_recalctimings(&nv1->svga);
    }
}