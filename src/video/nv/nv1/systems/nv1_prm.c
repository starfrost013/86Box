/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          WELCOME TO NVIDIA DRIVER VERSION 2.0!
 *          nv1_prmc: Real-Mode Communication driver and custom SVGA I/O handler
 *
 * Authors: starfrost
 *
 *          Copyright 2024-2026 starfrost
 */

#include "../nv1.h"

// Check if any of the RMC windows are enabled
bool nv1_rmc_window_is_enabled()
{
    for (int32_t i = 0; i < NV_PBUS_RMC_WINDOW__SIZE_1; i++)
    {
        if (nv1->prm.windows[i].enabled)
            return true;
    }

    return false; 
}

void nv1_enable_rmc_if_needed(uint32_t addr, uint32_t val)
{            
    if (addr >= NV_MEMORY_RMC_ACCESS(0)
    && addr <= NV_MEMORY_RMC_ACCESS(NV_MEMORY_RMC_ACCESS__SIZE_1)
    && (addr & 0xFFFFFF00)) // should not be required, but let's put it here for consistency...
    {
        // calculate window index (off by one due to some errata)
        int32_t window_index = (((addr - NV_MEMORY_RMC_ACCESS(0)) >> 2)); // - 1

        // the vbios has an off by one error, where it writes to b1e04 for window 1 but uses b1e40 for window 0
        // so we have to do this stupid shit
        if (window_index < 0)
            window_index = 0;

        nv1->prm.windows[window_index].enabled = (val == NV_MEMORY_RMC_ACCESS_SECURITY_ENABLE);
        nv1->prm.windows[window_index].enabled ? nv_log("PRMC window %d enabled\n", window_index) : nv_log("PRMC window %d disabled\n", window_index);
    }
}

uint8_t nv1_svga_read8(uint32_t addr, void* priv)
{
    if (!nv1_rmc_window_is_enabled())
        return svga_read(addr, &nv1->svga);
    else
        return nv1_prmc_read(addr);
}

uint16_t nv1_svga_read16(uint32_t addr, void* priv)
{
    if (!nv1_rmc_window_is_enabled())
        return svga_readw(addr, &nv1->svga);
    else
        return nv1_prmc_read(addr);
}

uint32_t nv1_svga_read32(uint32_t addr, void* priv)
{
    if (!nv1_rmc_window_is_enabled())
        return svga_readl(addr, &nv1->svga);
    else
        return nv1_prmc_read(addr);
}

void nv1_svga_write8(uint32_t addr, uint8_t val, void* priv)
{
    nv1_enable_rmc_if_needed(addr, val);

    // we don't need to put any checks on this since all RMC writes are gated in gonv/nonv writes
    if (!nv1_rmc_window_is_enabled())
        svga_write(addr, val, &nv1->svga);
    else
        nv1_prmc_write(addr, val);
}

void nv1_svga_write16(uint32_t addr, uint16_t val, void* priv)
{
    nv1_enable_rmc_if_needed(addr, val);

    if (!nv1_rmc_window_is_enabled())
        svga_writew(addr, val, &nv1->svga);
    else
        nv1_prmc_write(addr, val);
}

void nv1_svga_write32(uint32_t addr, uint32_t val, void* priv)
{
    nv1_enable_rmc_if_needed(addr, val);

    if (!nv1_rmc_window_is_enabled())
        svga_writel(addr, val, &nv1->svga);
    else
        nv1_prmc_write(addr, val);
}

//
// mmio regs
//

uint32_t nv1_prmc_read_prm(uint32_t addr)
{
    uint32_t ret = 0x00;

    switch (addr)
    {
        case NV_PRM_DEBUG_0:
            ret = nv1->prm.debug;
            break; 
        case NV_PRM_CONFIG_0:
            ret = nv1->prm.config; 
            break; 
        case NV_PRM_INTR_0:
            ret = nv1->prm.intr;
            break;
        case NV_PRM_INTR_EN_0:
            ret = nv1->prm.intr_en;
            break;
        case NV_PRM_TRACE:
        case NV_MEMORY_TRACE:
            ret = nv1->prm.trace;
            break; 
        case NV_PRM_IGNORE_0:
        case NV_MEMORY_IGNORE_0: //worst code
            ret = nv1->prm.ignore_0 | NV_PRM_IGNORE_0_DAC_READS;
            break;
        case NV_PRM_IGNORE_1:
        case NV_MEMORY_IGNORE_1: //worst code
            ret = nv1->prm.ignore_0;
            break;
        }

    return ret;
}

