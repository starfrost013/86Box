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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <86box/86box.h>
#include <86box/device.h>
#include <86box/mem.h>
#include <86box/io.h>
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
#define NV1_VRAM_SIZE_1MB                           0x100000
#define NV1_VRAM_SIZE_2MB                           0x200000
#define NV1_VRAM_SIZE_4MB                           0x400000

// 
// STRUCTS
//

typedef struct nv1_s
{

} nv1_t;

//
// GLOBALS
//

extern const device_config_t nv1_config[];

//
// FUNCTIONS
//

void* nv1_init(const device_t* dev);
void nv1_close(void* priv);
void nv1_speed_changed(void *priv);
void nv1_draw_cursor(svga_t* svga, int32_t drawline);
void nv1_recalc_timings(svga_t* svga);
void nv1_force_redraw(void* priv);