/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          NV3 PBUS: 128-bit unified bus
 *
 *
 *
 * Authors: Connor Hyde, <mario64crashed@gmail.com> I need a better email dataess ;^)
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


// NV3 PBUS RMA - Real Mode Access for VBIOS
uint8_t nv3_pbus_rma_read(uint16_t addr)
{
    addr &= 0xFF;
    uint32_t real_final_address;
    uint8_t ret;

    switch (addr)
    {
        case 0x00:
            ret = NV3_RMA_SIGNATURE_MSB;
            break;
        case 0x01:
            ret = NV3_RMA_SIGNATURE_BYTE2;
            break;
        case 0x02:
            ret = NV3_RMA_SIGNATURE_BYTE1;
            break;       
        case 0x03:
            ret = NV3_RMA_SIGNATURE_LSB;
            break;
        default:
            // reads must be dword aligned
            real_final_address = nv3->pbus.rma.addr + (addr & 0x03) & (NV3_MMIO_SIZE - 1);

            if (nv3->pbus.rma.addr < NV3_MMIO_SIZE) 
                ret = nv3_mmio_read8(nv3->pbus.rma.addr, NULL);
            else // failsafe code, i don't think you will ever write outsife 
                ret = svga_read_linear(nv3->pbus.rma.addr & 0xFFFFFF, &nv3->nvbase.svga);

            nv_log("NV3: MMIO Real Mode Access read, initial address=0x%04x final RMA MMIO address=0x%08x data=0x%08x\n",
                addr,  nv3->pbus.rma.addr, ret);

            break;
    }

    return ret; 
}

// Implements a 32-bit write using 16 bit port number
void nv3_pbus_rma_write(uint16_t addr, uint8_t val)
{
    uint8_t ret = 0x00;

    // addresses are in reality 8bit so just mask it to be safe
    addr &= 0xFF;

    // format:
    // 0x00     ID
    // 0x04     Pointer to data
    // 0x08     Data port(?) 
    // 0x0B     Data - 32bit. SENT IN THE RIGHT ORDER FOR ONCE WAHOO!
    // 0x10     Increment (?) data - implemented the same as data for now 

    if (addr < 0x08)
    {
        switch (addr % 0x04)
        {
            case 0x00: // lowest byte
                nv3->pbus.rma.addr &= ~0xff;
                nv3->pbus.rma.addr |= val;
                break;
            case 0x01: // 2nd highest byte
                nv3->pbus.rma.addr &= ~0xff00;
                nv3->pbus.rma.addr |= (val << 8);
                break;
            case 0x02: // 3rd highest byte
                nv3->pbus.rma.addr &= ~0xff0000;
                nv3->pbus.rma.addr |= (val << 16);
                break;
            case 0x03: // 4th highest byte 
                nv3->pbus.rma.addr &= ~0xff000000;
                nv3->pbus.rma.addr |= (val << 24);
                break;
        }
    }
    // Data to send to MMIO
    else
    {
        switch (addr % 0x04)
        {
            case 0x00: // lowest byte
                nv3->pbus.rma.data &= ~0xff;
                nv3->pbus.rma.data |= val;
                break;
            case 0x01: // 2nd highest byte
                nv3->pbus.rma.data &= ~0xff00;
                nv3->pbus.rma.data |= (val << 8);
                break;
            case 0x02: // 3rd highest byte
                nv3->pbus.rma.data &= ~0xff0000;
                nv3->pbus.rma.data |= (val << 16);
                break;
            case 0x03: // 4th highest byte 
                nv3->pbus.rma.data &= ~0xff000000;
                nv3->pbus.rma.data |= (val << 24);
                break;
        }
    }


    // Sent when each bit size fo the 
    if (addr == 0x0b
    || addr == 0x0f
    || addr == 0x13
    || addr == 0x17)
    {
        nv_log("NV3: MMIO Real Mode Access write transaction complete, initial data=0x%04x val=%04x final RMA MMIO address=0x%08x data=0x%08x\n",
        addr, val, nv3->pbus.rma.addr, nv3->pbus.rma.data);

        if (nv3->pbus.rma.addr < NV3_MMIO_SIZE) 
            nv3_mmio_write32(nv3->pbus.rma.addr, nv3->pbus.rma.data, NULL);
        else // failsafe code, i don't think you will ever write outsife 
            svga_writel_linear(nv3->pbus.rma.addr & 0xFFFFFF, nv3->pbus.rma.data, &nv3->nvbase.svga);
    }

    if (addr & 0x10)
        nv3->pbus.rma.addr += 0x04; // lignment
}