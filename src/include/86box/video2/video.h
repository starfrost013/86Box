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
 *          Notes:
 *              - All graphics data is converted to 32-bit on blit in order to simplify logic. The ov 
 *              - Palette stuff is handled by the graphics hardware emulation implementation
 *              
 *
 * Authors: Connor Hyde / starfrost, <mario64crashed@gmail.com>
 *
 *          Copyright 2025 Connor Hyde
 */

#pragma once
#include <stdint.h>

// Windows 98 supports a maximum of 9 monitors.
// This should not be practically used by 86Box, but cards like the STB MVP Pro-128 (NV3/Riva128) use 4. 
#define MAX_MONITORS        9

typedef struct video_monitor_settings_s
{    
    uint32_t size_x;
    uint32_t size_y;

    uint32_t bpp;
    uint32_t hz; 

} video_monitor_settings_t; 

typedef struct video_monitor_s
{
    video_monitor_settings_t settings; 

    double scale_factor; 
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