/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          WELCOME TO NVIDIA DRIVER VERSION 2.0! 
 *
 * Authors: starfrost
 *
 *          Copyright 2024-2026 starfrost
 */

#pragma once

// SOMEONE has to clean this up!!!
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <86box/86box.h>
#include <86box/device.h>
#include <86box/mem.h>
#include <86box/io.h>
#include <86box/log.h>
#include <86box/pci.h>
#include <86box/rom.h> 
#include <86box/timer.h> 
#include <86box/video.h>
#include <86box/vid_svga.h>
#include <86box/vid_svga_render.h>

#include "nv1_regs.h"
#include "../../ramdac/vid_ramdac_stg1732.h"

//
// DEFINES
// Extra defines whcih aren't in "nv1_regs.h"
//

#define NV1_VBIOS_E3D_3X00                          "roms/video/nvidia/nv1/Diamond_Edge_3D_3400_BIOS_M27C256.BIN" 
#define NV1_VBIOS_SIZE                              32768       // Size of Video BIOS
#define NV1_VBIOS_LOCATION                          0xC0000     // Location of Video BIOS
#define NV1_VRAM_SIZE_1MB                           0x100000
#define NV1_VRAM_SIZE_2MB                           0x200000
#define NV1_VRAM_SIZE_4MB                           0x400000

#define NV1_PCI_FUNCTION_VGA                        0           // STMicro VGA function
#define NV1_PCI_FUNCTION_NV1                        1           // NVIDIA NV1 functino
#define NV1_PCI_NUM_REGS                            0xFF        // Number of pci regs

#define NV1_MMIO_SIZE                               0x2000000   // 32M (incl. VRAM)

#define NV1_VGA_START                               0x03C0      // vga start
#define NV1_VGA_SIZE                                0x0020

#define NV1_VGA_MMIO_START                          0x6D03B0    // registers which are sent to VGA
#define NV1_VGA_MMIO_END                            0x6D03DF    // registers which are sent to VGA

// Not defined by NV's?
#define NV1_VGA_RAM_START                           0xA0000
#define NV1_VGA_RAM_END                             0xBFFFF
#define NV1_VGA_BIOS_START                          0xC0000
#define NV1_VGA_BIOS_END                            0xC7FFF

#define NV1_LAST_DAC_REG                            0x1C >> 2   // not in sgs_dac.h

#define NV1_PEEPROM_SIZE                            128         // 128 bytes 

#define NV1_PMC_BOOT_0_GENERIC_REVB2                0x00010102  // NV1 Revision B2

// 
// STRUCTS
//

typedef struct nv1_prm_window_s
{
    uint32_t addr_start;
    bool enabled;
} nv1_prm_window_t;

typedef struct nv1_prm_s
{
    // The GPU actually implements *4* windows, 
    // but in practice, only one is ever used, so to simplify the implementation, we just implement one
    // IF THERE IS A BUG IN DOS APPS FOR NV1, CHANGE THIS
    // there's also x86 segmentation type functionality to move the 8KB sliding window anywhere in MMIO, but this register is never touched?
    nv1_prm_window_t window;

    uint32_t debug;                 // debug register
    uint32_t config;                // 6/8 bits mode
    uint32_t intr, intr_en;
    uint32_t trace;                 // debug
    uint32_t ignore_0;              // ignore for trace
    uint32_t ignore_1;              // ignore for trace
} nv1_prm_t; 

typedef struct nv1_pfb_s
{    
    uint32_t boot;
    uint32_t debug;
    uint32_t config_0;              // config register
    uint32_t green_0;               // power-down register

    // NV1 is not vga compatible, but we "translate" this to 86box-SVGA for easier emulation
    uint32_t hfrontporch;           // 0x600500 NV_PFB_HOR_FRNT_PORCH
    uint32_t hsync_width;           // 0x600510 NV_PFB_HOR_SYNC_WIDTH
    uint32_t hbackporch;            // 0x600520 NV_PFB_HOR_BACK_PORCH
    uint32_t hdisp;                 // 0x600530 NV_PFB_HOR_DISP_WIDTH
    uint32_t vfrontporch;           // 0x600540 NV_PFB_VER_FRNT_PORCH
    uint32_t vsync_width;           // 0x600550 NV_PFB_VER_SYNC_WIDTH   
    uint32_t vbackporch;            // 0x600560 NV_PFB_VER_BACK_PORCH
    uint32_t vdisp;                 // 0x600570 NV_PFB_VER_DISP_WIDTH
} nv1_pfb_t; 

