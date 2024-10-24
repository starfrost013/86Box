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
 *          Notes:
 *             xfree86 ref has INVERTED bit numbering? What?
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
#include <86box/io.h>
#include <86box/pci.h>
#include <86Box/rom.h> // DEPENDENT!!!
#include <86Box/video.h>
#include <86Box/nv/vid_nv.h>
#include <86Box/nv/vid_nv3.h>

nv3_t* nv3;

// Prototypes for functions only used in this translation unit
void nv3_init_mappings_mmio();
void nv3_init_mappings_svga();
void nv3_shutdown_mappings_mmio();
void nv3_shutdown_mappings_svga();

uint8_t nv3_svga_in(uint16_t addr, void* priv);
void nv3_svga_out(uint16_t addr, uint8_t val, void* priv);

// Read 8-bit MMIO
uint8_t nv3_mmio_read8(uint32_t addr, void* priv)
{
    nv_log("NV3: nv3_mmio_read8 0x%04x\n", addr);

    return 0x00;
}

// Read 16-bit MMIO
uint16_t nv3_mmio_read16(uint32_t addr, void* priv)
{
    nv_log("NV3: nv3_mmio_read16 0x%04x\n", addr);

    return 0x00;
}

// Read 32-bit MMIO
uint32_t nv3_mmio_read32(uint32_t addr, void* priv)
{
    nv_log("NV3: nv3_mmio_read32 0x%04x\n", addr);

    return 0x00;
}

// Write 8-bit MMIO
void nv3_mmio_write8(uint32_t addr, uint8_t val, void* priv)
{
    nv_log("NV3: nv3_mmio_write8 0x%04x val=%04x\n", addr, val);
}

// Write 16-bit MMIO
void nv3_mmio_write16(uint32_t addr, uint16_t val, void* priv)
{
    nv_log("NV3: nv3_mmio_write16 0x%04x val=%04x\n", addr, val);
}

// Write 32-bit MMIO
void nv3_mmio_write32(uint32_t addr, uint32_t val, void* priv)
{
    nv_log("NV3: nv3_mmio_write32 0x%04x val=%04x\n", addr, val);
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
        case NV3_PCI_CFG_VENDOR_ID:
            ret = (PCI_VENDOR_SGS_NV & 0xFF);
            break;
        case NV3_PCI_CFG_VENDOR_ID+1: // all access 8bit
            ret = (PCI_VENDOR_SGS_NV >> 8);
            break;
        // Subsystem ID
        case NV3_PCI_CFG_DEVICE_ID:
            ret = (PCI_DEVICE_NV3 & 0xFF);
            break;
        case NV3_PCI_CFG_DEVICE_ID+1:
            ret = (PCI_DEVICE_NV3 >> 8);
            break;
        // various capabilities
        // IO space         enabled
        // Memory space     enabled
        // Bus master       enabled
        // Write/inval      enabled
        // Pal snoop        enabled
        // Capabiliies list enabled
        // 66Mhz FSB        capable

        case 0x04 ... 0x07:
            ret = 0xFF;
            break;
        case NV3_PCI_CFG_REVISION:
            ret = NV3_PCI_CFG_REVISION_B00; // Commercial release
            break;
        case NV3_PCI_CFG_ENABLE_VBIOS:
            ret = nv3->pci_config.vbios_enabled;
            break;
        default: // by default just return pci_config.pci_regs
            ret = nv3->pci_config.pci_regs[addr];
            break;
    }

    nv_log("nv3_pci_read func=0x%04x addr=0x%04x ret=0x%04x\n", func, addr, ret);
}

