/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          WELCOME TO NVIDIA DRIVER VERSION 2.0!
 *          nv1_pmc: Master Control
 *
 * Authors: starfrost
 *
 *          Copyright 2024-2026 starfrost
 */

#include "../nv1.h"

// Master control read functon
uint32_t nv1_pmc_read(uint32_t addr)
{
    uint32_t ret = 0x00;

    switch (addr)
    {
        case NV_PMC_BOOT_0: 
            ret = NV1_PMC_BOOT_0_GENERIC_REVB2;
            break;
        // TODO handle
        case NV_PMC_INTR_0:
            ret = nv1->pmc.intr;
            break;
        case NV_PMC_INTR_EN_0:
            ret = nv1->pmc.intr_en;
            break;
        case NV_PMC_ENABLE:
            ret = nv1->pmc.enable;
            break;
    }

    return ret; 
}

// Master control writes
void nv1_pmc_write(uint32_t addr, uint32_t val)
{
    // TODO: handle

    switch (addr)
    {
        case NV_PMC_INTR_0:
            nv1->pmc.intr = val;
            break;
        case NV_PMC_INTR_EN_0:
            nv1->pmc.intr_en = val;
            break;
        case NV_PMC_ENABLE:
            nv1->pmc.enable = val;
            break;
    }
}