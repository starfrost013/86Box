/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          NV3 bringup and device emulation.
 *
 *
 *
 * Authors: Connor Hyde, <mario64crashed@gmail.com> I need a better email address ;^)
 *
 *          Copyright 2024 starfrost
 */

// nv3_pramdac.c: NV3 RAMDAC
// Todo: Allow overridability using 68050C register...

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <86Box/86box.h>
#include <86Box/device.h>
#include <86Box/mem.h>
#include <86box/pci.h>
#include <86Box/rom.h> // DEPENDENT!!!
#include <86Box/video.h>
#include <86Box/nv/vid_nv.h>
#include <86Box/nv/vid_nv3.h>

void nv3_pramdac_init()
{
    nv_log("NV3: Initialising PRAMDAC.........");

    // defaults, these come from vbios in reality
    nv3->pramdac.memory_clock_m = nv3->pramdac.pixel_clock_m = 0x03;
    nv3->pramdac.memory_clock_n = nv3->pramdac.pixel_clock_n = 0xc2;
    nv3->pramdac.memory_clock_p = nv3->pramdac.pixel_clock_p = 0x0d;

    nv3_pramdac_set_pixel_clock();
    nv3_pramdac_set_vram_clock();

    nv_log("Done!\n");
}

void nv3_pramdac_set_vram_clock()
{
    int32_t frequency = 14318000.0f;

    // prevent division by 0
    if (nv3->pramdac.memory_clock_m == 0)
        nv3->pramdac.memory_clock_m == 1;
    else
        frequency = (frequency * nv3->pramdac.memory_clock_n) / (1 << nv3->pramdac.memory_clock_p) / nv3->pramdac.memory_clock_n;

    nv_log("NV3: Memory clock = %.2f MHz\n", frequency / 1000000.0f);    
}

void nv3_pramdac_set_pixel_clock()
{
        // frequency divider algorithm from old varcem riva driver,
        // verified by reversing NT drivers v1.29 CalcMNP [symbols] function

        // todo: actually implement it

        // missing section
        // if (nv3->pfb.vram_size == NV3_PFB_BOOT_0_VRAM_SIZE_8MB)
        // {
        //      freq = 13500000.0f;
        // }
        // else 
        //
        // {
        //      freq = 14318000.0f;
        // }

        int32_t frequency = 14318000.0f;

        // prevent division by 0
        if (nv3->pramdac.pixel_clock_m == 0)
            nv3->pramdac.pixel_clock_m == 1;
        else
            frequency = (frequency * nv3->pramdac.pixel_clock_n) / (1 << nv3->pramdac.pixel_clock_p) / nv3->pramdac.pixel_clock_n;

        nv3->nvbase.svga.clock = cpuclock / frequency;

        nv_log("NV3: Pixel clock = %.2f MHz\n", frequency / 1000000.0f);
}