void nv3_pci_write(int32_t func, int32_t addr, uint8_t val, void* priv)
{
    nv_log("nv3_pci_write func=%04x addr=%04x val=%04x", func, addr, val);

    // TOTAL IRRELEVANCY
    nv3->pci_config.pci_regs[addr] = val;

    switch (addr)
    {
        // standard pci command stuff
        case PCI_REG_COMMAND:

            // you have to turn everything off first
            nv3_shutdown_mappings_mmio();
            nv3_shutdown_mappings_svga();

            (val & PCI_COMMAND_IO) ? nv_log("...Enable I/O") : nv_log("...Disable I/O");
            (val & PCI_COMMAND_MEM) ? nv_log("...Enable Memory") : nv_log("...Disable Memory");
            
            // don't call init_svga as this is IO only
            if (val & PCI_COMMAND_IO)
                    io_sethandler(0x03c0, 0x0020, 
                    nv3_svga_in, NULL, NULL, 
                    nv3_svga_out, NULL, NULL, 
                    nv3);

            if (val & PCI_COMMAND_MEM)
                nv3_init_mappings_mmio();
                nv3_init_mappings_svga();
            
            break;
        case NV3_PCI_CFG_VBIOS_BASE:
        case NV3_PCI_CFG_ENABLE_VBIOS:
            
            // make sure we are actually toggling the vbios, not the rom base
            if (addr == NV3_PCI_CFG_ENABLE_VBIOS)
                nv3->pci_config.vbios_enabled = (val & 0x01);

            if (nv3->pci_config.vbios_enabled)
            {
                // First see if we simply wanted to change the VBIOS location

                if (addr != NV3_PCI_CFG_ENABLE_VBIOS)
                {
                    uint32_t old_addr = nv3->nvbase.vbios.mapping.base;
                    // 9bit register
                    uint32_t new_addr = nv3->pci_config.pci_regs[NV3_PCI_CFG_VBIOS_BASE_H] << 24 |
                    nv3->pci_config.pci_regs[NV3_PCI_CFG_VBIOS_BASE_L] << 16;

                    // move it
                    mem_mapping_set_addr(&nv3->nvbase.vbios.mapping, new_addr, 0x8000);

                    nv_log("...i like to move it move it (VBIOS Relocation) 0x%04x -> 0x%04x\n", old_addr, new_addr);

                }
                else
                {
                    nv_log("...VBIOS Enable");
                    mem_mapping_enable(&nv3->nvbase.vbios.mapping);
                }
            }
            else
            {
                nv_log("...VBIOS Disable");
                mem_mapping_disable(&nv3->nvbase.vbios.mapping);

            }
            break;
    }

    nv_log("\n");
}

void nv3_close(void* priv)
{
    svga_close(&nv3->nvbase.svga);
    free(nv3);
}

void nv3_speed_changed(void* priv)
{
    svga_recalctimings(&nv3->nvbase.svga);
}

// Force Redraw
// Reset etc.
void nv3_force_redraw(void* priv)
{
    nv3->nvbase.svga.fullchange = changeframecount; 
}

//
// SVGA functions
//
void nv3_recalc_timings(svga_t* svga)
{
    nv3_t* nv3 = (nv3_t*)svga->priv;

    // Set the pixel mode
    switch (svga->crtc[NV3_CRTC_REGISTER_PIXELMODE] & 0x03)
    {
        ///0x0 is VGA textmode
        case NV3_CRTC_REGISTER_PIXELMODE_8BPP:
            svga->bpp = 8;
            svga->lowres = 0;
            svga->render = svga_render_8bpp_highres;
            break;
        case NV3_CRTC_REGISTER_PIXELMODE_16BPP:
            svga->bpp = 16;
            svga->lowres = 0;
            svga->render = svga_render_16bpp_highres;
            break;
        case NV3_CRTC_REGISTER_PIXELMODE_32BPP:
            svga->bpp = 32;
            svga->lowres = 0;
            svga->render = svga_render_32bpp_highres;
            break;
    }

    // from nv_riva128
    if (((svga->miscout >> 2) & 2) == 2)
    {
        // set clocks
        nv3_pramdac_set_pixel_clock();
        nv3_pramdac_set_vram_clock();
    }
}

