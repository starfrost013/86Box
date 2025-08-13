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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <86box/86box.h>
#include <86box/device.h>
#include <86box/timer.h>


// Windows 98 supports a maximum of 9 monitors.
// This should not be practically filled up by 86Box, but cards like the STB MVP Pro-128 (NV3/Riva128) use 4. 
#define MAX_MONITORS        9
#define MAX_DEVICES         2

#define PALETTE_SIZE        256

// Defines blit modes
typedef struct video_blit_info_s
{
    uint32_t buffer_start;
    uint32_t buffer_pitch;
    uint32_t start_x;
    uint32_t start_y;

    uint32_t size_x;
    uint32_t size_y; 
} video_blit_info_t;

// Enumerates possible memory to memory blit modes.
// (More nvidia stuff)
typedef enum mem2mem_blit_mode_e
{
    buffer0_to_buffer1_pitch = 0,
    buffer0_to_buffer0_pitch = 1,
    buffer1_to_buffer0_pitch = 2,
    buffer1_to_buffer1_pitch = 3,
} mem2mem_blit_mode; 

typedef struct mem2mem_blit_info_s
{
    uint32_t buffer0_start;
    uint32_t buffer0_pitch;
    uint32_t buffer1_start;
    uint32_t buffer1_pitch; 

    uint32_t buffer0_start_x;
    uint32_t buffer0_start_y;
    uint32_t buffer1_start_x;
    uint32_t buffer1_start_y;

    // TODO: Allow stretching by specifying different swizes

    uint32_t size_x;
    uint32_t size_y; 

    mem2mem_blit_mode mode;
} mem2mem_blit_info_t;

typedef struct video_device_s
{
    device_t* device;  
    uint64_t pixel_clock;

} video_device_t;

typedef struct video_monitor_settings_s
{    
    uint32_t size_x;
    uint32_t size_y;

    uint32_t bpp;
    uint32_t hz; 

    // Characters per clock for blit_character_clock mode
    uint32_t characters_per_clock;

} video_monitor_settings_t; 

typedef struct video_monitor_s
{
    video_monitor_settings_t settings; 

    event_t blit_event; 

    double scale_factor; 

    uint32_t palette_indexed_4bpp[PALETTE_SIZE];            
    uint32_t palette_indexed_8bpp[PALETTE_SIZE];            // I8
    uint32_t palette_indexed_16bpp[PALETTE_SIZE];           // I16 (Riva 128, ???????)

} video_monitor_t; 

extern video_monitor_t video_monitors[MAX_MONITORS];
extern video_device_t video_devices[MAX_DEVICES];
extern uint32_t num_monitors;
extern uint32_t num_devices;

// Functions
void Video_Init();
void Video_AddDevice(device_t* video);
video_monitor_t* Video_AddMonitor(video_monitor_settings_t settings);
void Video_SetMonitorSize(video_monitor_t* monitor, uint32_t size_x, uint32_t size_y);
void Video_SetBPP(video_monitor_t* monitor, uint32_t bpp);
void Video_SetRefreshRate(video_monitor_t* monitor, uint32_t hz);
void Video_DestroyMonitor(video_monitor_t* monitor);

void Video_Blit_MemToScreen(video_blit_info_t blit_info);
void Video_Blit_MemToMem(mem2mem_blit_info_t blit_info);

void Video_Reset();
void Video_Shutdown();

uint32_t Video_DefaultConvert8to32(uint8_t colour);
uint32_t Video_DefaultConvert15to32(uint16_t colour);
uint32_t Video_DefaultConvert16to32(uint16_t colour);

int32_t Video_Calc6to8(int32_t c);
int32_t Video_Calc8to32(int32_t c);
int32_t Video_Calc15to32(int32_t c);
int32_t Video_Calc16to32(int32_t c);
