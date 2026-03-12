/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          WELCOME TO NVIDIA DRIVER VERSION 2.0! 
 *
 * Authors: starfrost
 *
 *          Copyright 2024-2026 starfrost
 */

#include "nv1.h"
#include "nv1_regs.h"

void nv1_init(const device_t* dev)
{

}

void nv1_close(void* priv)
{
    
}

void nv1_speed_changed(void *priv)
{

}

void nv1_draw_cursor(svga_t* svga, int32_t drawline)
{

}

void nv1_recalc_timings(svga_t* svga)
{

}

void nv1_force_redraw(void* priv)
{

}

// See if the bios rom is available.
int32_t nv1_available(void)
{
    return (rom_present(NV1_VBIOS_E3D_3X00));
}

// NV3T (RIVA 128 ZX)
// PCI
// 8MB VRAM
const device_t nv1_device = 
{
    .name = "NVIDIA NV1 [Diamond Edge 3D 3400] [Not Direct3D Compatible]",
    .internal_name = "nv1",
    .flags = DEVICE_PCI,
    .local = 0,
    .init = nv1_init,
    .close = nv1_close,
    .speed_changed = nv1_speed_changed,
    .force_redraw = nv1_force_redraw,
    .available = nv1_available,
    .config = nv1_config,
};