// Read from SVGA core memory
uint8_t nv3_svga_in(uint16_t addr, void* priv)
{
    nv3_t* nv3 = (nv3_t*)priv;

    uint8_t ret = 0x00;

    // If we need to RMA from GPU MMIO, go do that
    if (addr >= 0x3d0
    && addr <= 0x3d3)
    {
        if (!(nv3->pbus.rma.mode & 0x01))
            return ret;

        // must be dword aligned
        uint32_t real_rma_read_addr = addr + ((nv3->pbus.rma.mode & NV3_CRTC_REGISTER_RMA_MODE_MAX - 1) << 1) + (addr & 0x03); 
        ret = nv3_pbus_rma_read(real_rma_read_addr);
        return ret;
    }

    nv_log("nv3_svga_in addr=0x%04x\n", addr);

    // mask off b0/d0 registers 
    if ((((addr & 0xFFF0) == 0x3D0 
    || (addr & 0xFFF0) == 0x3B0) && addr < 0x3de) 
    && !(nv3->nvbase.svga.miscout & 1))
        addr ^= 0x60;

    switch (addr)
    {
        // Alias for "get current SVGA CRTC register ID"
        case 0x3D4:
            ret = nv3->nvbase.svga.crtcreg;
            break;
        case 0x3D5:
            // Support the extended NVIDIA CRTC register range
            switch (nv3->nvbase.svga.crtcreg)
            {
                default:
                    ret = nv3->nvbase.svga.crtc[nv3->nvbase.svga.crtcreg];
            }
            break;
        default:
            ret = svga_in(addr, &nv3->nvbase.svga);
            break;
    }

    return ret; //TEMP
}

// Write to SVGA core memory
void nv3_svga_out(uint16_t addr, uint8_t val, void* priv)
{
    nv_log("nv3_svga_out addr=0x%04x val=0x%04x", addr, val);

    // If we need to RMA to GPU MMIO, go do that
    if (addr >= 0x3d0
    && addr <= 0x3d3)
    {
        // we don't need to store these registers...
        \
        if (!(nv3->pbus.rma.mode & 0x01))
            return;

        uint32_t real_rma_write_addr = addr + ((nv3->pbus.rma.mode & NV3_CRTC_REGISTER_RMA_MODE_MAX - 1) << 1) + (addr & 0x03); 

        nv3_pbus_rma_write(real_rma_write_addr, val);
        return;
    }

    // mask off b0/d0 registers 
    if ((((addr & 0xFFF0) == 0x3D0 || (addr & 0xFFF0) == 0x3B0) 
    && addr < 0x3de) 
    && !(nv3->nvbase.svga.miscout & 1))
        addr ^= 0x60;

    uint8_t crtcreg = nv3->nvbase.svga.crtcreg;
    uint8_t old_value;

    // todo:
    // RMA
    // Pixel formats (8bit vs 555 vs 565)
    // VBE 3.0?
    
    switch (addr)
    {
        case 0x3D4:
            // real mode access to GPU MMIO space...
            nv3->nvbase.svga.crtcreg = val;
            break;
        // support the extended crtc regs and debug this out
        case 0x3D5:

            // Implements the VGA Protect register
            if ((nv3->nvbase.svga.crtcreg < NV3_CRTC_REGISTER_OVERFLOW) && (nv3->nvbase.svga.crtc[0x11] & 0x80))
                return;

            // Ignore certain bits when VGA Protect register set and we are writing to CRTC register=07h
            if ((nv3->nvbase.svga.crtcreg == NV3_CRTC_REGISTER_OVERFLOW) && (nv3->nvbase.svga.crtc[0x11] & 0x80))
                val = (nv3->nvbase.svga.crtc[NV3_CRTC_REGISTER_OVERFLOW] & ~0x10) | (val & 0x10);

            // set the register value...
            nv3->nvbase.svga.crtc[crtcreg] = val;
            // ...now act on it

            if (crtcreg > NV3_CRTC_REGISTER_STANDARDVGA_END)
                nv_log("...Extended CRTC reg=0x%04x", crtcreg);
            else   
                nv_log("...Standard CRTC reg=0x%04x", crtcreg);

            // Handle nvidia extended Bank0/Bank1 IDs
            switch (crtcreg)
            {
                case NV3_CRTC_REGISTER_READ_BANK:
                        nv3->nvbase.cio_read_bank = val;
                        if (nv3->nvbase.svga.chain4) // chain4 addressing (planar?)
                            nv3->nvbase.svga.read_bank = nv3->nvbase.cio_read_bank << 15;
                        else
                            nv3->nvbase.svga.read_bank = nv3->nvbase.cio_read_bank << 13; // extended bank numbers
                    break;
                case NV3_CRTC_REGISTER_WRITE_BANK:
                    nv3->nvbase.cio_write_bank = val;
                        if (nv3->nvbase.svga.chain4)
                            nv3->nvbase.svga.write_bank = nv3->nvbase.cio_write_bank << 15;
                        else
                            nv3->nvbase.svga.write_bank = nv3->nvbase.cio_write_bank << 13;
                    break;
                case NV3_CRTC_REGISTER_RMA:
                    nv3->pbus.rma.mode = val & NV3_CRTC_REGISTER_RMA_MODE_MAX;
                    break;
            }

            break;
        default:
            svga_out(addr, val, &nv3->nvbase.svga);
            break;
    }

    nv_log("\n");
}

