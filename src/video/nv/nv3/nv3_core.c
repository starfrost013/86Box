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

// All MMIO regs are 32-bit i believe internally
// so we have to do some munging to get this to read

// Read 8-bit MMIO
uint8_t nv3_mmio_read8(uint32_t addr, void* priv)
{
    // see if unaligned reads are a problem
    uint32_t ret = nv3_mmio_read32(addr, priv);
    return (uint8_t)(ret >> ((addr & 3) << 3) & 0xFF);
}

// Read 16-bit MMIO
uint16_t nv3_mmio_read16(uint32_t addr, void* priv)
{
    uint32_t ret = nv3_mmio_read32(addr, priv);
    return (uint8_t)(ret >> ((addr & 3) << 3) & 0xFFFF);
}

// Read 32-bit MMIO
uint32_t nv3_mmio_read32(uint32_t addr, void* priv)
{
    return nv3_mmio_arbitrate_read(addr);
}

// Write 8-bit MMIO
void nv3_mmio_write8(uint32_t addr, uint8_t val, void* priv)
{
    // overwrite first 8bits of a 32 bit value
    uint32_t new_val = nv3_mmio_read32(addr, NULL);

    new_val &= (~0xFF << (addr & 3) << 3);
    new_val |= (val << ((addr & 3) << 3));

    nv3_mmio_write32(addr, new_val, priv);
}

// Write 16-bit MMIO
void nv3_mmio_write16(uint32_t addr, uint16_t val, void* priv)
{
    // overwrite first 16bits of a 32 bit value
    uint32_t new_val = nv3_mmio_read32(addr, NULL);

    new_val &= (~0xFFFF << (addr & 3) << 3);
    new_val |= (val << ((addr & 3) << 3));

    nv3_mmio_write32(addr, new_val, priv);
}

// Write 32-bit MMIO
void nv3_mmio_write32(uint32_t addr, uint32_t val, void* priv)
{
    nv3_mmio_arbitrate_write(addr, val);
}

