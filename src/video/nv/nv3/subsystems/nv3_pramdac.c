/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          NV3 bringup and device emulation.
 *
 *
 *
 * Authors: Connor Hyde, <mario64crashed@gmail.com> I need a better email address ;^)
 *
 *          Copyright 2024 starfrost
 */

// nv3_pramdac.c: NV3 RAMDAC
// Todo: Allow overridability using 68050C register...

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

void nv3_pramdac_init()
{
    nv_log("NV3: Initialising PRAMDAC\n");

    // defaults, these come from vbios in reality
    // driver defaults are nonsensical(?), or the algorithm is wrong
    // munged this to 100mhz for now
    nv3->pramdac.memory_clock_m = nv3->pramdac.pixel_clock_m = 0x07;
    nv3->pramdac.memory_clock_n = nv3->pramdac.pixel_clock_n = 0xc8;
    nv3->pramdac.memory_clock_p = nv3->pramdac.pixel_clock_p = 0x0c;


    nv3_pramdac_set_pixel_clock();
    nv3_pramdac_set_vram_clock();

    nv_log("NV3: Initialising PRAMDAC: Done\n");
}

// Polls the pixel clock.
// This updates the 2D/3D engine PGRAPH
void nv3_pramdac_pixel_clock_poll(void* priv)
{
    nv3_t* nv3_poll = (nv3_t*)priv;

    timer_on_auto(&nv3_poll->nvbase.pixel_clock_timer, nv3_poll->nvbase.pixel_clock_period);
}

// Polls the memory clock.
void nv3_pramdac_memory_clock_poll(void* priv)
{
    nv3_t* nv3_poll = (nv3_t*)priv;

    // Let's hope qeeg was right here.
    nv3_ptimer_tick();

    timer_on_auto(&nv3_poll->nvbase.memory_clock_timer, nv3_poll->nvbase.memory_clock_period);
}

// Gets the vram clock register.
uint32_t nv3_pramdac_get_vram_clock_register()
{
    // the clock format is packed into 19 bits
    // M divisor            [7-0]
    // N divisor            [16-8]
    // P divisor            [18-16]
    return (nv3->pramdac.memory_clock_m)
    + (nv3->pramdac.memory_clock_n << 8)
    + (nv3->pramdac.memory_clock_p << 16); // 0-3
}

uint32_t nv3_pramdac_get_pixel_clock_register()
{
    return (nv3->pramdac.pixel_clock_m)
    + (nv3->pramdac.pixel_clock_n << 8)
    + (nv3->pramdac.pixel_clock_p << 16); // 0-3
}

void nv3_pramdac_set_vram_clock_register(uint32_t value)
{
    nv3->pramdac.memory_clock_m = value & 0xFF;
    nv3->pramdac.memory_clock_n = (value >> 8) & 0xFF;
    nv3->pramdac.memory_clock_p = (value >> 16) & 0x07;
    
    nv3_pramdac_set_vram_clock();
}

void nv3_pramdac_set_pixel_clock_register(uint32_t value)
{
    nv3->pramdac.pixel_clock_m = value & 0xFF;
    nv3->pramdac.pixel_clock_n = (value >> 8) & 0xFF;
    nv3->pramdac.pixel_clock_p = (value >> 16) & 0x07;

    nv3_pramdac_set_pixel_clock();
}

void nv3_pramdac_set_vram_clock()
{
    // from driver and vbios source
    float frequency = 13500000.0f;

    // prevent division by 0
    if (nv3->pramdac.memory_clock_m == 0)
        nv3->pramdac.memory_clock_m = 1;
    
    if (nv3->pramdac.memory_clock_n == 0)
        nv3->pramdac.memory_clock_n = 1;
    
    frequency = (frequency * nv3->pramdac.memory_clock_n) / (nv3->pramdac.memory_clock_m << nv3->pramdac.memory_clock_p); 

    double time = (1000000.0 * NV3_86BOX_TIMER_SYSTEM_FIX_QUOTIENT) / (double)frequency; // needs to be a double for 86box

    nv_log("NV3: Memory clock = %.2f MHz\n", frequency / 1000000.0f);    

    nv3->nvbase.memory_clock_period = time;
    
    timer_on_auto(&nv3->nvbase.memory_clock_timer, nv3->nvbase.memory_clock_period);

    //Breaks everything?
    //timer_set_delay_u64(&nv3->nvbase.memory_clock_timer, time * TIMER_USEC); // do we need to decrease
}

