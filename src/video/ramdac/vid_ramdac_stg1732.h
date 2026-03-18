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


/* read from internal 64kb register space */
uint8_t stg1732_ramdac_reg_read(uint16_t addr, void *priv, svga_t *svga);
void stg1732_ramdac_reg_write(uint16_t addr, uint8_t val, void *priv, svga_t *svga);

/* Read from UPort */
uint8_t stg1732_ramdac_uport_read(uint8_t addr, void* priv, svga_t* svga);
void stg1732_ramdac_uport_write(uint8_t addr, uint8_t val, void* priv, svga_t* svga);