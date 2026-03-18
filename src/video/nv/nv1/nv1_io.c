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

uint8_t nv1_pci_read(int32_t func, int32_t addr, int32_t len, void* priv)
{
    uint8_t ret = 0x00;

    // completely ignore invalid function reads
    if (func > NV1_PCI_FUNCTION_NV1)
        return 0xFF;

    // technically,
    // VGA              0-FF
    // NV1              100-1FF
    // but in reality we don't realy care
    addr &= 0xFF;

    // Use 86box devs rather tha nnvidia defs since they are less confusing + nvidia defs are for 32-bit writses
    switch (addr)
    {
        // PCI ID
        case PCI_REG_VENDOR_ID_L:
            ret = (NV_CONFIG_PCI_NV_0_VENDOR_ID_NVIDIA) & 0xFF;    
            break;
        case PCI_REG_VENDOR_ID_H:
            ret = (NV_CONFIG_PCI_NV_0_VENDOR_ID_NVIDIA >> 8) & 0xFF;    
            break;
        case PCI_REG_DEVICE_ID_L:
            ret = (NV_CONFIG_PCI_NV_0_DEVICE_ID_CHIP_NV1 << 3) | func;
            break;
        case PCI_REG_DEVICE_ID_H:
            ret = 0x00; // it doesn't actually matter what this value is
            break;
        case PCI_REG_STATUS_H: // STATUS_H, these use different devsel timing
            if (func == NV1_PCI_FUNCTION_NV1)
                ret = nv1->pci_regs_nv[addr] | (NV_CONFIG_PCI_NV_1_DEVSEL_TIMING_FAST << 5);
            else
                ret = nv1->pci_regs_vga[addr] | (NV_CONFIG_PCI_NV_1_DEVSEL_TIMING_MEDIUM << 5);
            break;
        case PCI_REG_REVISION:
            // A01 and B01 are prototypes.
            // Most NV1s in the wild seem to be B02
            // C01 was possibly never put into production (it's after the production halt)
            ret = NV_CONFIG_PCI_NV_2_REVISION_ID_B02;
            break; 
        case PCI_REG_PROG_IF:
            ret = 0x00;
            break;
        // 'VGA device' for func 0
        // else 0x48000 ('Multifunction device;)
        case PCI_REG_SUBCLASS:
            if (func == NV1_PCI_FUNCTION_VGA)
                ret = 0x00; 
            else
                ret = 0x80; // 0x48000
            break;
        case PCI_REG_CLASS:
            if (func == NV1_PCI_FUNCTION_VGA)
                ret = 0x03; // vga controller
            else   
                ret = 0x04; // multifunction device
            break;
        case PCI_REG_BAR0_BYTE0:
            if (func == NV1_PCI_FUNCTION_VGA)
                ret = 0x00;
            else
                ret = ((nv1->bar0_addr >> 25) << 1) | (1 << NV_CONFIG_PCI_NV_4_PREFETCHABLE); // bit 24 is disregarded, 1 byte boundary
            break;
        case PCI_REG_BAR0_BYTE1 ... PCI_REG_BAR5_BYTE3: // all other BARs are hardwired to 0
            ret = 0x00; 
            break;
        case PCI_REG_HEADER_TYPE: // multifunction device
            ret = NV_CONFIG_PCI_NV_3_HEADER_TYPE_MULTIFUNC;
            break;
        case PCI_REG_INT_LINE:
            ret = nv1->pci_int_line;
            break;
        case PCI_REG_INT_PIN:
            ret = NV_CONFIG_PCI_NV_15_INTR_PIN_INTA;
            break;
        case PCI_REG_MIN_GRANT: // maximum grant
            if (func == NV1_PCI_FUNCTION_VGA)
                ret = 0x00;
            else
                ret = NV_CONFIG_PCI_NV_15_MIN_GNT_750NS;
            break;
        case PCI_REG_MAX_LAT:
            if (func == NV1_PCI_FUNCTION_VGA)
                ret = 0x00;
            else
                ret = NV_CONFIG_PCI_NV_15_MAX_LAT_250NS;
                break;  
        // aliased across functions
        case PCI_REG_ROM_BAR_BYTE0 ... PCI_REG_ROM_BAR_BYTE3:
            ret = nv1->pci_regs_vga[addr & 0xFF];
            break;
        default:
            // return pci block based on function
            if (func == NV1_PCI_FUNCTION_VGA)
                ret = nv1->pci_regs_vga[addr & 0xFF];
            else   
                ret = nv1->pci_regs_nv[addr & 0xFF];
            break;
    }

    nv_log("PCI func %d, read 0x%08x from 0x%02x\n", func, ret, addr);

    return ret;
}