// PCI stuff
// BAR0         Pointer to MMIO space
// BAR1         Pointer to Linear Framebuffer (NV_USER)

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
        
        case NV3_PCI_CFG_VENDOR_ID + 1: // all access 8bit
            ret = (PCI_VENDOR_SGS_NV >> 8);
            break;

        // device id

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

        case PCI_REG_COMMAND_L:
            ret = nv3->pci_config.pci_regs[PCI_REG_COMMAND_L]; // we actually respond to the fucking 
            break;
        
        case PCI_REG_COMMAND_H:
            ret = nv3->pci_config.pci_regs[PCI_REG_COMMAND_H] | NV3_PCI_COMMAND_H_FAST_BACK2BACK; // always enable fast back2back
            break;

        // pci status register
        case PCI_REG_STATUS_L:
            if (nv3->pextdev.straps 
            & NV3_PSTRAPS_BUS_SPEED_66MHZ)
                ret = (nv3->pci_config.pci_regs[PCI_REG_STATUS_L] | NV3_PCI_STATUS_L_66MHZ_CAPABLE);
            else
                ret = nv3->pci_config.pci_regs[PCI_REG_STATUS_L];

            break;

        case PCI_REG_STATUS_H:
            ret = (nv3->pci_config.pci_regs[PCI_REG_STATUS_H]) & (NV3_PCI_STATUS_H_FAST_DEVSEL_TIMING << NV3_PCI_STATUS_H_DEVSEL_TIMING);
            break;
        
        case NV3_PCI_CFG_REVISION:
            ret = NV3_PCI_CFG_REVISION_B00; // Commercial release
            break;
       
        case PCI_REG_PROG_IF:
            ret = 0x00;
            break;
            
        case NV3_PCI_CFG_SUBCLASS_CODE:
            ret = 0x00; // nothing
            break;
        
        case NV3_PCI_CFG_CLASS_CODE:
            ret = NV3_PCI_CFG_CLASS_CODE_VGA; // CLASS_CODE_VGA 
            break;
        
        case NV3_PCI_CFG_CACHE_LINE_SIZE:
            ret = NV3_PCI_CFG_CACHE_LINE_SIZE_DEFAULT_FROM_VBIOS;
            break;
        
        case NV3_PCI_CFG_LATENCY_TIMER:
        case NV3_PCI_CFG_HEADER_TYPE:
        case NV3_PCI_CFG_BIST:
            ret = 0x00;
            break;

        // BARs are marked as prefetchable per the datasheet
        case NV3_PCI_CFG_BAR0_L:
        case NV3_PCI_CFG_BAR1_L:
            // only bit that matters is bit 3 (prefetch bit)
            ret =(NV3_PCI_CFG_BAR_PREFETCHABLE_ENABLED << NV3_PCI_CFG_BAR_PREFETCHABLE);
            break;

        // These registers are hardwired to zero per the datasheet
        // Writes have no effect, we can just handle it here though
        case NV3_PCI_CFG_BAR0_BYTE1 ... NV3_PCI_CFG_BAR0_BYTE2:
        case NV3_PCI_CFG_BAR1_BYTE1 ... NV3_PCI_CFG_BAR1_BYTE2:
            ret = 0x00;
            break;

        // MMIO base address
        case NV3_PCI_CFG_BAR0_BASE_ADDRESS:
            ret = nv3->nvbase.bar0_mmio_base >> 24;//8bit value
            break; 

        case NV3_PCI_CFG_BAR1_BASE_ADDRESS:
            ret = nv3->nvbase.bar1_lfb_base >> 24; //8bit value
            break;

        case NV3_PCI_CFG_ENABLE_VBIOS:
            ret = nv3->pci_config.vbios_enabled;
            break;
        
        case NV3_PCI_CFG_INT_LINE:
            ret = nv3->pci_config.int_line;
            break;
        
        case NV3_PCI_CFG_INT_PIN:
            ret = PCI_INTA;
            break;

        case NV3_PCI_CFG_MIN_GRANT:
            ret = NV3_PCI_CFG_MIN_GRANT_DEFAULT;
            break;

        case NV3_PCI_CFG_MAX_LATENCY:
            ret = NV3_PCI_CFG_MAX_LATENCY_DEFAULT;
            break;

        //bar2-5 are not used and hardwired to 0
        case NV3_PCI_CFG_BAR_INVALID_START ... NV3_PCI_CFG_BAR_INVALID_END:
            ret = 0x00;
            break;
            
        case NV3_PCI_CFG_SUBSYSTEM_ID_MIRROR_START:
        case NV3_PCI_CFG_SUBSYSTEM_ID_MIRROR_END:
            ret = nv3->pci_config.pci_regs[NV3_PCI_CFG_SUBSYSTEM_ID + (addr & 0x03)];
            break;

        default: // by default just return pci_config.pci_regs
            ret = nv3->pci_config.pci_regs[addr];
            break;
        
    }

    nv_log("NV3: nv3_pci_read func=0x%04x addr=0x%04x ret=0x%04x\n", func, addr, ret);
    return ret; 
}

