/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          WELCOME TO NVIDIA DRIVER VERSION 2.0!
 *          nv1_prmc: Real-Mode Communication driver
 *
 * Authors: starfrost
 *
 *          Copyright 2024-2026 starfrost
 */

#include "../nv1.h"

uint32_t nv1_prmc_read(uint32_t addr)
{
    uint32_t ret = 0x00;

    // if not, read SVGA (don't log this)
    if (!nv1->prm.window.enabled)
        return svga_readl(addr, &nv1->svga);

    // the indexes are wrong because for some reason the VBIOS uses "access" index 1 (B1E04) and "window" index 0 (B1E40)
    // (GPU errata?)
    switch (addr)
    {
        case NV_MEMORY_RMC_ACCESS(1):
            if (nv1->prm.window.enabled)
                ret = NV_MEMORY_RMC_ACCESS_SECURITY_DISABLE;
            else
                ret = NV_MEMORY_RMC_ACCESS_SECURITY_ENABLE;
            break;
        case NV_MEMORY_RMC_WINDOW(0):
            ret = nv1->prm.window.addr_start;
            break;
        // VBIOS never uses these values, but depends on them to boot??? These are debug features
        // (NV_MEMORY_TRACE & 0x0F) must return 1
        // (NV_MEMORY_IGNORE_0 & 0x0F) must return 2
        case NV_MEMORY_TRACE:
            ret = NV_PRM_TRACE_IO_CAPTURE_WRITES;
            break; 
        case NV_MEMORY_IGNORE_0: 
            ret = NV_PRM_IGNORE_0_DAC_READS;
            break;
    }  

    if (addr >= NV_MEMORY_WINDOW032(0, 0)
    && addr <= NV_MEMORY_WINDOW032(0, NV_MEMORY_WINDOW032__SIZE_2))
    {
        uint32_t mmio_addr = nv1->prm.window.addr_start + (addr - NV_MEMORY_WINDOW032(0, 0));

        // these are literally the only 8bit addresses in the system (the dac has some but the nv1 doesn't care)
        if (mmio_addr >= NV1_VGA_MMIO_START
        && mmio_addr <= NV1_VGA_MMIO_END)
        {
            ret = (uint32_t)nv1_mmio_read8(mmio_addr, &nv1);
        }
        else
            ret = nv1_mmio_read32(mmio_addr, &nv1);
    
        nv_log("RMC-MMIO read %08x from %08x (VGA addr = %05x)\n", ret, mmio_addr, addr);

        return ret; 
    }
    
    nv_log("RMC-SVGA read %08x from %08x\n", ret, addr);
    return ret; // no idea what this would do...
}

void nv1_prmc_write(uint32_t addr, uint32_t val)
{    

    // the indexes are wrong because for some reason the VBIOS uses "access" index 1 (B1E04) and "window" index 0 (B1E40)
    // (GPU errata?)
    // check this before writing

    if (addr == NV_MEMORY_RMC_ACCESS(1))
    {
        nv1->prm.window.enabled = (val == NV_MEMORY_RMC_ACCESS_SECURITY_ENABLE);
        nv1->prm.window.enabled ? nv_log("PRMC window 1 enabled\n") : nv_log("PRMC window 1 disabled\n");
    }

    // if not, send writes down to SVGA
    if (!nv1->prm.window.enabled)
    {
        svga_writel(addr, val, &nv1->svga);
        return;
    } 

    if (addr == NV_MEMORY_RMC_WINDOW(0))
    {
        nv1->prm.window.addr_start = val;
        nv_log("PRMC start address is now 0x%08x\n", val);
    }

    if (addr >= NV_MEMORY_WINDOW032(0, 0)
    && addr <= NV_MEMORY_WINDOW032(0, NV_MEMORY_WINDOW032__SIZE_2))
    {
        uint32_t mmio_addr = nv1->prm.window.addr_start + (addr - NV_MEMORY_WINDOW032(0, 0));

        nv_log("RMC-MMIO write %08x to %08x (VGA addr = %05x)\n", val, mmio_addr, addr);

        // these are literally the only 8bit addresses in the system (the dac has some but the nv1 doesn't care)
        if (mmio_addr >= NV1_VGA_MMIO_START
        && mmio_addr <= NV1_VGA_MMIO_END)
        {
            nv1_mmio_write8(mmio_addr, val & 0xFF, &nv1);
        }
        else
            nv1_mmio_write32(mmio_addr, val, &nv1);
    }
}