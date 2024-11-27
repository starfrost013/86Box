/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          The insane NV3 MMIO arbiter.
 *          Writes to ALL sections of the GPU based on the write position
 *          All writes are internally considered to be 32-bit! Be careful...
 * 
 *          Also handles interrupt dispatch
 *
 *          
 *
 * Authors: Connor Hyde, <mario64crashed@gmail.com> I need a better email address ;^)
 *
 *          Copyright 2024 starfrost
 */

// STANDARD NV3 includes
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <86Box/86box.h>
#include <86Box/device.h>
#include <86Box/mem.h>
#include <86box/pci.h>
#include <86Box/rom.h> // DEPENDENT!!!
#include <86Box/video.h>
#include <86Box/nv/vid_nv.h>
#include <86Box/nv/vid_nv3.h>

// Gets a register...
// move this somewhere else when we have more models
nv_register_t* nv_get_register(uint32_t address, nv_register_t* register_list, uint32_t num_regs)
{
    for (int32_t reg_num = 0; reg_num < num_regs; reg_num++)
    {
        if (register_list[reg_num].address == NV_REG_LIST_END)
            break; //unimplemented

        if (register_list[reg_num].address == address)
            return &register_list[reg_num];
    }

    return NULL;
}

// Arbitrates an MMIO read
uint32_t nv3_mmio_arbitrate_read(uint32_t address)
{
    uint32_t ret = 0x00;
    
    // note: some registers are byte aligned not dword aligned
    // only very few are though, so they can be handled specially, using the register list most likely
    address &= 0xFFFFFC;

    // gigantic set of if statements to send the write to the right subsystem
    if (address >= NV3_PMC_START && address <= NV3_PMC_END)
        ret = nv3_pmc_read(address);
    else if (address >= NV3_CIO_START && address <= NV3_CIO_END)
        ret = nv3_cio_read(address);
    else if (address >= NV3_PBUS_PCI_START && address <= NV3_PBUS_PCI_END)
        ret = nv3_pci_read(0x00, address & 0xFF, NULL);
    else if (address >= NV3_PBUS_START && address <= NV3_PBUS_END)
        ret = nv3_pbus_read(address);
    else if (address >= NV3_PFB_START && address <= NV3_PFB_END)
        ret = nv3_pfb_read(address);
    else if (address >= NV3_PRM_START && address <= NV3_PRM_END)
        ret = nv3_prm_read(address);
    else if (address >= NV3_PRMIO_START && address <= NV3_PRMIO_END)
        ret = nv3_prmio_read(address);
    else if (address >= NV3_PTIMER_START && address <= NV3_PTIMER_END)
        ret = nv3_ptimer_read(address);
    else if (address >= NV3_PFB_START && address <= NV3_PFB_END)
        ret = nv3_pfb_read(address);
    else if (address >= NV3_PEXTDEV_START && address <= NV3_PEXTDEV_END)
        ret = nv3_pextdev_read(address);
    else if (address >= NV3_PROM_START && address <= NV3_PROM_END)
        ret = nv3_prom_read(address);
    else if (address >= NV3_PALT_START && address <= NV3_PALT_END)
        ret = nv3_palt_read(address);
    else if (address >= NV3_PME_START && address <= NV3_PME_END)
        ret = nv3_pme_read(address);
    else if (address >= NV3_PGRAPH_START && address <= NV3_PGRAPH_REAL_END) // what we're actually doing here determined by nv3_pgraph_* func
        ret = nv3_pgraph_read(address);
    else if (address >= NV3_PRMCIO_START && address <= NV3_PRMCIO_END)
        ret = nv3_prmcio_read(address);    
    else if (address >= NV3_PVIDEO_START && address <= NV3_PVIDEO_END)
        ret = nv3_pvideo_read(address);
    else if (address >= NV3_PRAMDAC_START && address <= NV3_PRAMDAC_END)
        ret = nv3_pramdac_read(address);
    else if (address >= NV3_VRAM_START && address <= NV3_VRAM_END)
        ret = nv3_vram_read(address);
    else if (address >= NV3_USER_START && address <= NV3_USER_END)
        ret = nv3_user_read(address);
    else if (address >= NV3_PRAMIN_START && address <= NV3_PRAMIN_END)
        ret = nv3_pramin_read(address); // RAMHT, RAMFC, RAMRO etc dettermined by nv3_ramin_* function
    else 
    {
        nv_log("NV3: MMIO read arbitration failed, INVALID address NOT mapped to any GPU subsystem 0x%08x [returning 0x00]\n", address);
        return 0x00;
    }

    nv_log("NV3: MMIO read, 0x%08x <- 0x%08x\n", ret, address);
    return ret;
}

