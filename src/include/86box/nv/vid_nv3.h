/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          JENSEN HUANG APPROVED !!!!
 *
 *
 *
 * Authors: Connor Hyde <mario64crashed@gmail.com>
 *
 *          Copyright 2024 Connor Hyde
 */

// vid_nv3.h: NV3 Architecture Hardware Reference (open-source)
// Last updated     23 October 2024

// PCI config

// Master Control

#define PMC_BOOT                                0x0 
#define PMC_INTERRUPT                           0x100
#define PMC_ENABLE                              0x200

// CRTC/CIO (0x3b0-0x3df)

#define NV3_CRTC_DATA_OUT                       0x3C0
#define NV3_CRTC_MISCOUT                        0x3C2

// These are standard (0-18h)
#define NV3_CRTC_REGISTER_HTOTAL                0x00
#define NV3_CRTC_REGISTER_HDISPEND              0x01
#define NV3_CRTC_REGISTER_HBLANKSTART           0x02
#define NV3_CRTC_REGISTER_HBLANKEND             0x03
#define NV3_CRTC_REGISTER_HRETRACESTART         0x04
#define NV3_CRTC_REGISTER_HRETRACEEND           0x05
#define NV3_CRTC_REGISTER_VTOTAL                0x06
#define NV3_CRTC_REGISTER_OVERFLOW              0x07
#define NV3_CRTC_REGISTER_PRESETROWSCAN         0x08
#define NV3_CRTC_REGISTER_MAXSCAN               0x09
#define NV3_CRTC_REGISTER_CURSOR_START          0x0A
#define NV3_CRTC_REGISTER_CURSOR_END            0x0B
#define NV3_CRTC_REGISTER_STARTADDR_HIGH        0x0C
#define NV3_CRTC_REGISTER_STARTADDR_LOW         0x0D
#define NV3_CRTC_REGISTER_CURSORLOCATION_HIGH   0x0E
#define NV3_CRTC_REGISTER_CURSORLOCATION_LOW    0x0F
#define NV3_CRTC_REGISTER_VRETRACESTART         0x10
#define NV3_CRTC_REGISTER_VRETRACEEND           0x11
#define NV3_CRTC_REGISTER_VDISPEND              0x12
#define NV3_CRTC_REGISTER_OFFSET                0x13
#define NV3_CRTC_REGISTER_UNDERLINELOCATION     0x14
#define NV3_CRTC_REGISTER_STARTVBLANK           0x15
#define NV3_CRTC_REGISTER_ENDVBLANK             0x16
#define NV3_CRTC_REGISTER_CRTCCONTROL           0x17
#define NV3_CRTC_REGISTER_LINECOMP              0x18
#define NV3_CRTC_REGISTER_STANDARDVGA_END       0x18

// These are nvidia (25-63)
#define NV3_CRTC_REGISTER_READ_BANK             0x1D
#define NV3_CRTC_REGISTER_WRITE_BANK            0x1E
#define NV3_CRTC_REGISTER_FORMAT                0x25
#define NV3_CRTC_REGISTER_PIXELMODE             0x28

// Register value defines start here 

//todo: pixel format

#define NV3_CRTC_REGISTER_PIXELMODE_4BPP        0x00 // vga 16-colours?
#define NV3_CRTC_REGISTER_PIXELMODE_8BPP        0x01
#define NV3_CRTC_REGISTER_PIXELMODE_16BPP       0x02
#define NV3_CRTC_REGISTER_PIXELMODE_32BPP       0x03 

// PRAMDAC
#define NV3_PRAMDAC_CLOCK_MEMORY                0x680504

#define NV3_PRAMDAC_CLOCK_MEMORY_VDIV           7:0
#define NV3_PRAMDAC_CLOCK_MEMORY_NDIV           15:8
#define NV3_PRAMDAC_CLOCK_MEMORY_PDIV           18:16
#define NV3_PRAMDAC_CLOCK_PIXEL                 0x680508