void nv3_pci_write(int32_t func, int32_t addr, uint8_t val, void* priv)
{

    // TOTAL IRRELEVANCY

    // some addresses are not writable so can't have any effect and can't be allowed to be modified using this code
    // only the most significant byte of the PCI BARs can be modified
    if (addr >= NV3_PCI_CFG_BAR0_L && addr <= NV3_PCI_CFG_BAR0_BYTE2
    && addr >= NV3_PCI_CFG_BAR1_L && addr <= NV3_PCI_CFG_BAR1_BYTE2)
        return;

    nv_log("NV3: nv3_pci_write func=0x%04x addr=0x%04x val=0x%04x\n", func, addr, val);

    nv3->pci_config.pci_regs[addr] = val;

    switch (addr)
    {
        // standard pci command stuff
        case PCI_REG_COMMAND_L:
            nv3->pci_config.pci_regs[PCI_REG_COMMAND_L] = val;
            // actually update the mappings
            nv3_update_mappings();
            break;
        // pci status register
        case PCI_REG_STATUS_L:
            nv3->pci_config.pci_regs[PCI_REG_STATUS_L] = val | (NV3_PCI_STATUS_L_66MHZ_CAPABLE);
            break;
        case PCI_REG_STATUS_H:
            nv3->pci_config.pci_regs[PCI_REG_STATUS_H] = val | (NV3_PCI_STATUS_H_FAST_DEVSEL_TIMING << NV3_PCI_STATUS_H_DEVSEL_TIMING);
            break;
        //TODO: ACTUALLY REMAP THE MMIO AND NV_USER
        case NV3_PCI_CFG_BAR0_BASE_ADDRESS:
            nv3->nvbase.bar0_mmio_base = val << 24;
            nv3_update_mappings();
            break; 
        case NV3_PCI_CFG_BAR1_BASE_ADDRESS:
            nv3->nvbase.bar1_lfb_base = val << 24;
            nv3_update_mappings();
            break;
        case NV3_PCI_CFG_ENABLE_VBIOS:
        case NV3_PCI_CFG_VBIOS_BASE:
            
            // make sure we are actually toggling the vbios, not the rom base
            if (addr == NV3_PCI_CFG_ENABLE_VBIOS)
                nv3->pci_config.vbios_enabled = (val & 0x01);

            if (nv3->pci_config.vbios_enabled)
            {
                // First see if we simply wanted to change the VBIOS location

                // Enable it in case it was disabled before
                mem_mapping_enable(&nv3->nvbase.vbios.mapping);

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
                    nv_log("...VBIOS Enable\n");
                }
            }
            else
            {
                nv_log("...VBIOS Disable\n");
                mem_mapping_disable(&nv3->nvbase.vbios.mapping);

            }
            break;
        case NV3_PCI_CFG_INT_LINE:
            nv3->pci_config.int_line = val;
            break;
        //bar2-5 are not used and can't be written to
        case NV3_PCI_CFG_BAR_INVALID_START ... NV3_PCI_CFG_BAR_INVALID_END:
            break;

        // these are mirrored to the subsystem id and also stored in the ROMBIOS
        case NV3_PCI_CFG_SUBSYSTEM_ID_MIRROR_START:
        case NV3_PCI_CFG_SUBSYSTEM_ID_MIRROR_END:
            nv3->pci_config.pci_regs[NV3_PCI_CFG_SUBSYSTEM_ID + (addr & 0x03)] = val;
            break;

        default:

    }
}

void nv3_close(void* priv)
{
    svga_close(&nv3->nvbase.svga);
    free(nv3);
}


