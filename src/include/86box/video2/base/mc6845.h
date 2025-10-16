/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Motorola MC6845 core defines
 *
 * Authors: Connor Hyde, <mario64crashed@gmail.com>
 * 
 *          Copyright 2025 Connor Hyde.
 */

//
// Defines
//

// there are 32 internal registers on the original 6845, but only 19 are used. But many other CRTC's have many more so we'll allocate 256
#define CRTC_NUM_REGISTERS                              256

// https://stardot.org.uk/mirrors/www.bbcdocs.com/filebase/hardware/datasheets/MC6845-AD1465.pdf
typedef enum mc6845_crtc_e
{
    mc6845_crtc_htotal = 0x00,                          // Total number of horizontal pixels
    mc6845_crtc_hdisp = 0x01,                           // Total number of *displayed* horizontal pixels
    mc6845_crtc_hsyncpos = 0x02,                        // Horizontal position of hsync
    mc6845_crtc_hsyncwidth = 0x03,                      // Width (in pixels) of hsync
    mc6845_crtc_vtotal = 0x04,                          // Total number of vertical lines
    mc6845_crtc_vtotaladj = 0x05,                       // Adjust added for vtotal to be an exact 50 or 60 Hz CRT raster signal
    mc6845_crtc_vdisp = 0x06,                           // Total number of *actually displayed* vertical lines
    mc6845_crtc_vsyncpos = 0x07,                        // Position of the beginning of the vertical sync interval
    mc6845_crtc_interlace = 0x08,                       // Interlace and raster mode select
    mc6845_crtc_maxscanlineaddr = 0x09,                 // Number of scan lines per character row (max addr - 1)
    mc6845_crtc_curstart = 0x0A,                        // bit 6 - blink enable; bit 5 - blink frequency; bits 4:0 - Cursor start scanline; 
    mc6845_crtc_curend = 0x0B,                          // Cursor end scanline
    mc6845_crtc_startaddrhigh = 0x0C,                   // 14-bit start address for display after vblank (high 6 bits)
    mc6845_crtc_startaddrlow = 0x0D,                    // 14-bit start address for display after vblank (low 8 bits)
    mc6845_crtc_curaddrhigh = 0x0E,                     // 14-bit start address for cursor (high 6 bits)
    mc6845_crtc_curaddrlow = 0x0F,                      // 14-bit start address for cursor (low 8 bits)
    mc6845_crtc_lightpenaddrhigh = 0x10,                // 14-bit start address for light pen touch (high 6 bits)
    mc6845_crtc_lightpenaddrlow = 0x11,                 // 14-bit start address for light pen touch (low 8 bits)
} mc6845_crtc; 

// I/O registers for MC-6845 on IBM PC architecture machines
typedef enum mc6845_registers_e
{
    mc6845_crtc_index_mono = 0x3B4, 
    mc6845_crtc_data_mono = 0x3B5,
    mc6845_crtc_index_color = 0x3D4,
    mc6845_crtc_data_color = 0x3D5,
} mc6845_registers;