typedef struct nv1_s
{
    void* log;                                                  // debug builds only
    void* ramdac;                                               // so we can run it without the ramdac
    uint32_t vram_amount;                                       // amount of vram
    uint8_t pci_slot;                                           // PCI slot number
    uint8_t pci_regs_vga[NV1_PCI_NUM_REGS];                     // Function 0
    uint8_t pci_regs_nv[NV1_PCI_NUM_REGS];                      // Function 1
    uint8_t pci_int_line;                                       // 0-15, FF for none
    bool pci_vbios_enabled;                                     // is bios enabled
    uint32_t bar0_addr;                                         // Must align to 32M
    mem_mapping_t mapping_vga;
    mem_mapping_t mapping_mmio;
    mem_mapping_t mapping_prm;                                  // map all vga ram accesses to nv1 for PRM handling
    svga_t svga;                                                // Function 0 Base
    rom_t vbios;

    // SYSTEMS
    nv1_prm_t prm;
    nv1_pfb_t pfb; 
    uint32_t eeprom[NV1_PEEPROM_SIZE >> 2];                     // eeprom apparently
} nv1_t;


//
// GLOBALS
//
extern nv1_t* nv1;                              // NV1 device
extern const device_config_t nv1_config[];      // NV1 device configuration

//
// FUNCTIONS
//

// Device General
void* nv1_init(const device_t* dev);
void nv1_close(void* priv);
void nv1_speed_changed(void *priv);
void nv1_draw_cursor(svga_t* svga, int32_t drawline);
void nv1_recalc_timings(svga_t* svga);
void nv1_force_redraw(void* priv);
void nv1_update_mappings(int32_t func);
void nv_log(const char *fmt, ...);

// I/O - PCI
// PCI function 0 is VGA. PCI function 1 is NVIDIA

uint8_t nv1_pci_read(int32_t func, int32_t address, int32_t len, void* priv);
void nv1_pci_write(int32_t func, int32_t address, int32_t len, uint8_t val, void* priv);

// I/O - SVGA
uint8_t nv1_svga_read_io(uint16_t addr, void* priv);
void nv1_svga_write_io(uint16_t addr, uint8_t val, void* priv);

uint8_t nv1_svga_read8(uint32_t addr, void* priv);
uint16_t nv1_svga_read16(uint32_t addr, void* priv);
uint32_t nv1_svga_read32(uint32_t addr, void* priv);
void nv1_svga_write8(uint32_t addr, uint8_t val, void* priv);
void nv1_svga_write16(uint32_t addr, uint16_t val, void* priv);
void nv1_svga_write32(uint32_t addr, uint32_t val, void* priv);

// I/O - MMIO
uint8_t nv1_mmio_read8(uint32_t addr, void* priv);
uint16_t nv1_mmio_read16(uint32_t addr, void* priv);
uint32_t nv1_mmio_read32(uint32_t addr, void* priv);
void nv1_mmio_write8(uint32_t addr, uint8_t val, void* priv);
void nv1_mmio_write16(uint32_t addr, uint16_t val, void* priv);
void nv1_mmio_write32(uint32_t addr, uint32_t val, void* priv);

uint32_t nv1_mmio_dispatch_read(uint32_t addr);                         // ensures reads are sent to the right gpu subsystem
void nv1_mmio_dispatch_write(uint32_t addr, uint32_t val);              // ensures writes are sent to the right gpu subsystem

// subsystems
void nv1_prmc_init();
uint32_t nv1_prmc_read(uint32_t addr);
void nv1_prmc_write(uint32_t addr, uint32_t val);
uint32_t nv1_pfb_read(uint32_t addr);
void nv1_pfb_write(uint32_t addr, uint32_t val);
uint32_t nv1_pmc_read(uint32_t addr);
void nv1_pmc_write(uint32_t addr, uint32_t val);