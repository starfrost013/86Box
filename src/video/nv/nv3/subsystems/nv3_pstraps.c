/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          NV3 PSTRAPS - External Devices
 *                        Including straps
 *
 *
 *
 * Authors: Connor Hyde, <mario64crashed@gmail.com> I need a better email address ;^)
 *
 *          Copyright 2024-2025 starfrost
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <86box/86box.h>
#include <86box/device.h>
#include <86box/mem.h>
#include <86box/pci.h>
#include <86box/rom.h> // DEPENDENT!!!
#include <86box/video.h>
#include <86box/nv/vid_nv.h>
#include <86box/nv/vid_nv3.h>

void nv3_pstraps_init(void)
{
    nv_log("Initialising straps....\n");

    // Set the chip straps (Make these configurable in the future...)
    // Current settings: AGP 2X disabled, TV Mode NTSC, Crystal 13.5Mhz, 128-bit bus width (cheaper models used 64bit buses), 
    // BIOS present, 66Mhz bus speed (PCI)

    nv3->straps = (NV3_PSTRAPS_AGP2X_DISABLED << NV3_PSTRAPS_AGP2X) |
    (NV3_PSTRAPS_TVMODE_NTSC << NV3_PSTRAPS_TVMODE) |
    (NV3_PSTRAPS_CRYSTAL_13500K << NV3_PSTRAPS_CRYSTAL);

    // figure out the bus
    if (nv3->nvbase.bus_generation == nv_bus_pci)
        nv3->straps |= (NV3_PSTRAPS_BUS_TYPE_PCI << NV3_PSTRAPS_BUS_TYPE);
    else
        nv3->straps |= (NV3_PSTRAPS_BUS_TYPE_AGP << NV3_PSTRAPS_BUS_TYPE);

    // now the lower bits 
    nv3->straps |= (NV3_PSTRAPS_BUS_WIDTH_128BIT << NV3_PSTRAPS_BUS_WIDTH) |
    (NV3_PSTRAPS_BIOS_PRESENT << NV3_PSTRAPS_BIOS) |
    (NV3_PSTRAPS_BUS_SPEED_66MHZ << NV3_PSTRAPS_BUS_SPEED);

    nv_log("Straps = 0x%04x\n", nv3->straps);
    nv_log("Initialising PSTRAPS: Done\n");
}

//
// ****** Read/Write functions start ******
//

uint32_t nv3_pstraps_read(uint32_t address) 
{ 
    return nv3->straps;
}

void nv3_pstraps_write(uint32_t address, uint32_t val) 
{
    /* For some reason, all RIVA 128 ZX VBIOSes try to write to the straps. So only indicate this as a problem and return on Rev A/B */
    if (nv3->nvbase.gpu_revision != NV3_PCI_CFG_REVISION_C00)
    {
        warning("Huh? Tried to write to the straps (val=%d). Something is wrong...\n", nv3->straps);
        return;
    }
    else
    {
        if (nv3->straps & NV3_PSTRAPS_OVERWRITE_ENABLED)
            nv3->straps = val; 
    }
}