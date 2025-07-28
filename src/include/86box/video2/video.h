/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Header files for Video2 system
 * 
 *          Notes & changes from previous video system:
 *              - *MAY* have it so 32bpp conversion is not necessary. It depends how feasible it is
 *              - Palette stuff is handled by the graphics hardware emulation implementation
 *              - The video device itself controls the monitor setup. 
 *              - Video_AddMonitor in add -> Video_RemoveMonitor in shutdown 
 *              - Rather than vram->target_buffer->qt, it directly blits from vram to qt in order to reduce overhead 
 *                In order to prevent conflicts, you can set the blit frequency and give a pixel clock in order to blit at character, line, or 
 *                frame level. This will allow accelerated graphics hardware to "set and forget" a refresh rate 
 *                (since they don't really operate off of these anyway) while something liek CGA
 *                
 *              
 *
 * Authors: Connor Hyde / starfrost, <mario64crashed@gmail.com>
 *
 *          Copyright 2025 Connor Hyde
 */

#pragma once
#include <stdint.h>

// Windows 98 supports a maximum of 9 monitors.
// This should not be practically filled up by 86Box, but cards like the STB MVP Pro-128 (NV3/Riva128) use 4. 
#define MAX_MONITORS        9

#define PALETTE_SIZE        256

// Defines blit modes
typedef enum video_monitor_blit_frequency_e
{
    blit_on_character_clock = 0,
    blit_on_line_clock = 1,
    blit_on_vblank = 2,
} video_monitor_blit_frequency;

typedef struct video_monitor_settings_s
{    
    uint32_t size_x;
    uint32_t size_y;

    uint32_t bpp;
    uint32_t hz; 

    // Characters per clock for blit_character_clock mode
    uint32_t characters_per_clock;

    uint64_t pixel_clock;

    video_monitor_blit_frequency blit_frequency;

} video_monitor_settings_t; 

typedef struct video_monitor_s
{
    video_monitor_settings_t settings; 

    double scale_factor; 

    uint32_t palette_indexed_4bpp[PALETTE_SIZE];
    uint32_t palette_indexed_8bpp[PALETTE_SIZE];            // I8
    uint32_t palette_indexed_16bpp[PALETTE_SIZE];           // I16 (Riva 128, ???????)

} video_monitor_t; 

extern video_monitor_t monitors[MAX_MONITORS];
extern uint32_t num_monitors;

// Functions
void Video_Init();
video_monitor_t* Video_AddMonitor(video_monitor_settings_t settings);
void Video_SetMonitorSize(uint32_t size_x, uint32_t size_y);
void Video_SetBPP(uint32_t bpp);
void Video_SetRefreshRate(uint32_t hz);
void Video_DestroyMonitor(video_monitor_t* monitor);

uint32_t Video_Default8to32(uint8_t colour);
uint32_t Video_Default15to32(uint16_t colour);
uint32_t Video_Default16to32(uint16_t colour);