//
// SVGA functions
//
void nv3_recalc_timings(svga_t* svga)
{
    nv3_t* nv3 = (nv3_t*)svga->priv;

    svga->ma_latch += (svga->crtc[NV3_CRTC_REGISTER_RPC0] & 0x1F) << 16;
    svga->rowoffset += (svga->crtc[NV3_CRTC_REGISTER_RPC0] & 0xE0) << 3;

    // should these actually use separate values?
    // i don't we should force the top 2 bits to 1...

    // required for VESA resolutions, force parameters higher
    if (svga->crtc[NV3_CRTC_REGISTER_PIXELMODE] & 1 << (NV3_CRTC_REGISTER_FORMAT_VDT10)) svga->vtotal += 0x400;
    if (svga->crtc[NV3_CRTC_REGISTER_PIXELMODE] & 1 << (NV3_CRTC_REGISTER_FORMAT_VDE10)) svga->dispend += 0x400;
    if (svga->crtc[NV3_CRTC_REGISTER_PIXELMODE] & 1 << (NV3_CRTC_REGISTER_FORMAT_VRS10)) svga->vblankstart += 0x400;
    if (svga->crtc[NV3_CRTC_REGISTER_PIXELMODE] & 1 << (NV3_CRTC_REGISTER_FORMAT_VBS10)) svga->vsyncstart += 0x400;
    if (svga->crtc[NV3_CRTC_REGISTER_PIXELMODE] & 1 << (NV3_CRTC_REGISTER_FORMAT_HBE6)) svga->hdisp += 0x400;  

    if (svga->crtc[NV3_CRTC_REGISTER_HEB] & 0x01)
        svga->hdisp += 0x100; // large screen bit

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

void nv3_speed_changed(void* priv)
{
    nv3_recalc_timings(&nv3->nvbase.svga);
}

// Force Redraw
// Reset etc.
void nv3_force_redraw(void* priv)
{
    nv3->nvbase.svga.fullchange = changeframecount; 
}

// Read from SVGA core memory
uint8_t nv3_svga_in(uint16_t addr, void* priv)
{
    nv3_t* nv3 = (nv3_t*)priv;

    uint8_t ret = 0x00;

    // If we need to RMA from GPU MMIO, go do that
    if (addr >= NV3_RMA_REGISTER_START
    && addr <= NV3_RMA_REGISTER_END)
    {
        if (!(nv3->pbus.rma.mode & 0x01))
            return ret;

        // must be dword aligned
        uint32_t real_rma_read_addr = ((nv3->pbus.rma.mode & NV3_CRTC_REGISTER_RMA_MODE_MAX - 1) << 1) + (addr & 0x03); 
        ret = nv3_pbus_rma_read(real_rma_read_addr);
        return ret;
    }

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

    // If we need to RMA to GPU MMIO, go do that
    if (addr >= NV3_RMA_REGISTER_START
    && addr <= NV3_RMA_REGISTER_END)
    {
        // we don't need to store these registers...
        nv3->pbus.rma.rma_regs[addr & 3] = val;

        if (!(nv3->pbus.rma.mode & 0x01)) // we are halfway through sending something
            return;

        uint32_t real_rma_write_addr = ((nv3->pbus.rma.mode & (NV3_CRTC_REGISTER_RMA_MODE_MAX - 1)) << 1) + (addr & 0x03); 

        nv3_pbus_rma_write(real_rma_write_addr, nv3->pbus.rma.rma_regs[addr & 3]);
        return;
    }

    // mask off b0/d0 registers 
    if ((((addr & 0xFFF0) == 0x3D0 || (addr & 0xFFF0) == 0x3B0) 
    && addr < 0x3de) 
    && !(nv3->nvbase.svga.miscout & 1))//miscout bit 7 controls mappping
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
    mem_mapping_add(&nv3->nvbase.mmio_mapping, 0, 0, 
        nv3_mmio_read8,
        nv3_mmio_read16,
        nv3_mmio_read32,
        nv3_mmio_write8,
        nv3_mmio_write16,
        nv3_mmio_write32,
        NULL, MEM_MAPPING_EXTERNAL, nv3);
    
    // initialize the mmio mapping
    mem_mapping_add(&nv3->nvbase.ramin_mapping, 0, 0, 
        nv3_ramin_read8,
        nv3_ramin_read16,
        nv3_ramin_read32,
        nv3_ramin_write8,
        nv3_ramin_write16,
        nv3_ramin_write32,
        NULL, MEM_MAPPING_EXTERNAL, nv3);

    
    mem_mapping_add(&nv3->nvbase.ramin_mapping_mirror, 0, 0,
        nv3_ramin_read8,
        nv3_ramin_read16,
        nv3_ramin_read32,
        nv3_ramin_write8,
        nv3_ramin_write16,
        nv3_ramin_write32,
        NULL, MEM_MAPPING_EXTERNAL, nv3);


}

void nv3_init_mappings_svga()
{
    nv_log("NV3: Initialising SVGA core memory mapping\n");

    // setup the svga mappings
    mem_mapping_set(&nv3->nvbase.framebuffer_mapping, 0, 0,
        svga_read_linear,
        svga_readw_linear,
        svga_readl_linear,
        svga_write_linear,
        svga_writew_linear,
        svga_writel_linear,
        NULL, 0, &nv3->nvbase.svga);

    // the SVGA/LFB mapping is also mirrored
    mem_mapping_set(&nv3->nvbase.framebuffer_mapping_mirror, 0, 0, 
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

void nv3_init_mappings()
{
    nv3_init_mappings_mmio();
    nv3_init_mappings_svga();
}

// Updates the mappings after initialisation. 
void nv3_update_mappings()
{

    // setting this to 0 doesn't seem to disable it, based on the datasheet

    nv_log("\nMemory Mapping Config Change:\n");

    (nv3->pci_config.pci_regs[PCI_REG_COMMAND] & PCI_COMMAND_IO) ? nv_log("Enable I/O\n") : nv_log("Disable I/O\n");

    io_removehandler(0x03c0, 0x0020, 
        nv3_svga_in, NULL, NULL, 
        nv3_svga_out, NULL, NULL, 
        nv3);

    if (nv3->pci_config.pci_regs[PCI_REG_COMMAND] & PCI_COMMAND_IO)
        io_sethandler(0x03c0, 0x0020, 
        nv3_svga_in, NULL, NULL, 
        nv3_svga_out, NULL, NULL, 
        nv3);   
    
    // turn off bar0 and bar1 by defualt
    mem_mapping_disable(&nv3->nvbase.mmio_mapping);
    mem_mapping_disable(&nv3->nvbase.framebuffer_mapping);
    mem_mapping_disable(&nv3->nvbase.framebuffer_mapping_mirror);
    mem_mapping_disable(&nv3->nvbase.ramin_mapping);
    mem_mapping_disable(&nv3->nvbase.ramin_mapping_mirror);

    if (!(nv3->pci_config.pci_regs[PCI_REG_COMMAND]) & PCI_COMMAND_MEM)
    {
        nv_log("NV3: The memory was turned off, not much is going to happen.\n");
        return;
    }

    mem_mapping_enable(&nv3->nvbase.mmio_mapping);
    mem_mapping_enable(&nv3->nvbase.framebuffer_mapping);
    mem_mapping_enable(&nv3->nvbase.framebuffer_mapping_mirror);
    mem_mapping_enable(&nv3->nvbase.ramin_mapping);
    mem_mapping_enable(&nv3->nvbase.ramin_mapping_mirror);

    // first map bar0

    nv_log("NV3: BAR0 (MMIO Base) = 0x%08x\n", nv3->nvbase.bar0_mmio_base);

    //mem_mapping_enable(&nv3->nvbase.mmio_mapping); // should have no effect if already enabled

    mem_mapping_set_addr(&nv3->nvbase.mmio_mapping, nv3->nvbase.bar0_mmio_base, NV3_MMIO_SIZE);


    // if this breaks anything, remove it
    // skeptical that 0 is used to disable...
    nv_log("NV3: BAR1 (Linear Framebuffer / NV_USER Base & RAMIN) = 0x%08x\n", nv3->nvbase.bar1_lfb_base);

    // this is likely mirrored 
    // 4x on 2mb cards
    // 2x on 4mb cards
    // and not at all on 8mb
    mem_mapping_enable(&nv3->nvbase.framebuffer_mapping);
    mem_mapping_enable(&nv3->nvbase.framebuffer_mapping_mirror);
    mem_mapping_enable(&nv3->nvbase.ramin_mapping);
    mem_mapping_enable(&nv3->nvbase.ramin_mapping_mirror);

    mem_mapping_set_addr(&nv3->nvbase.framebuffer_mapping, nv3->nvbase.bar1_lfb_base, NV3_MMIO_SIZE);
    // 4MB VRAM memory map:
    // LFB_BASE+VRAM_SIZE=RAMIN Mirror(?)                                                   0x1400000 (VERIFY PCBOX)
    // LFB_BASE+VRAM_SIZE*2=LFB Mirror(?)                                                   0x1800000            
    // LFB_BASE+VRAM_SIZE*3=Definitely RAMIN (then it ends, the total ram space is 16mb)    0x1C00000
    mem_mapping_set_addr(&nv3->nvbase.ramin_mapping_mirror, nv3->nvbase.bar1_lfb_base + VRAM_SIZE_4MB, VRAM_SIZE_4MB);
    mem_mapping_set_addr(&nv3->nvbase.framebuffer_mapping_mirror, nv3->nvbase.bar1_lfb_base + (VRAM_SIZE_4MB * 2), VRAM_SIZE_4MB);
    mem_mapping_set_addr(&nv3->nvbase.ramin_mapping, nv3->nvbase.bar1_lfb_base + (VRAM_SIZE_4MB * 3), VRAM_SIZE_4MB);
            // TODO: RAMIN and its mirror

    // Did we change the banked SVGA mode?
    switch (nv3->nvbase.svga.gdcreg[0x06] & 0x0c)
    {
        case NV3_CRTC_BANKED_128K_A0000:
            nv_log("NV3: SVGA Banked Mode = 128K @ A0000h\n");
            mem_mapping_set_addr(&nv3->nvbase.svga.mapping, 0xA0000, 0x20000); // 128kb @ 0xA0000
            nv3->nvbase.svga.banked_mask = 0x1FFFF;
            break;
        case NV3_CRTC_BANKED_64K_A0000:
            nv_log("NV3: SVGA Banked Mode = 64K @ A0000h\n");
            mem_mapping_set_addr(&nv3->nvbase.svga.mapping, 0xA0000, 0x10000); // 64kb @ 0xA0000
            nv3->nvbase.svga.banked_mask = 0xFFFF;
            break;
        case NV3_CRTC_BANKED_32K_B0000:
            nv_log("NV3: SVGA Banked Mode = 32K @ B0000h\n");
            mem_mapping_set_addr(&nv3->nvbase.svga.mapping, 0xB0000, 0x8000); // 32kb @ 0xB0000
            nv3->nvbase.svga.banked_mask = 0x7FFF;
            break;
        case NV3_CRTC_BANKED_32K_B8000:
            nv_log("NV3: SVGA Banked Mode = 32K @ B8000h\n");
            mem_mapping_set_addr(&nv3->nvbase.svga.mapping, 0xB8000, 0x8000); // 32kb @ 0xB8000
            nv3->nvbase.svga.banked_mask = 0x7FFF;
            break;
    }
}

// 
// Init code
//
void* nv3_init(const device_t *info)
{
    nv_log("NV3: initialising core\n");

    // currently using ELSA VICTORY Erazor    Ver. 1.54.03    [WD/VBE30/DDC2B/DPMS] 
    //                 ELSA VICTORY Erazor    Ver. 1.55.00    [WD/VBE30/DDC2B/DPMS] seems to be broken :(
    
    int32_t err = rom_init(&nv3->nvbase.vbios, NV3_VBIOS_ERAZOR_V15403, 0xC0000, 0x8000, 0x7fff, 0, MEM_MAPPING_EXTERNAL);
    
    if (err)
            nv_log("NV3: failed to load VBIOS err=%d\n", err);
    else    
            nv_log("NV3: Successfully loaded VBIOS %s\n", NV3_VBIOS_ERAZOR_V15403);

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

    // set vram
    nv_log("NV3: VRAM=%d bytes\n", nv3->nvbase.svga.vram_max);

    // init memory mappings
    nv3_init_mappings();

    // make us actually exist
    nv3->pci_config.int_line = 0xFF; // per datasheet
    nv3->pci_config.pci_regs[PCI_REG_COMMAND] = PCI_COMMAND_IO | PCI_COMMAND_MEM;

    // svga is done, so now initialise the real gpu
    nv_log("NV3: Initialising GPU core...\n");

    nv3_pextdev_init();             // Initialise Straps
    nv3_pmc_init();                 // Initialise Master Control
    nv3_pbus_init();                // Initialise Bus (the 128 part of riva)
    nv3_pfb_init();                 // Initialise Framebuffer Interface
    nv3_pramdac_init();             // Initialise RAMDAC (CLUT, final pixel presentation etc)
    nv3_pfifo_init();               // Initialise FIFO for graphics object submission
    nv3_pgraph_init();              // Initialise accelerated graphics engine
    nv3_ptimer_init();              // Initialise programmable interval timer
    nv3_pvideo_init();              // Initialise video overlay engines

    return nv3;
}

// This function simply allocates ram and sets the bus to pci before initialising.
void* nv3_init_pci(const device_t* info)
{
    nv3 = (nv3_t*)calloc(1, sizeof(nv3_t));
    nv3->nvbase.bus_generation = nv_bus_pci;
    nv3_init(info);
}

// This function simply allocates ram and sets the bus to agp before initialising.
void* nv3_init_agp(const device_t* info)
{
    nv3 = (nv3_t*)calloc(1, sizeof(nv3_t));
    nv3->nvbase.bus_generation = nv_bus_agp_1x;
    nv3_init(info);
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
    .init = nv3_init_pci,
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
    .init = nv3_init_agp,
    .close = nv3_close,
    .speed_changed = nv3_speed_changed,
    .force_redraw = nv3_force_redraw
};