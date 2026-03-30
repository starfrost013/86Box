/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          STG1732 (Van Gogh) / 1764 (Van Dyke; aka NVDAC64) 
 *          32-bit colour (64-bit pixel input on STG1764) DAC - SGS-Thomson/Nvidia
 *          Used for NVidia NV1. Based on 1702 code
 *
 * Authors: Connor Hyde, <mario64crashed@gmail.com>
 *
 *          Copyright 2026 starfrost
 *          This DAC has no vga interface
 */


#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <86box/86box.h>
#include <86box/device.h>
#include <86box/mem.h>
#include <86box/timer.h>
#include <86box/video.h>
#include <86box/vid_svga.h>
#include <86box/plat_unused.h>

#include "vid_ramdac_stg1732_regs.h"

#define STG1732_BASE_CLOCK              12096000.0f

//
// DEFINES
// (local bitflags)
//
#define DAC_IS_STG1764            0x01
#define PALETTE_SIZE                256
#define LUT_SIZE                    768     // R,G,B components

typedef struct stg1732_ramdac_t {
    uint16_t index;
    uint32_t config_0, config_1;            // config registers 
    uint8_t write_pal_addr, read_pal_addr;  // read/write addr's for lut
    uint8_t write_lut_addr, read_lut_addr;  

    // I don't think O factor is used (it's always 1)
    uint8_t vpll_m, vpll_n, vpll_o, vpll_p; // Video PLL
    uint8_t mpll_m, mpll_n, mpll_o, mpll_p; // Core & Memory PLL
    uint8_t apll_m, apll_n, apll_o, apll_p; // Audio PLL

    uint32_t vpll_hz;                       // VPLL clock speed in hertz
    uint32_t mpll_hz;                       // MPLL clock speed in hertz
    uint32_t apll_hz;                       // APLL clock speed in hertz

    uint8_t palette_control;                // controls the DAC state

    uint8_t palette[PALETTE_SIZE];
    uint8_t lut[LUT_SIZE];
    
    uint8_t data;       // need to save this because this is written separately and can receive data from several rgisters
    int     type;
} stg1732_ramdac_t;


void stg1732_set_clock_speed(uint16_t addr, void* priv);
void stg1732_ramdac_set_bpp(svga_t *svga, stg1732_ramdac_t *ramdac);

/* read from internal 64kb register space */
uint8_t stg1732_ramdac_reg_read(uint16_t addr, void *priv, svga_t *svga);
void stg1732_ramdac_reg_write(uint16_t addr, uint8_t val, void *priv, svga_t *svga);

/* Read from UPort */
uint8_t stg1732_ramdac_uport_read(uint8_t addr, void* priv, svga_t* svga);
void stg1732_ramdac_uport_write(uint8_t addr, uint8_t val, void* priv, svga_t* svga);