uint32_t nv1_prmc_write_prm(uint32_t addr, uint32_t val)
{
    switch (addr)
    {
        case NV_PRM_DEBUG_0:
            nv1->prm.debug = val;
            break;
        // bit 0 - text mode
        // bit 4 - 0x01 - 8 bit, 0x00 - 6bit
        case NV_PRM_CONFIG_0:
            nv1->prm.config = val;
            //nv1->svga.ramdac_type = ((val >> NV_PRM_CONFIG_0_DAC_WIDTH) & 0x01) ? RAMDAC_8BIT : RAMDAC_6BIT; 
            break;
        case NV_PRM_INTR_0:
            nv1->prm.intr = val;
            // TODO: FIRE INTERRUPT!!!!!
            break;
        case NV_PRM_INTR_EN_0:
            nv1->prm.intr_en = val;
            break;
        // these are also mirrored into the real-mode space :/
        case NV_PRM_TRACE:
        case NV_MEMORY_TRACE:
            nv1->prm.trace = val;
            break; 
        case NV_PRM_IGNORE_0:
        case NV_MEMORY_IGNORE_0: //worst code, hack for some stupid shit
            nv1->prm.ignore_0 = val; 
            break;
        case NV_PRM_IGNORE_1:
        case NV_MEMORY_IGNORE_1: //worst code
            nv1->prm.ignore_1 = val; 
            break;
    }
}

// init function
void nv1_prmc_init()
{
    nv1->prm.intr_en |= (NV_PRM_INTR_EN_0_VBLANK_ENABLED << NV_PRM_INTR_EN_0_VBLANK);

    // unknown reasons these are required to boot the Video BIOS
    nv1->prm.trace |= NV_PRM_TRACE_IO_CAPTURE;
    nv1->prm.ignore_0 |= NV_PRM_IGNORE_0_DAC_READS;
}

//
// REAL MODE REGS
//

