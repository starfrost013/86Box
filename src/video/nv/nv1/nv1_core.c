/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          WELCOME TO NVIDIA DRIVER VERSION 2.0!
 *          nv1_core: Handles device registration, init and shutdown
 *
 * Authors: starfrost
 *
 *          Copyright 2024-2026 starfrost
 */

#include "nv1.h"

/* These are a ****PLACEHOLDER**** and are copied from 3dfx VoodooBanshee/Voodoo3*/
static video_timings_t timing_nv1 = { .type = VIDEO_PCI, .write_b = 2, .write_w = 2, .write_l = 1, .read_b = 20, .read_w = 20, .read_l = 21 };

nv1_t *nv1;

//
// LOGGING stuff
//


void
nv_log_internal(const char *fmt, va_list arg)
{
    if (!nv1->log)
        return;

    log_out(nv1->log, fmt, arg);

    // If our debug config option is configured, full log. Otherwise log with cyclical detection.
    //log_out_cyclic(nv1->log, fmt, arg);
}

void
nv_log(const char *fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    nv_log_internal(fmt, arg);
    va_end(arg);
}

void
nv1_init_mappings()
{
    io_sethandler(NV1_VGA_START, NV1_VGA_SIZE, nv1_svga_read, NULL, NULL, nv1_svga_write, NULL, NULL, nv1);

    mem_mapping_add(&nv1->mapping_mmio, 0, 0,
                    nv1_mmio_read8,
                    nv1_mmio_read16,
                    nv1_mmio_read32,
                    nv1_mmio_write8,
                    nv1_mmio_write16,
                    nv1_mmio_write32,
                    NULL, MEM_MAPPING_EXTERNAL, nv1);

    
    mem_mapping_add(&nv1->mapping_prm, 0, 0,
                    nv1_mmio_read8,
                    nv1_mmio_read16,
                    nv1_mmio_read32,
                    nv1_mmio_write8,
                    nv1_mmio_write16,
                    nv1_mmio_write32,
                    NULL, MEM_MAPPING_EXTERNAL, nv1);
    
    mem_mapping_set_addr(&nv1->mapping_prm, NV1_VGA_RAM_START, 0x1FFFF);
}

// Update the mappings for the VGA
void nv1_update_mappings_vga()
{
    bool io_enabled = nv1->pci_regs_vga[PCI_REG_COMMAND_L] & PCI_COMMAND_IO;
    (io_enabled) ? nv_log("I/O enabled\n") : nv_log("I/O disabled\n");

    // remove to avoid setting multiple times
    io_removehandler(NV1_VGA_START, NV1_VGA_SIZE, nv1_svga_read, NULL, NULL, nv1_svga_write, NULL, NULL, nv1);

    if (io_enabled)
        io_sethandler(NV1_VGA_START, NV1_VGA_SIZE, nv1_svga_read, NULL, NULL, nv1_svga_write, NULL, NULL, nv1);

}

// Update the mappings for the NV1
void nv1_update_mappings_nv()
{
    bool mem_enabled = nv1->pci_regs_nv[PCI_REG_COMMAND_L] & PCI_COMMAND_MEM;
    (mem_enabled) ? nv_log("MMIO enabled\n") : nv_log("MMIO disabled\n");

    mem_mapping_disable(&nv1->mapping_mmio);

    // SET_ADDR enables automatically
    if (mem_enabled
    && nv1->bar0_addr)
    {
        nv_log("BAR0 is now %08x\n", nv1->bar0_addr);
        mem_mapping_set_addr(&nv1->mapping_mmio, nv1->bar0_addr, NV1_MMIO_SIZE);
    }
        
}

// Update the GPU mappings.
void 
nv1_update_mappings(int32_t func)
{
    if (func == NV1_PCI_FUNCTION_VGA)
        nv1_update_mappings_vga();
    else
        nv1_update_mappings_nv();
}

void *
nv1_init(const device_t *dev)
{
    nv1 = calloc(sizeof(nv1_t), 1);
    nv1->log = log_open("NV1");
    
    // initial values
    nv1->pci_int_line = 0xFF;

    nv_log("NV1 Emulation Driver is initialising\n");

    nv1->vram_amount = device_get_config_int("vram_size");
    nv_log("Video RAM is %d bytes\n", nv1->vram_amount);

    // doesn't matter as long as we can start with osmething
    const char *vbios_path = NV1_VBIOS_E3D_3X00;

    nv_log("[Phase 1] Loading Video BIOS at %s\n", vbios_path);

    // Load video bios
    int32_t rom_err = rom_init(&nv1->vbios, vbios_path, NV1_VBIOS_LOCATION, NV1_VBIOS_SIZE, NV1_VBIOS_SIZE - 1,
                               0, MEM_MAPPING_EXTERNAL);

    if (rom_err) {
        nv_log("[Phase 1] Error %d!\n", rom_err);
        return NULL;
    } else
        nv_log("[Phase 1] OK!\n");

    pci_add_card(PCI_ADD_NORMAL, nv1_pci_read, nv1_pci_write, nv1, &nv1->pci_slot);

    nv_log("[Phase 2] Initialising PCI OK!\n");

    // tell the video subsystem we have an SVGA class card
    svga_init(&nv1_device, &nv1->svga, nv1, nv1->vram_amount,
              nv1_recalc_timings, nv1_svga_read, nv1_svga_write, nv1_draw_cursor, NULL);

    video_inform(VIDEO_FLAG_TYPE_SPECIAL, &timing_nv1);

    nv1->svga.ramdac = device_add(&stg1764_ramdac_device);
    nv1->svga.clock_gen = nv1->svga.ramdac;

    nv_log("[Phase 3] Initialising SVGA OK! [RAMDAC = STG1764]\n");

    // initialise mappings
    nv1_init_mappings(); 

    nv_log("[Phase 4] Initialising memory mappings OK! [32 MB MMIO, 32 I/O addresses for VGA]\n");

    // initialise gpu subsystems
    nv1_prmc_init();

    nv_log("[Phase 5] Initialising GPU subsystems OK! ALL INIT OK!\n");

    return nv1;
}

void
nv1_speed_changed(void *priv)
{
    svga_recalctimings(&nv1->svga);
}

void
nv1_draw_cursor(svga_t *svga, int32_t drawline)
{
}

void
nv1_recalc_timings(svga_t *svga)
{
    //svga_recalctimings(svga);
}

void
nv1_force_redraw(void *priv)
{
    nv1->svga.fullchange = changeframecount;
}

// See if the bios rom is available.
int32_t
nv1_available(void)
{
    return (rom_present(NV1_VBIOS_E3D_3X00));
}

void
nv1_close(void *priv)
{
    log_close(nv1->log);
    svga_close(&nv1->svga);
}

// NV3T (RIVA 128 ZX)
// PCI
// 8MB VRAM
const device_t nv1_device = {
    .name          = "NVIDIA NV1 [Diamond Edge 3D 3400] [Not Direct3D Compatible]",
    .internal_name = "nv1",
    .flags         = DEVICE_PCI,
    .local         = 0,
    .init          = nv1_init,
    .close         = nv1_close,
    .speed_changed = nv1_speed_changed,
    .force_redraw  = nv1_force_redraw,
    .available     = nv1_available,
    .config        = nv1_config,
};