void nv3_draw_cursor(svga_t* svga, int32_t drawline)
{
    nv_log("nv3_draw_cursor drawline=0x%04x", drawline);
}

// Initialise the MMIO mappings
void nv3_init_mappings_mmio()
{
    nv_log("NV3: Initialising 32MB MMIO area\n");

    // 0x0 - 1000000: regs
    // 0x1000000-2000000

    // initialize the mmio mapping
    mem_mapping_add(&nv3->nvbase.mmio, 0, 0, 
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

void nv3_init_mappings_svga()
{
    nv_log("NV3: Initialising SVGA core memory mapping\n");

    // setup the svga mappings
    mem_mapping_set(&nv3->nvbase.svga_mapping, 0, 0,
        svga_read_linear,
        svga_readw_linear,
        svga_readl_linear,
        svga_write_linear,
        svga_writew_linear,
        svga_writel_linear,
        NULL, 0, &nv3->nvbase.svga);

    io_sethandler(0x03c0, 0x0020, 
    nv3_svga_in, NULL, NULL, 
    nv3_svga_out, NULL, NULL, 
    nv3);
}

void nv3_shutdown_mappings_mmio()
{
    mem_mapping_disable(&nv3->nvbase.mmio);
}

void nv3_shutdown_mappings_svga()
{
    mem_mapping_disable(&nv3->nvbase.svga_mapping);
    io_removehandler(0x03c0, 0x0020, 
    nv3_svga_in, NULL, NULL, 
    nv3_svga_out, NULL, NULL, 
    nv3);
}

void nv3_init_mappings()
{
    nv3_init_mappings_mmio();
    nv3_init_mappings_svga();
}

// 
// Init code
//
void* nv3_init(const device_t *info)
{
    nv_log("NV3: initialising core\n");

    nv3 = (nv3_t*)calloc(1, sizeof(nv3_t));

    // currently using ELSA VICTORY Erazor    Ver. 1.54.03    [WD/VBE30/DDC2B/DPMS] 
    //                 ELSA VICTORY Erazor    Ver. 1.55.00    [WD/VBE30/DDC2B/DPMS] seems to be broken :(
    
    int32_t err = rom_init(&nv3->nvbase.vbios, NV_VBIOS_V15403, 0xC0000, 0x8000, 0x7fff, 0, MEM_MAPPING_EXTERNAL);
    
    if (err)
            nv_log("NV3: failed to load VBIOS err=%d\n", err);
    else    
            nv_log("NV3: Successfully loaded VBIOS %s\n", NV_VBIOS_V15403);


    // set up the bus and start setting up SVGA core
    if (nv3->nvbase.bus_generation == nv_bus_pci)
    {
        nv_log("NV3: using PCI bus\n");

        pci_add_card(PCI_ADD_NORMAL, nv3_pci_read, nv3_pci_write, NULL, &nv3->nvbase.pci_slot);

        svga_init(&nv3_device_pci, &nv3->nvbase.svga, nv3, VRAM_SIZE_4MB, 
        nv3_recalc_timings, nv3_svga_in, nv3_svga_out, nv3_draw_cursor, NULL);
    }
    else if (nv3->nvbase.bus_generation == nv_bus_agp_1x)
    {
        nv_log("NV3: using AGP 1X bus\n");

        pci_add_card(PCI_ADD_AGP, nv3_pci_read, nv3_pci_write, NULL, &nv3->nvbase.pci_slot);

        svga_init(&nv3_device_agp, &nv3->nvbase.svga, nv3, VRAM_SIZE_4MB, 
        nv3_recalc_timings, nv3_svga_in, nv3_svga_out, nv3_draw_cursor, NULL);
    }

    // init memory mappings
    nv3_init_mappings();

    // svga is done, so now initialise the real gpu
    nv_log("NV3: Initialising GPU core...\n");
    nv3_pramdac_init();

    return nv3;
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