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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <86Box/86box.h>
#include <86Box/device.h>
#include <86Box/mem.h>
#include <86Box/rom.h> // DEPENDENT!!!
#include <86Box/video.h>
#include <86Box/nv/vid_nv.h>

nv3_device_t* nv3;

void* nv3_init(const device_t *info)
{
    nv3 = (nv3_device_t*)calloc(1, sizeof(nv3_device_t));
    //int ret;

    // ELSA VICTORY Erazor Ver. 1.55.00    [WD/VBE30/DDC2B/DPMS]
    //ret = bios_load_linear("roms/video/nvidia/nv3/Ver15500_.rv", 0xC000, 32768, 0); // TODO: HASH BASED!
    rom_init(&nv3->nvbase.vbios, "roms/video/nvidia/nv3/Ver15500_.rv", 0xC000, 32768, 0x7fff, 0, MEM_MAPPING_EXTERNAL);
}

void nv3_close(void* priv)
{
    free(nv3);
}

void nv3_speed_changed(void *priv)
{

}

void nv3_force_redraw(void* priv)
{

}

const device_t nv3_device_pci = 
{
    .name = "NVidia RIVA 128 (NV3) PCI",
    .internal_name = "nv3",
    .flags = DEVICE_PCI,
    .local = 0,
    .init = nv3_init,
    .close = nv3_close,
    .speed_changed = nv3_speed_changed,
    .force_redraw = nv3_force_redraw
};

const device_t nv3_device_agp = 
{
    .name = "NVidia RIVA 128 (NV3) AGP",
    .internal_name = "nv3_agp",
    .flags = DEVICE_AGP,
    .local = 0,
    .init = nv3_init,
    .close = nv3_close,
    .speed_changed = nv3_speed_changed,
    .force_redraw = nv3_force_redraw
};