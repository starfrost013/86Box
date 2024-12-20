/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          NV3 PMC - Master control for the chip
 *
 *
 *
 * Authors: Connor Hyde, <mario64crashed@gmail.com> I need a better email address ;^)
 *
 *          Copyright 2024 starfrost
 */

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

// Single unified write function...

void nv3_pmc_init()
{
    nv_log("NV3: Initialising PMC....\n");

    if (nv3->nvbase.gpu_revision == NV3_PCI_CFG_REVISION_A00)
        nv3->pmc.boot = NV3_BOOT_REG_REV_A00;
    else if (nv3->nvbase.gpu_revision == NV3_PCI_CFG_REVISION_B00)
        nv3->pmc.boot = NV3_BOOT_REG_REV_B00;
    else 
        nv3->pmc.boot = NV3_BOOT_REG_REV_C00;

    nv3->pmc.interrupt_enable = NV3_PMC_INTERRUPT_ENABLE_HARDWARE | NV3_PMC_INTERRUPT_ENABLE_SOFTWARE;

    nv_log("NV3: Initialising PMC: Done\n");
}

//
// ****** PMC register list START ******
//

nv_register_t pmc_registers[] = {
    { NV3_PMC_BOOT, "PMC: Boot Manufacturing Information", NULL, NULL },
    { NV3_PMC_INTERRUPT_STATUS, "PMC: Current Pending Subsystem Interrupts", NULL, NULL},
    { NV3_PMC_INTERRUPT_ENABLE, "PMC: Global Interrupt Enable", NULL, NULL,},
    { NV3_PMC_ENABLE, "PMC: Global Subsystem Enable", NULL, NULL },
    { NV_REG_LIST_END, NULL, NULL, NULL}, // sentinel value 
};

uint32_t nv3_pmc_clear_interrupts()
{
    nv_log("NV3: Clearing IRQs\n");
    pci_set_irq(nv3->nvbase.pci_slot, PCI_INTA, &nv3->nvbase.pci_irq_state);
}

// Handle hardware interrupts
// We only clear when we need to, in other functions...
uint32_t nv3_pmc_handle_interrupts(bool send_now)
{
    // TODO:
    // PGRAPH DMA INTR_EN (there is no DMA engine yet)
    // PRM Real-Mode Compatibility Interrupts

    uint32_t new_intr_value;

    // set the new interrupt value
    // PAUDIO not used
    // IF NV3 REV A EMULATION IS ADDED, ADD THIS COMPONENT!
    
    // the registers are designed to line up so you can enable specific interrupts

    // Check Mediaport interrupts
    if (nv3->pme.interrupt_status & nv3->pme.interrupt_enable)
        new_intr_value |= (NV3_PMC_INTERRUPT_PMEDIA_PENDING << NV3_PMC_INTERRUPT_PMEDIA);

    // Check FIFO interrupts
    if (nv3->pfifo.interrupt_status & nv3->pfifo.interrupt_enable)
        new_intr_value |= (NV3_PMC_INTERRUPT_PFIFO_PENDING << NV3_PMC_INTERRUPT_PFIFO);

    // PFB interrupt is VBLANK PGRAPH interrupt...what nvidia...clean this up once we verify it
    if (nv3->pgraph.interrupt_status_0 & (1 << 8))
        new_intr_value |= (NV3_PMC_INTERRUPT_PFB_PENDING << NV3_PMC_INTERRUPT_PFB);
    else if (nv3->pgraph.interrupt_status_0 & ~(1 << 8)) // otherwise PGRAPH-0 interurpt
        new_intr_value |= (NV3_PMC_INTERRUPT_PGRAPH0_PENDING << NV3_PMC_INTERRUPT_PGRAPH0);

    // Check second pgraph interrupt register
    if (nv3->pgraph.interrupt_status_1 & nv3->pgraph.interrupt_enable_1)
        new_intr_value |= (NV3_PMC_INTERRUPT_PGRAPH1_PENDING << NV3_PMC_INTERRUPT_PGRAPH1);

    // check video overlay interrupts
    if (nv3->pvideo.interrupt_status & nv3->pvideo.interrupt_enable)
        new_intr_value |= (NV3_PMC_INTERRUPT_PVIDEO_PENDING << NV3_PMC_INTERRUPT_PVIDEO);

    // check PIT interrupts
    if (nv3->ptimer.interrupt_status & nv3->ptimer.interrupt_enable)
        new_intr_value |= (NV3_PMC_INTERRUPT_PTIMER_PENDING << NV3_PMC_INTERRUPT_PTIMER);

    // check bus interrupts
    if (nv3->pbus.interrupt_status & nv3->pbus.interrupt_enable)
        new_intr_value |= (NV3_PMC_INTERRUPT_PBUS_PENDING << NV3_PMC_INTERRUPT_PBUS);

    // check SW interrupts
    if (nv3->pmc.interrupt_status & (1 << NV3_PMC_INTERRUPT_SOFTWARE))
        new_intr_value |= (NV3_PMC_INTERRUPT_SOFTWARE_PENDING << NV3_PMC_INTERRUPT_SOFTWARE);

    nv3->pmc.interrupt_status = new_intr_value;

    // ***TODO: DOes INTR still change if INTR_EN=0???***
    // If interrupts are disabled don't bother

    if (!nv3->pmc.interrupt_enable)
    {
        nv3_pmc_clear_interrupts();
        return nv3->pmc.interrupt_status;
    }
        

    // if we actually need to send the interrupt (i.e. this is a write) send it now
    if (send_now)
    {
        // no interrupts to send
        if (!(nv3->pmc.interrupt_status)
         || !(nv3->pmc.interrupt_status - 0x80000000))
        {
            nv3_pmc_clear_interrupts();
            return nv3->pmc.interrupt_status;
        }
            
        if ((nv3->pmc.interrupt_status & 0x7FFFFFFF))
        {
            if (nv3->pmc.interrupt_enable & NV3_PMC_INTERRUPT_ENABLE_HARDWARE)
            {
                nv_log("NV3: Firing hardware-originated interrupt NV3_PMC_INTR_0=0x%08x\n", nv3->pmc.interrupt_status);
                pci_set_irq(nv3->nvbase.pci_slot, PCI_INTA, &nv3->nvbase.pci_irq_state);
            }
            else
                nv_log("NV3: NOT firing hardware-originated interrupt NV3_PMC_INTR_0=0x%08x, BECAUSE HARDWARE INTERRUPTS ARE DISABLED\n", nv3->pmc.interrupt_status);      
        }
        else   
        {
            if (nv3->pmc.interrupt_enable & NV3_PMC_INTERRUPT_ENABLE_SOFTWARE)
            {
                nv_log("NV3: Firing software-originated interrupt NV3_PMC_INTR_0=0x%08x\n", nv3->pmc.interrupt_status);
                pci_set_irq(nv3->nvbase.pci_slot, PCI_INTA, &nv3->nvbase.pci_irq_state);
            }
            else
                nv_log("NV3: NOT firing software-originated interrupt NV3_PMC_INTR_0=0x%08x, BECAUSE SOFTWARE INTERRUPTS ARE DISABLED\n", nv3->pmc.interrupt_status); 
        }
    }

    return nv3->pmc.interrupt_status;
}



