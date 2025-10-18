/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Definitions for the video controller module.
 *
 * Authors: Sarah Walker, <https://pcem-emulator.co.uk/>
 *          Miran Grca, <mgrca8@gmail.com>
 *          Fred N. van Kempen, <decwiz@yahoo.com>
 *          Connor Hyde, <mario64crashed@gmail.com>
 *
 *          Copyright 2025 Connor Hyde.
 */

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <86box/86box.h>
#include <86box/device.h>
#include <86box/timer.h>

#define VIDEO_MAX_DEVICES           3       // 2xSLI + 1 card

// Enumerates OpenGL input scale mode types
typedef enum gl_input_scale_mode_type_e 
{
    FULLSCR_SCALE_FULL = 0,
    FULLSCR_SCALE_43,
    FULLSCR_SCALE_KEEPRATIO,
    FULLSCR_SCALE_INT,
    FULLSCR_SCALE_INT43
} gl_input_scale_mode_type;

typedef struct video_timings_s
{
    uint32_t read_byte;
    uint32_t read_word;
    uint32_t read_dword;
    uint32_t write_byte;
    uint32_t write_word;
    uint32_t write_dword;
} video_timings_t;

// Declare a video palette
typedef struct video_palette_s
{
    uint32_t num_entries;
    uint8_t bpp;
    uint32_t* entries;              // index->32-bit colour
    struct video_palette_s* prev;
    struct video_palette_s* next;
} video_palette_t;

// Set up the settings for the video engine for a particular video car.d
typedef struct video_engine_settings_s
{
    bool gpu_accelerated;
    video_palette_t* current_palette;
} video_engine_settings_t;

// Defines a monitor.
typedef struct video_monitor_s
{
    uint32_t size_x;
    uint32_t size_y;
    
    uint32_t size_x_overscan;               // keep qt happy for now
    uint32_t size_y_overscan;               // keep qt happy for now
    uint32_t size_x_scaled;
    uint32_t size_y_scaled;

    // 32-bit is always allocated as the destination format is 32bpp, but this is provided simply for convenience
    uint8_t* buffer8;
    uint16_t* buffer16;
    uint32_t* buffer32;
    struct video_monitor_s* prev;
    struct video_monitor_s* next;
} video_monitor_t;

// Defines a blit region
typedef struct video_blit_rect_s
{
    uint32_t x;
    uint32_t y;
    uint32_t size_x;
    uint32_t size_y; 
} video_blit_rect_t;

// Defines a font. TODO: Move this around? (Only allow 1 font file to be loaded but allow multiple descriptors to contain a font in that file)
typedef struct video_font_s
{
    uint32_t char_size_x;
    uint32_t char_size_y;
    uint32_t num_chars;
    uint8_t* font;
    struct video_monitor_s* prev;
    struct video_monitor_s* next;
} video_font_t;

// Some motherboards need this but the video engine doesn't care
typedef enum video_card_flags_s
{
    VIDEO_CARD_FLAG_MDA = 1 << 0,           // This graphics hardware supports MDA features.
    VIDEO_CARD_FLAG_CGA = 1 << 1,           // This graphics hardware supports CGA features.
    VIDEO_CARD_FLAG_EGA = 1 << 2,           // This graphics hardware supports EGA features.
    VIDEO_CARD_FLAG_PGC = 1 << 3,           // This graphics hardware supports PGC/IM1024 features.
    VIDEO_CARD_FLAG_VGA = 1 << 4,           // This graphics hardware supports VGA features.
    VIDEO_CARD_FLAG_SVGA = 1 << 5,          // This graphics hardware supports some kind of "above VGA features"
    VIDEO_CARD_FLAG_8514 = 1 << 6,          // This graphics hardware supports IBM 8514/A features.
    VIDEO_CARD_FLAG_XGA = 1 << 7,           // This graphics hardware supports XGA features.
    VIDEO_CARD_FLAG_ACCEL = 1 << 8,         // This graphics hardware supports 2D GUI acceleration features beyond XGA level.
    VIDEO_CARD_FLAG_3D = 1 << 9,            // This graphics hardware supports 3D acceleration

    // Special flags
    VIDEO_CARD_FLAG_NV1 = 1 << 29,          // This graphics hardware supports 3D acceleration, but uses quadlilaterals or quadratic texture mapping (QTM) features. (i.e. It is an Nvidia NV1)
    VIDEO_CARD_FLAG_NVIDIA = 1 << 30,       // This graphics hardware uses the NVIDIA object-oriented architecture (therefore, bypass as much of SVGA as possible for perf).
    VIDEO_CARD_FLAG_VOODOO = 1 << 31,       // This graphics hardware requires piping into a second GPU using a passthrough cable.
} video_card_flags_t;

typedef struct video_engine_device_s
{
    device_t* device;
    video_monitor_t* monitor_head;
    video_monitor_t* monitor_tail;
    video_palette_t* palette_head;
    video_palette_t* palette_tail;
    video_font_t* font_head;
    video_font_t* font_tail;
    video_engine_settings_t settings;
    uint32_t num_monitors;                   // Required for QT.
} video_engine_device_t;

// This basically contains everything
typedef struct video_engine_s
{
    video_engine_device_t device[VIDEO_MAX_DEVICES];
    device_t* video_device;                  // Easy way to access the primary device
    video_card_flags_t flags;                // Flags
    void* log;
} video_engine_t;

//
// Globals
//

extern video_engine_t video_engine;
extern const device_t* video_devices[];                                 // The list of video devices.
//
// Functions
//

void video_init(void);
video_palette_t* video_add_palette(uint8_t bpp, uint32_t num_entries);  // Call from GPU
void video_add_monitor(uint32_t size_x, uint32_t size_y);               // Add a monitor.
video_monitor_t* video_get_monitor_by_index(uint32_t index);            // Get a monitor by index. Not recommended, store the object 
                                                                        // pointers somewhere in GPU code. QT shit only
void video_remove_monitor(video_monitor_t* monitor);                    // Remove a monitor.
void video_set_device(device_t* device);

void video_blit_screen(void);                                           // Blit to the entire screen.
void video_blit_screen_region(video_blit_rect_t rect);                  // Blit to a region of the screen.

// Automatically removes any added palettes or monitors.
void video_reset(void);                                                 // Not sure if we need this. Just put it in for 86box.c for now
void video_close(void);

// Other
void video_screenshot(video_monitor_t* monitor);                        // Take a screenshot of a certain monitor.

// Utility functions (TODO: Remove and replace with flag check)
bool video_is_mda();
bool video_is_cga();
bool video_is_ega();
bool video_is_pgc();
bool video_is_vga();
bool video_is_svga();
bool video_is_8514();
bool video_is_xga();
bool video_is_accel();
bool video_is_3d();
bool video_is_nv1();
bool video_is_nvidia();
bool video_is_voodoo();