void nv1_pci_write(int32_t func, int32_t addr, int32_t len, uint8_t val, void* priv)
{
    addr &= 0xFF;

    // update the mappings after we are done
    bool update_mappings = false;

    // completely ignore invalid function reads
    if (func > NV1_PCI_FUNCTION_NV1)
        return;
        
    switch (addr)
    {
        case PCI_REG_COMMAND_H:
            update_mappings = true;
            break;
        case PCI_REG_COMMAND_L:
            update_mappings = true;
            break;
        case PCI_REG_BAR0_BYTE0:
            // vga function has no bars
            if (func == NV1_PCI_FUNCTION_NV1)
            {            
                nv1->bar0_addr = ((val & 0b11111110) << 24);   
                update_mappings = true;
            }
            break;
        case PCI_REG_INT_LINE:
            nv1->pci_int_line = val;
            break;
        // vbios control
        case PCI_REG_ROM_BAR_BYTE0:
            nv1->pci_vbios_enabled = (val & 0x01);

            if (nv1->pci_vbios_enabled)
            {
                mem_mapping_enable(&nv1->vbios.mapping);
                nv_log("VBIOS enabled\n");
            }
            else
            {
                mem_mapping_disable(&nv1->vbios.mapping);
                nv_log("VBIOS disabled\n");
            }
            break;
        // we don't need to do anything in byte 1 (or most of 2)
        // This is meant to be mapped at a 4M boundary per the datasheet and are aliased between both functions
        // but if we implement that the VBIOS never runs
        case PCI_REG_ROM_BAR_BYTE3:
            if (nv1->pci_vbios_enabled)
                mem_mapping_set_addr(&nv1->vbios.mapping, NV1_VBIOS_LOCATION, NV1_VBIOS_SIZE);
            break; 

    }

    // always reflect the registers (read will decide what gets returned)
    if (func == NV1_PCI_FUNCTION_VGA)
        nv1->pci_regs_vga[addr] = val;
    else
        nv1->pci_regs_nv[addr] = val;

    // write all register changes out before updating mappings
    if (update_mappings)
        nv1_update_mappings(func);

    nv_log("PCI func %d, write 0x%08x to 0x%02x\n", func, val, addr);
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
    switch (addr)
    {
        case NV_IO_CC_ADDRESS__COLOR:
            
            break;

    }
    svga_out(addr, val, &nv1->svga);
}

//
// MMIO
// *ALL* reads are 32-bit *EXCEPT* vga registers, so 8-bit has some special handling for these 
//

uint8_t nv1_mmio_read8(uint32_t addr, void* priv)
{
    uint32_t ret = 0x00;

    // see if unaligned reads are a problem
    // VGA mirror at 6d13c0-6d13df (also gamepot)

    if (addr >= NV1_VGA_MMIO_START
    && addr <= NV1_VGA_MMIO_END)
    {
        ret = svga_in(NV1_VGA_START + (addr & 0x1F), &nv1->svga);
        return ret; 
    }

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
    uint32_t* vram_32 = (uint32_t*)nv1->svga.vram;

    addr &= (NV1_MMIO_SIZE - 1); // must be power of 2

    // DFB at 1000000
    if (addr & 0x1000000)
    {
        // temp debug code
        uint32_t vram_addr = addr & nv1->vram_amount - 1;
        ret = vram_32[vram_addr >> 2];

        nv_log("DFB read %08x from %08x (raw addr - %08x)\n", ret, vram_addr, addr);
    }
    else
        ret = nv1_mmio_dispatch_read(addr);

    return ret; 
}

void nv1_mmio_write8(uint32_t addr, uint8_t val, void* priv)
{
    // VGA mirror at 6d13c0-6d13df (also gamepot)

    if (addr >= NV1_VGA_MMIO_START
    && addr <= NV1_VGA_MMIO_END)
    {
        svga_out(NV1_VGA_START + (addr & 0x1F), val, &nv1->svga);
        return; 
    }

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
    uint32_t* vram_32 = (uint32_t*)nv1->svga.vram;

    // DFB at 1000000
    if (addr & 0x1000000)
    {
        // temp debug code
        uint32_t vram_addr = addr & nv1->vram_amount - 1;
        vram_32[vram_addr >> 2] = val;

        nv_log("DFB write %08x to %08x (raw addr - %08x)\n", val, vram_addr, addr);
    }
    else
        nv1_mmio_dispatch_write(addr, val);
}

// ensure a read reaches the right part of the emulated gpi
uint32_t nv1_mmio_dispatch_read(uint32_t addr)
{    
    // allow us to turn off logs for certain subsystems that have endless writes or reads
    bool send_log = true; 
    bool unimpl = false; // same for unimplemented
    uint32_t ret = 0x00;

    // TODO: PMC_ENABLE et al

    switch (addr)
    {   
        case NV1_VGA_RAM_START ... NV1_VGA_RAM_END:
            send_log = false; 
            ret = nv1_prmc_read(addr);
            break;
        case NV1_VGA_BIOS_START ... NV1_VGA_BIOS_END: // read vbios
            ret = nv1->vbios.rom[addr & 0x7FFF];
            break;
        default: // set unimplemented
            unimpl = true;
            break;
    }           

    if (send_log)
    {
        if (unimpl)
            nv_log("***** UNIMPLEMENTED SUBSYSTEM ***** MMIO read 0x%08x from 0x%08x\n", ret, addr);
        else 
            nv_log("MMIO read 0x%08x from 0x%08x\n", ret, addr);
    }

    return ret; 
}

// ensure a write reaches the right part of the emulated gpu
void nv1_mmio_dispatch_write(uint32_t addr, uint32_t val)
{
    // allow us to turn off logs for certain subsystems that have endless writes or reads
    bool send_log = true; 
    bool unimpl = false; // same for unimplemented

    // TODO: PMC_ENABLE et al

    switch (addr)
    {
        case NV1_VGA_RAM_START ... NV1_VGA_RAM_END:
            send_log = false; 

            nv1_prmc_write(addr, val);
            break;
        case NV1_VGA_BIOS_START ... NV1_VGA_BIOS_END: // read vbios
            break; // can't write to ROM
        default: // set unimplemented
            unimpl = true;
            break;
    }

    if (send_log)
    {
        if (unimpl)
            nv_log("***** UNIMPLEMENTED SUBSYSTEM ***** MMIO write 0x%08x to 0x%08x\n", val, addr);
        else 
            nv_log("MMIO write 0x%08x to 0x%08x\n", val, addr);
    }
}