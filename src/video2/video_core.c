/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Modern video engine
 *
 *          Video timing settings -
 *
 *            8-bit ISA (e.g. CGA/MDA) - 1mb/sec
 *              BYTE = 8 ISA clocks, WORD = 16 ISA clock, DWORD = 32 ISA clocks
 *
 *            Slow 16-bit ISA - 2mb/sec
 *              BYTE = 6 ISA clocks, WORD = 8 ISA clocks, DWORD = 16 ISA clocks
 *
 *            Fast 16-bit ISA - 4mb/sec
 *              BYTE = 3 ISA clocks, WORD = 3 ISA clocks, DWORD = 6 ISA clocks
 *
 *            Slow VLB/PCI - 8mb/sec (ish)
 *              BYTE = 4 bus clocks, WORD = 8 bus clocks, DWORD = 16 bus clocks
 *
 *            Mid VLB/PCI -
 *               BYTE = 4 bus clocks, WORD = 5 bus clocks, DWORD = 10 bus clocks
 *
 *            Fast VLB/PCI -
 *              BYTE = 3 bus clocks, WORD = 3 bus clocks, DWORD = 4 bus clocks
 *  
 *            AGP is theoretically capable of one DWORD per clock. However the GPU will introduce some bottlenecks most likely        
 *
 * Authors: Connor Hyde <mario64crashed@gmail.com>
 *
 *          Copyright 2025 Connor Hyde
 */

#include <86box/video2/video.h>
#include <86box/log.h>

/* Functions only used in this translation unit */
void video_clear(void);

/* Globals only used in this translation unit 1*/

void video_init(void)
{
    video_engine.log = log_open("Video Core");    
}


video_palette_t* video_add_palette(uint8_t bpp, uint32_t num_entries)
{
    video_palette_t* palette = calloc(1, sizeof(video_palette_t));
    palette->entries = calloc(num_entries, sizeof(uint32_t));

    palette->num_entries = num_entries;
    palette->bpp = bpp;

    if (!video_engine.palette_head)
        video_engine.palette_head = video_engine.palette_tail = palette;
    else
    {
        palette->prev = video_engine.palette_tail;
        video_engine.palette_tail->next = palette; 
    }
}

void video_add_monitor(uint32_t size_x, uint32_t size_y)
{
    video_monitor_t* monitor = calloc(1, sizeof(video_monitor_t));
    
    if (size_x == 0
    || size_y == 0)
    {
        fatal("Tried to create a monitor of X or Y size 0.");
    }
    
    monitor->size_x = size_x;
    monitor->size_y = size_y;
    monitor->size_x_scaled = size_x;
    monitor->size_y_scaled = size_y;
    monitor->buffer32 = calloc(1, (size_x * size_y * 4));
    monitor->buffer16 = (uint16_t*)monitor->buffer32;
    monitor->buffer8 = (uint8_t*)monitor->buffer32;

    if (!video_engine.monitor_head)
        video_engine.monitor_head = video_engine.monitor_tail = monitor;
    else
    {
        monitor->prev = video_engine.monitor_tail;
        video_engine.monitor_tail->next = monitor; 
    }

    video_engine.num_monitors++;

    //log_out(video_engine.log, "Added monitor of size %dx%d", size_x, size_y);
}

// DO NOT USE!!!! LEGACY CODE ONLY!!!
video_monitor_t* video_get_monitor_by_index(uint32_t index)
{
    if (index < 0
    || index > video_engine.num_monitors)
    {
        //log_out(video_engine.log, "Tried to get invalid monitor index %d", index);
    }

    video_monitor_t* monitor = video_engine.monitor_head;

    for (uint32_t i = 0; i < index; i++)
        monitor = monitor->next;
}

void video_remove_monitor(video_monitor_t* monitor)
{

}

void video_set_device(device_t* device)
{
    video_engine.video_device = device;
}

void video_blit_screen(void)
{

}

void video_blit_screen_region(video_blit_rect_t rect)
{

}

void video_clear(void)
{

}

void video_reset(void)
{

}

void video_close(void)
{

}