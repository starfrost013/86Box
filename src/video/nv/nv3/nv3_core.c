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
#include <86box/pci.h>
#include <86Box/rom.h> // DEPENDENT!!!
#include <86Box/video.h>
#include <86Box/nv/vid_nv.h>

nv3_t* nv3;

#define MMIO_SIZE       0x1000000
#define NV_VBIOS_V15403 "roms/video/nvidia/nv3/VCERAZOR.BIN" //TODO: move to hash system

// Read 8-bit MMIO
uint8_t nv3_mmio_read8(uint32_t addr, void* priv)
{

}

// Read 16-bit MMIO
uint16_t nv3_mmio_read16(uint32_t addr, void* priv)
{

}

// Read 32-bit MMIO
uint32_t nv3_mmio_read32(uint32_t addr, void* priv)
{

}

// Write 8-bit MMIO
void nv3_mmio_write8(uint32_t addr, uint8_t val, void* priv)
{

}

// Write 16-bit MMIO
void nv3_mmio_write16(uint32_t addr, uint16_t val, void* priv)
{
    
}

// Write 32-bit MMIO
void nv3_mmio_write32(uint32_t addr, uint32_t val, void* priv)
{
    
}

void nv3_init_mmio()
{
    nv_log("NV3: Initialising 32MB MMIO area\n");

    // 0x0 - 1000000: regs
    // 0x1000000-2000000

    // initialize the mmio mapping
    mem_mapping_add(&nv3->nvbase.mmio, 0, MMIO_SIZE, 
    nv3_mmio_read8,
    nv3_mmio_read16,
    nv3_mmio_read32,
    nv3_mmio_write8,
    nv3_mmio_write16,
    nv3_mmio_write32,
    NULL,
    MEM_MAPPING_EXTERNAL,
    nv3);
}

// PCI

uint8_t nv3_pci_read(int32_t func, int32_t addr, void* priv)
{
    uint8_t ret = 0x00;

    // figure out what size this gets read as first
    // seems func does not matter at least here?
    switch (addr) 
    {
        // Get the pci vendor id..
        case 0x00:
            ret = (PCI_VENDOR_SGS_NV & 0xFF);
            break;
        case 0x01:
            ret = (PCI_VENDOR_SGS_NV >> 8);
            break;
        case 0x02:
            ret = 0x00;
            break;
        case 0x03:
            ret = NV_ARCHITECTURE_NV3;
            break;
    }

    nv_log("nv3_pci_read func=0x%04x addr=0x%04x ret=0x%04x\n", func, addr, ret);
}

void nv3_pci_write(int32_t func, int32_t addr, uint8_t val, void* priv)
{
    nv_log("nv3_pci_write func=%04x addr=%04x val=%04x\n", func, addr, val);
}

void* nv3_init(const device_t *info)
{
    nv_log("NV3: initialising core\n");

    nv3 = (nv3_t*)calloc(1, sizeof(nv3_t));
    //int ret;

    // currently using ELSA VICTORY Erazor    Ver. 1.54.03    [WD/VBE30/DDC2B/DPMS] 
    // ELSA VICTORY Erazor    Ver. 1.55.00    [WD/VBE30/DDC2B/DPMS] seems to be broken :(
    
    int32_t err = rom_init(&nv3->nvbase.vbios, NV_VBIOS_V15403, 0xC0000, 32768, 0x7fff, 0, MEM_MAPPING_EXTERNAL);
    
    if (err)
            nv_log("NV3: failed to load VBIOS err=%d\n", err);
    else    
            nv_log("NV3: Successfully loaded VBIOS %s\n", NV_VBIOS_V15403);

    if (nv3->nvbase.bus_generation == nv_bus_pci)
    {
        nv_log("NV3: using PCI bus\n");

        pci_add_card(PCI_ADD_NORMAL, nv3_pci_read, nv3_pci_write, NULL, &nv3->nvbase.pci_slot);
    }
    else if (nv3->nvbase.bus_generation == nv_bus_agp_1x)
    {
        nv_log("NV3: using AGP 1X bus\n");

        pci_add_card(PCI_ADD_AGP, nv3_pci_read, nv3_pci_write, NULL, &nv3->nvbase.pci_slot);

    }

    nv3_init_mmio();

    return nv3;
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

// NV3 (RIVA 128)
// PCI
// 2MB or 4MB VRAM
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

// NV3 (RIVA 128)
// AGP
// 2MB or 4MB VRAM
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