uint32_t nv1_prmc_read(uint32_t addr)
{
    uint32_t ret = 0x00;
    // if not, read SVGA (don't log this)

    // send mmio writes to mmio
    if (addr >= NV_PRM_START
    && addr <= NV_PRM_END)
    {
        return nv1_prmc_read_prm(addr); 
    }

    int32_t window_index = 0;

    // the indexes are wrong because for some reason the GPU uses "access" index i+1 (B1E04) and "window" index 0 (B1E40)
    // (GPU errata?)
    switch (addr)
    {
        case NV_MEMORY_RMC_ACCESS(0) ... NV_MEMORY_RMC_ACCESS(NV_MEMORY_RMC_ACCESS__SIZE_1):
            window_index = (((addr - NV_MEMORY_RMC_ACCESS(0)) >> 2));// - 1

            // fix off by one error in vbios, idk why this even works
            if (window_index < 0)
                window_index = 0; 

            if (nv1->prm.windows[window_index].enabled)
                ret = NV_MEMORY_RMC_ACCESS_SECURITY_DISABLE;
            else
                ret = NV_MEMORY_RMC_ACCESS_SECURITY_ENABLE;
            break;
        case NV_MEMORY_RMC_WINDOW(0) ... NV_MEMORY_RMC_WINDOW(NV_MEMORY_RMC_WINDOW__SIZE_1):
            window_index = (((addr - NV_MEMORY_RMC_WINDOW(0)) >> 4)); // /16

            ret = nv1->prm.windows[window_index].addr_start;
            break;
        // VBIOS never uses these values, but depends on them to boot??? These are debug features, so we don't need to emulate them
        // (NV_MEMORY_TRACE & 0x0F) must return 1
        // (NV_MEMORY_IGNORE_0 & 0x0F) must return 2
        case NV_MEMORY_TRACE:
        case NV_MEMORY_IGNORE_0: 
        case NV_MEMORY_IGNORE_1: 
            ret = nv1_prmc_read_prm(addr);
            break;
    }  

    if (addr >= NV_MEMORY_WINDOW032(0, 0)
    && addr <= NV_MEMORY_WINDOW032(NV_MEMORY_WINDOW032__SIZE_1, NV_MEMORY_WINDOW032__SIZE_2)) // END AT b7fff (3 windows)
    {
        window_index = (addr - NV_MEMORY_WINDOW032(0, 0)) / NV_MEMORY_WINDOW008__SIZE_2; // use 8 bitindex here
 
        uint32_t mmio_addr = nv1->prm.windows[window_index].addr_start + (addr - NV_MEMORY_WINDOW032(window_index, 0));

        // these are literally the only 8bit addresses in the system (the dac has some but the nv1 doesn't care)
        if (mmio_addr >= NV1_VGA_MMIO_START
        && mmio_addr <= NV1_VGA_MMIO_END)
        {
            ret = (uint32_t)nv1_mmio_read8(mmio_addr, &nv1);
        }
        else
        {
            // don't log svga read/write 
            ret = nv1_mmio_read32(mmio_addr, &nv1);
            nv_log("RMC-MMIO window %d read %08x from %08x (VGA addr = %05x)\n", window_index, ret, mmio_addr, addr);
        }
    

        return ret; 
    }
    
    nv_log("RMC-SVGA read %08x from %08x\n", ret, addr);
    return ret; // no idea what this would do...
}

// prmc write
void nv1_prmc_write(uint32_t addr, uint32_t val)
{    
    uint32_t window_index = 0;

    // the indexes are wrong because for some reason the VBIOS uses "access" index 1 (B1E04) and "window" index 0 (B1E40)
    // (GPU errata?)
    // check this before writing

    // send mmio writes to mmio
    if (addr >= NV_PRM_START
    && addr <= NV_PRM_END)
    {
        nv1_prmc_write_prm(addr, val);
        return; 
    }

    /* only respond to b1e40, b1e50, be160, b1e70 */

    if (addr >= NV_MEMORY_RMC_WINDOW(0)
    && addr <= NV_MEMORY_RMC_WINDOW(NV_MEMORY_RMC_WINDOW__SIZE_1))
    {
        window_index = (((addr - NV_MEMORY_RMC_WINDOW(0)) >> 4)); // /16

        // only bits 24:13 matter
        nv1->prm.windows[window_index].addr_start = ((val & 0x1FFFFFF) >> 13) << 13;
        nv_log("PRMC window %d start address is now 0x%08x\n", window_index, nv1->prm.windows[window_index].addr_start);
    }

    if (addr >= NV_MEMORY_WINDOW032(0, 0)
    && addr <= NV_MEMORY_WINDOW032(NV_MEMORY_WINDOW032__SIZE_1, NV_MEMORY_WINDOW032__SIZE_2)
    && (addr & 0xFFFFFF00))
    {
        window_index = (addr - NV_MEMORY_WINDOW032(0, 0)) / NV_MEMORY_WINDOW008__SIZE_2; // use 8 bitindex here
 
        uint32_t mmio_addr = nv1->prm.windows[window_index].addr_start + (addr - NV_MEMORY_WINDOW032(window_index, 0));

        // these are literally the only 8bit addresses in the system (the dac has some but the nv1 doesn't care)
        if (mmio_addr >= NV1_VGA_MMIO_START
        && mmio_addr <= NV1_VGA_MMIO_END)
        {
            nv1_mmio_write8(mmio_addr, val & 0xFF, &nv1);
        }
        else
        {
            // don't log
            nv1_mmio_write32(mmio_addr, val, &nv1);
            nv_log("RMC-MMIO window %d write %08x to %08x (VGA addr = %05x)\n", window_index, val, mmio_addr, addr);
        }
    }
}