/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Provides NV4 configuration
 *
 *
 * Authors: Connor Hyde, <mario64crashed@gmail.com> I need a better email address ;^)
 *
 *          Copyright 2024-2025 starfrost
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <86box/86box.h>
#include <86box/device.h>
#include <86box/mem.h>
#include <86box/io.h>
#include <86box/pci.h>
#include <86box/rom.h> // DEPENDENT!!!
#include <86box/video.h>
#include "nv1.h"
#include "nv1_regs.h"

const device_config_t nv1_config[] =
{
    // Memory configuration
    {
        .name = "vram_size",
        .description = "VRAM Size",
        .type = CONFIG_SELECTION,
        .default_int = NV1_VRAM_SIZE_4MB,
        .selection = 
        {
            // I thought this was never released, but it seems that at least one was released:
            // The card was called the "NEC G7AGK"
            {
                .description = "1 MB",
                .value = NV1_VRAM_SIZE_1MB,
            },
            {
                .description = "2 MB",
                .value = NV1_VRAM_SIZE_2MB,
            },
            {
                .description = "4 MB",
                .value = NV1_VRAM_SIZE_4MB,
            },
        }

    },
    { 
        .type = CONFIG_END,
    }
};