void nv3_pramdac_set_pixel_clock()
{
    // frequency divider algorithm from old varcem/86box/pcbox riva driver,
    // verified by reversing NT drivers v1.50e CalcMNP [symbols] function

    // todo: actually implement it

    // missing section
    // not really needed.
    // if (nv3->pfb.boot.clock_crystal == CLOCK_CRYSTAL_13500)
    // {
    //      freq = 13500000.0f;
    // }
    // else 
    //
    // {
    //      freq = 14318000.0f;
    // }

    float frequency = 13500000.0f;

    // prevent division by 0
    if (nv3->pramdac.pixel_clock_m == 0)
        nv3->pramdac.pixel_clock_m = 1;
    
    if (nv3->pramdac.memory_clock_n == 0)
        nv3->pramdac.memory_clock_n = 1;

    frequency = (frequency * nv3->pramdac.pixel_clock_n) / (nv3->pramdac.pixel_clock_m << nv3->pramdac.pixel_clock_p); 

    nv3->nvbase.svga.clock = cpuclock / frequency;

    double time = (1000000.0 * NV3_86BOX_TIMER_SYSTEM_FIX_QUOTIENT) / (double)frequency; // needs to be a double for 86box

    nv_log("NV3: Pixel clock = %.2f MHz\n", frequency / 1000000.0f);

    nv3->nvbase.pixel_clock_period = time;

    timer_on_auto(&nv3->nvbase.pixel_clock_timer, nv3->nvbase.pixel_clock_period);
    //Breaks everything?
    //timer_set_delay_u64(&nv3->nvbase.pixel_clock_timer, time * TIMER_USEC); // do we need to decrease
}

//
// ****** PRAMDAC register list START ******
//

// NULL means handle in read functions
nv_register_t pramdac_registers[] = 
{
    { NV3_PRAMDAC_CLOCK_PIXEL, "PRAMDAC - NV3 GPU Core - Pixel clock", nv3_pramdac_get_pixel_clock_register, nv3_pramdac_set_pixel_clock_register },
    { NV3_PRAMDAC_CLOCK_MEMORY, "PRAMDAC - NV3 GPU Core - Memory clock", nv3_pramdac_get_vram_clock_register, nv3_pramdac_set_vram_clock_register },
    { NV3_PRAMDAC_COEFF_SELECT, "PRAMDAC - PLL Clock Coefficient Select", NULL, NULL},
    { NV3_PRAMDAC_GENERAL_CONTROL, "PRAMDAC - General Control", NULL, NULL },
    { NV3_PRAMDAC_VSERR_WIDTH, "PRAMDAC - Vertical Sync Error Width", NULL, NULL},
    { NV3_PRAMDAC_VEQU_END, "PRAMDAC - VEqu End", NULL, NULL},
    { NV3_PRAMDAC_VBBLANK_END, "PRAMDAC - VBBlank End", NULL, NULL},
    { NV3_PRAMDAC_VBLANK_END, "PRAMDAC - Vertical Blanking Interval End", NULL, NULL},
    { NV3_PRAMDAC_VBLANK_START, "PRAMDAC - Vertical Blanking Interval Start", NULL, NULL},
    { NV3_PRAMDAC_VEQU_START, "PRAMDAC - VEqu Start", NULL, NULL},
    { NV3_PRAMDAC_VTOTAL, "PRAMDAC - Total Vertical Lines", NULL, NULL},
    { NV3_PRAMDAC_HSYNC_WIDTH, "PRAMDAC - Horizontal Sync Pulse Width", NULL, NULL},
    { NV3_PRAMDAC_HBURST_START, "PRAMDAC - Horizontal Burst Signal Start", NULL, NULL},
    { NV3_PRAMDAC_HBURST_END, "PRAMDAC - Horizontal Burst Signal Start", NULL, NULL},
    { NV3_PRAMDAC_HTOTAL, "PRAMDAC - Total Horizontal Lines", NULL, NULL},
    { NV3_PRAMDAC_HEQU_WIDTH, "PRAMDAC - HEqu End", NULL, NULL},
    { NV3_PRAMDAC_HSERR_WIDTH, "PRAMDAC - Horizontal Sync Error", NULL, NULL},
    { NV_REG_LIST_END, NULL, NULL, NULL}, // sentinel value 
};

//
// ****** Read/Write functions start ******
//