void nv3_mmio_arbitrate_write(uint32_t address, uint32_t value)
{
    // note: some registers are byte aligned not dword aligned
    // only very few are though, so they can be handled specially, using the register list most likely
    address &= 0xFFFFFC;

    // gigantic set of if statements to send the write to the right subsystem
    if (address >= NV3_PMC_START && address <= NV3_PMC_END)
        nv3_pmc_write(address, value);
    else if (address >= NV3_CIO_START && address <= NV3_CIO_END)
        nv3_cio_write(address, value);
    else if (address >= NV3_PBUS_PCI_START && address <= NV3_PBUS_PCI_END)              // PCI mirrored at 0x1800 in MMIO
        nv3_pci_write(0x00, address & 0xFF, value, NULL); // priv does not matter
    else if (address >= NV3_PBUS_START && address <= NV3_PBUS_END)
        nv3_pbus_write(address, value);
    else if (address >= NV3_PFIFO_START && address <= NV3_PFIFO_END)
        nv3_pfifo_write(address, value);
    else if (address >= NV3_PRM_START && address <= NV3_PRM_END)
        nv3_prm_write(address, value);
    else if (address >= NV3_PRMIO_START && address <= NV3_PRMIO_END)
        nv3_prmio_write(address, value);
    else if (address >= NV3_PTIMER_START && address <= NV3_PTIMER_END)
        nv3_ptimer_write(address, value);
    else if (address >= NV3_PFB_START && address <= NV3_PFB_END)
        nv3_pfb_write(address, value);
    else if (address >= NV3_PEXTDEV_START && address <= NV3_PEXTDEV_END)
        nv3_pextdev_write(address, value);
    else if (address >= NV3_PROM_START && address <= NV3_PROM_END)
        nv3_prom_write(address, value);
    else if (address >= NV3_PALT_START && address <= NV3_PALT_END)
        nv3_palt_write(address, value);
    else if (address >= NV3_PME_START && address <= NV3_PME_END)
        nv3_pme_write(address, value);
    else if (address >= NV3_PGRAPH_START && address <= NV3_PGRAPH_REAL_END) // what we're actually doing here is determined by the nv3_pgraph_* functions
        nv3_pgraph_write(address, value);
    else if (address >= NV3_PRMCIO_START && address <= NV3_PRMCIO_END)
        nv3_prmcio_write(address, value);
    else if (address >= NV3_PVIDEO_START && address <= NV3_PVIDEO_END)
        nv3_pvideo_write(address, value);
    else if (address >= NV3_PRAMDAC_START && address <= NV3_PRAMDAC_END)
        nv3_pramdac_write(address, value);
    else if (address >= NV3_VRAM_START && address <= NV3_VRAM_END)
        nv3_vram_write(address, value);
    else if (address >= NV3_USER_START && address <= NV3_USER_END)
        nv3_user_write(address, value);
    else if (address >= NV3_PRAMIN_START && address <= NV3_PRAMIN_END)
        nv3_pramin_write(address, value); // RAMHT, RAMFC, RAMRO etc is determined by the nv3_ramin_* functions
    else 
    {
        nv_log("NV3: MMIO write arbitration failed, INVALID address NOT mapped to any GPU subsystem 0x%08x\n", address);
        return;
    }

    nv_log("NV3: MMIO write, 0x%08x -> 0x%08x\n", value, address);
}


//                                                              //
// ******* DUMMY FUNCTIONS FOR UNIMPLEMENTED SUBSYSTEMS ******* //
//                                                              //

// Read and Write functions for GPU subsystems
// Remove the ones that aren't used here eventually, have all of htem for now
uint32_t    nv3_cio_read(uint32_t address) { return 0; };
void        nv3_cio_write(uint32_t address, uint32_t value) {};
uint32_t    nv3_prm_read(uint32_t address) { return 0; };
void        nv3_prm_write(uint32_t address, uint32_t value) {};
uint32_t    nv3_prmio_read(uint32_t address) { return 0; };
void        nv3_prmio_write(uint32_t address, uint32_t value) {};
uint32_t    nv3_ptimer_read(uint32_t address) { return 0; };
void        nv3_ptimer_write(uint32_t address, uint32_t value) {};
uint32_t    nv3_prom_read(uint32_t address) { return 0; };
void        nv3_prom_write(uint32_t address, uint32_t value) {};
uint32_t    nv3_palt_read(uint32_t address) { return 0; };
void        nv3_palt_write(uint32_t address, uint32_t value) {};

// TODO: PGRAPH class registers
uint32_t    nv3_prmcio_read(uint32_t address) { return 0; };
void        nv3_prmcio_write(uint32_t address, uint32_t value) {};

uint32_t    nv3_vram_read(uint32_t address) { return 0; };
void        nv3_vram_write(uint32_t address, uint32_t value) {};

uint32_t    nv3_user_read(uint32_t address) { return 0; };
void        nv3_user_write(uint32_t address, uint32_t value) {};
uint32_t    nv3_pramin_read(uint32_t address) { return 0; };
void        nv3_pramin_write(uint32_t address, uint32_t value) {};