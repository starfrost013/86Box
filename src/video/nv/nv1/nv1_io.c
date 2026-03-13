/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          WELCOME TO NVIDIA DRIVER VERSION 2.0! 
 *          nv1_io: Handles PCI, VGA, MMIO, STG1732/1764, AD1848, NV_USER
 *
 * Authors: starfrost
 *
 *          Copyright 2024-2026 starfrost
 */

#include "nv1.h"

//
// PCI
// 

uint8_t nv1_pci_read(int32_t func, int32_t address, int32_t len, void* priv)
{
    uint8_t ret;

    // technically,
    // VGA              0-FF
    // NV1              100-1FF
    // but in reality we don't realy care
    address &= 0xFF;

    switch (address)
    {
        // PCI ID
        case NV_CONFIG_PCI_VGA_0:
            return (NV_CONFIG_PCI_NV_0_VENDOR_ID_NVIDIA) & 0xFF;    
        case NV_CONFIG_PCI_VGA_0 + 1:
            return (NV_CONFIG_PCI_NV_0_VENDOR_ID_NVIDIA >> 8) & 0xFF;    
        case NV_CONFIG_PCI_VGA_0 + 2:
            return (NV_CONFIG_PCI_NV_0_DEVICE_ID_CHIP_NV1 << 3) | func;
        case NV_CONFIG_PCI_VGA_0 + 3:
            return 0x00; // it doesn't actually matter what this value is
        case NV_CONFIG_PCI_NV_4:
            if (func == NV1_PCI_FUNCTION_VGA)
                return 0x00;

            return ((nv1->bar0_addr >> 25) << 25) | (1 << NV_CONFIG_PCI_NV_4_PREFETCHABLE);
        case NV_CONFIG_PCI_NV_4 + 1 ... NV_CONFIG_PCI_NV_4 + 3:
                return 0x00; 
        default:
            // return pci block based on function
            if (func == NV1_PCI_FUNCTION_VGA)
                ret = nv1->pci_regs_vga[address & 0xFF];
            else   
                ret = nv1->pci_regs_nv[address & 0xFF];

    }

    return ret;
}

void nv1_pci_write(int32_t func, int32_t address, int32_t len, uint8_t value, void* priv)
{
    address &= 0xFF;

    switch (address)
    {
        case NV_CONFIG_PCI_NV_4:
            if (func == NV1_PCI_FUNCTION_VGA)
                nv1->pci_regs_vga[address] = 0x00;
            else
                nv1->bar0_addr = (value << 24);
            break; 

            nv1_update_mappings();
    }

    // default case
    if (func == NV1_PCI_FUNCTION_VGA)
        nv1->pci_regs_vga[address] = value;
    else
        nv1->pci_regs_nv[address] = value;
}

//
// SVGA (Function 0)
//

uint8_t nv1_svga_read(uint16_t addr, void* priv)
{
    // temp
    return svga_in(addr, &nv1->svga);
}

void nv1_svga_write(uint16_t addr, uint8_t val, void* priv)
{
    svga_out(addr, val, &nv1->svga);
}

//
// MMIO
// *ALL* reads are 32-bit
//

uint8_t nv1_mmio_read8(uint32_t addr, void* priv)
{
    uint32_t ret = 0x00;

    // see if unaligned reads are a problem
    ret = nv1_mmio_read32(addr, priv);
    return (uint8_t)(ret >> ((addr & 3) << 3) & 0xFF);
}

uint16_t nv1_mmio_read16(uint32_t addr, void* priv)
{
    uint32_t ret = 0x00;

    ret = nv1_mmio_read32(addr, priv);
    return (uint16_t)(ret >> ((addr & 3) << 3) & 0xFFFF);
}

uint32_t nv1_mmio_read32(uint32_t addr, void* priv)
{
    uint32_t ret = 0x00;

    addr &= (NV1_MMIO_SIZE - 1); // must be power of 2

    // DFB at 1000000
    if (addr & 0x1000000)
        ret = nv1->svga.vram[addr & (nv1->vram_amount) - 1]; // always pot so fine

    return ret; 
}

void nv1_mmio_write8(uint32_t addr, uint8_t val, void* priv)
{
    // overwrite first 8 bits of a 32 bit value
    uint32_t new_val = nv1_mmio_read32(addr, nv1);

    new_val &= (~0xFF << (addr & 3) << 3);
    new_val |= (val << ((addr & 3) << 3));

    nv1_mmio_write32(addr, new_val, nv1);
}

// 16-bit MMIO write
void nv1_mmio_write16(uint32_t addr, uint16_t val, void* priv)
{
    // overwrite first 16 bits of a 32 bit value
    uint32_t new_val = nv1_mmio_read32(addr, nv1);

    new_val &= (~0xFFFF << (addr & 3) << 3);
    new_val |= (val << ((addr & 3) << 3));

    nv1_mmio_write32(addr, new_val, nv1);
}

// 32-bit MMIO write
void nv1_mmio_write32(uint32_t addr, uint32_t val, void* priv)
{
    addr &= (NV1_MMIO_SIZE - 1); // must be power of 2
    
    // DFB at 1000000
    if (addr & 0x1000000)
        nv1->svga.vram[addr & (nv1->vram_amount) - 1] = val;
}