//
// ****** Read/Write functions start ******
//

uint32_t nv3_pmc_read(uint32_t address) 
{ 
    nv_register_t* reg = nv_get_register(address, pmc_registers, sizeof(pmc_registers)/sizeof(pmc_registers[0]));

    // todo: friendly logging
    nv_log("NV3: PMC Read from 0x%08x", address);

    // if the register actually exists
    if (reg)
    {
        if (reg->friendly_name)
            nv_log(": %s\n", reg->friendly_name);
        else   
            nv_log("\n");

        // on-read function
        if (reg->on_read)
            return reg->on_read();
        else
        {
            switch (reg->address)
            {
                case NV3_PMC_BOOT:          
                    return nv3->pmc.boot;
                case NV3_PMC_INTERRUPT_STATUS:
                    nv3_pmc_clear_interrupts();

                    return nv3_pmc_handle_interrupts(false);
                case NV3_PMC_INTERRUPT_ENABLE:
                    //TODO: ACTUALLY CHANGE THE INTERRUPT STATE
                    return nv3->pmc.interrupt_enable;
                case NV3_PMC_ENABLE:
                    return nv3->pmc.enable;

            }
        }
    }

    return 0x0; 
}

void nv3_pmc_write(uint32_t address, uint32_t value) 
{
    nv_register_t* reg = nv_get_register(address, pmc_registers, sizeof(pmc_registers)/sizeof(pmc_registers[0]));

    nv_log("NV3: PMC Write 0x%08x -> 0x%08x", value, address);

    // if the register actually exists...
    if (reg)
    {
        if (reg->friendly_name)
            nv_log(": %s\n", reg->friendly_name);
        else   
            nv_log("\n");

        // ... call its on-write function
        if (reg->on_write)
            reg->on_write(value);
        else
        {
            // if it doesn't have one fallback to a switch statement
            switch (reg->address)
            {
                case NV3_PMC_INTERRUPT_STATUS:
                    // This can only be done by software interrupts...
                    if (!(nv3->pmc.interrupt_status & 0x7FFFFFFF))
                    {
                        nv_log("Huh? This is a hardware interrupt...Please use the INTR_EN registers of the GPU subsystem you want to trigger "
                        " an interrupt on, rather than writing to NV3_PMC_INTERRUPT_STATUS (Or this is a bug)...NV3_PMC_INTERRUPT_STATUS=0x%08x)\n", nv3->pmc.interrupt_enable);
                        return; 
                    }
                    
                    nv3_pmc_handle_interrupts(true);
                    nv3->pmc.interrupt_status = value;
                    break;
                case NV3_PMC_INTERRUPT_ENABLE:
                    nv3->pmc.interrupt_enable = value & 0x03;
                    nv3_pmc_handle_interrupts(value != 0);
                    break;
                case NV3_PMC_ENABLE:
                    nv3->pmc.enable = value;
                    break;
            }
        }
    }
}