uint32_t nv3_pramdac_read(uint32_t address) 
{ 
    nv_register_t* reg = nv_get_register(address, pramdac_registers, sizeof(pramdac_registers)/sizeof(pramdac_registers[0]));

    // todo: friendly logging

    nv_log("NV3: PRAMDAC Read from 0x%08x", address);

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
            //s hould be pretty easy to understand
            switch (reg->address)
            {
                case NV3_PRAMDAC_COEFF_SELECT:
                    return nv3->pramdac.coeff_select;
                case NV3_PRAMDAC_GENERAL_CONTROL:
                    return nv3->pramdac.general_control;
                case NV3_PRAMDAC_VSERR_WIDTH:
                    return nv3->pramdac.vserr_width;
                case NV3_PRAMDAC_VBBLANK_END:
                    return nv3->pramdac.vbblank_end;
                case NV3_PRAMDAC_VBLANK_END:
                    return nv3->pramdac.vblank_end;
                case NV3_PRAMDAC_VBLANK_START:
                    return nv3->pramdac.vblank_start;
                case NV3_PRAMDAC_VEQU_START:
                    return nv3->pramdac.vequ_start;
                case NV3_PRAMDAC_VTOTAL:
                    return nv3->pramdac.vtotal;
                case NV3_PRAMDAC_HSYNC_WIDTH:
                    return nv3->pramdac.hsync_width;
                case NV3_PRAMDAC_HBURST_START:
                    return nv3->pramdac.hburst_start;
                case NV3_PRAMDAC_HBURST_END:
                    return nv3->pramdac.hburst_end;
                case NV3_PRAMDAC_HBLANK_START:
                    return nv3->pramdac.hblank_start;
                case NV3_PRAMDAC_HBLANK_END:
                    return nv3->pramdac.hblank_end;
                case NV3_PRAMDAC_HTOTAL:
                    return nv3->pramdac.htotal;
                case NV3_PRAMDAC_HEQU_WIDTH:
                    return nv3->pramdac.hequ_width;
                case NV3_PRAMDAC_HSERR_WIDTH:
                    return nv3->pramdac.hserr_width;
            }
        }
    }

    return 0x0; 
}

void nv3_pramdac_write(uint32_t address, uint32_t value) 
{
    nv_register_t* reg = nv_get_register(address, pramdac_registers, sizeof(pramdac_registers)/sizeof(pramdac_registers[0]));

    nv_log("NV3: PRAMDAC Write 0x%08x -> 0x%08x", value, address);

    // if the register actually exists
    if (reg)
    {
        if (reg->friendly_name)
            nv_log(": %s\n", reg->friendly_name);
        else   
            nv_log("\n");

        // on-read function
        if (reg->on_write)
            reg->on_write(value);
        else
        {
            //s hould be pretty easy to understand
            // we also update the SVGA state here
            switch (reg->address)
            {
                case NV3_PRAMDAC_COEFF_SELECT:
                    nv3->pramdac.coeff_select = value;
                    break;
                case NV3_PRAMDAC_GENERAL_CONTROL:
                    nv3->pramdac.general_control = value;
                    break;
                case NV3_PRAMDAC_VSERR_WIDTH:
                    //vslines?
                    nv3->pramdac.vserr_width = value;
                    break;
                case NV3_PRAMDAC_VBBLANK_END:
                    nv3->pramdac.vbblank_end = value;
                    break;
                case NV3_PRAMDAC_VBLANK_END:
                    nv3->pramdac.vblank_end = value;
                    break;
                case NV3_PRAMDAC_VBLANK_START:
                    nv3->nvbase.svga.vblankstart = value;
                    nv3->pramdac.vblank_start = value;
                    break;
                case NV3_PRAMDAC_VEQU_START:
                    nv3->pramdac.vequ_start = value;
                    break;
                case NV3_PRAMDAC_VTOTAL:
                    nv3->pramdac.vtotal = value;
                    nv3->nvbase.svga.vtotal = value;
                    break;
                case NV3_PRAMDAC_HSYNC_WIDTH:
                    nv3->pramdac.hsync_width = value;
                    break;
                case NV3_PRAMDAC_HBURST_START:
                    nv3->pramdac.hburst_start = value;
                    break;
                case NV3_PRAMDAC_HBURST_END:
                    nv3->pramdac.hburst_end = value;
                    break;
                case NV3_PRAMDAC_HBLANK_START:
                    nv3->nvbase.svga.hblankstart = value;
                    nv3->pramdac.hblank_start = value;
                    break;
                case NV3_PRAMDAC_HBLANK_END:
                    nv3->nvbase.svga.hblank_end_val = value;
                    nv3->pramdac.hblank_end = value;
                    break;
                case NV3_PRAMDAC_HTOTAL:
                    nv3->pramdac.htotal = value;
                    nv3->nvbase.svga.htotal = value;
                    break;
                case NV3_PRAMDAC_HEQU_WIDTH:
                    nv3->pramdac.hequ_width = value;
                    break;
                case NV3_PRAMDAC_HSERR_WIDTH:
                    nv3->pramdac.hserr_width = value;
                    break;
            }
        }
    }
}