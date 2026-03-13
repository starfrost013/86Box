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

#include "nv1_regs.h"

//
// DEFINES
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

#define NV1_VGA_START                               0x03C0
#define NV1_VGA_SIZE                                0x0020

// 
// STRUCTS
//

typedef struct nv1_s
{
    void* log;                                                  // debug builds only
    uint32_t vram_amount;
    uint8_t pci_slot;                                           // PCI slot number
    uint8_t pci_regs_vga[NV1_PCI_NUM_REGS];                     // Function 0
    uint8_t pci_regs_nv[NV1_PCI_NUM_REGS];                      // Function 1
    mem_mapping_t mapping_vga;
    mem_mapping_t mapping_mmio;
    svga_t svga;                                                // Function 0 Base
    rom_t vbios;
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

// I/O - PCI
// PCI function 0 is VGA. PCI function 1 is NVIDIA

uint8_t nv1_pci_read(int32_t func, int32_t address, int32_t len, void* priv);
void nv1_pci_write(int32_t func, int32_t address, int32_t len, uint8_t value, void* priv);

// I/O - SVGA
uint8_t nv1_svga_read(uint16_t addr, void* priv);
void nv1_svga_write(uint16_t addr, uint8_t val, void* priv);

// I/O - MMIO
uint8_t nv1_mmio_read8(uint32_t addr, void* priv);
uint16_t nv1_mmio_read16(uint32_t addr, void* priv);
uint32_t nv1_mmio_read32(uint32_t addr, void* priv);
void nv1_mmio_write8(uint32_t addr, uint8_t val, void* priv);
void nv1_mmio_write16(uint32_t addr, uint16_t val, void* priv);
void nv1_mmio_write32(uint32_t addr, uint32_t val, void* priv);
