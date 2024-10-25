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
// Last updated     24 October 2024

#define NV3_MMIO_SIZE   0x1000000       // Max MMIO size
#define NV_VBIOS_V15403 "roms/video/nvidia/nv3/VCERAZOR.BIN" //TODO: move to hash system

// Temporary, will be loaded from settings
#define VRAM_SIZE_2MB   0x200000 // 2MB
#define VRAM_SIZE_4MB   0x400000 // 4MB
#define VRAM_SIZE_8MB   0x800000 // NV3T only

// GPU Subsystems
// These most likely correspond to functional blocks in the original design

#define NV3_PMC_START                           0x0         // Chip Master Control Subsystem
#define NV3_PMC_END                             0x0
#define NV3_CIO_START                           0x3b0       // Legacy SVGA Emulation Subsystem
#define NV3_CIO_END                             0x3df
#define NV3_PBUS_START                          0x1000      // Bus Control Subsystem
#define NV3_PBUS_END                            0x1FFF
#define NV3_PFIFO_START                         0x2000      // FIFO for DMA Object Submission (uses hashtable to store the objects)
#define NV3_PFIFO_END                           0x2FFF
#define NV3_PRM_START                           0x4000      // Real-Mode Device Support Subsystem
#define NV3_PRM_END                             0x4FFF
#define NV3_PRMIO_START                         0x7000      // Real-Mode I/O Subsystem
#define NV3_PRMIO_END                           0x7FFF
#define NV3_VGA_VRAM_START                      0xA0000     // VGA Emulation VRAM
#define NV3_VGA_VRAM_END                        0xBFFFF
#define NV3_VGA_START                           0xC0000     // VGA Emulation Registers
#define NV3_VGA_END                             0xC7FFF
#define NV3_PFB_START                           0x100000    // GPU Interface to VRAM
#define NV3_PFB_END                             0x100FFF
#define NV3_PEXTDEV_START                       0x101000    // External Devices
#define NV3_PSTRAPS                             0x101000    // Straps Bits
#define NV3_PEXTDEV_END                         0x101FFF
#define NV3_PROM_START                          0x110000    // VBIOS?
#define NV3_PROM_END                            0x110FFF
#define NV3_PALT_START                          0x120000    // ??? but it exists
#define NV3_PALT_END                            0x120FFF
#define NV3_PME_START                           0x200000    // Mediaport 
#define NV3_PME_END                             0x200FFF
#define NV3_PGRAPH_START                        0x400000    // Scene graph for 2d/3d rendering...the most important part

// not sure about the class ids
// these are NOT what each class is, just ued to manipulate it
#define NV3_PGRAPH_CLASS18_BETA_START           0x410000    // Beta blending factor
#define NV3_PGRAPH_CLASS18_BETA_END             0x411FFF  
#define NV3_PGRAPH_CLASS20_ROP_START            0x420000    // Blending render operation used at final pixel/fragment generation stage
#define NV3_PGRAPH_CLASS20_ROP_END              0x421FFF
#define NV3_PGRAPH_CLASS21_COLORKEY_START       0x430000    // Color key for image
#define NV3_PGRAPH_CLASS21_COLORKEY_END         0x431FFF      
#define NV3_PGRAPH_CLASS22_PLANEMASK_START      0x440000
#define NV3_PGRAPH_CLASS22_PLANEMASK_END        0x441FFF
#define NV3_PGRAPH_CLASSXX_CLIP_START           0x450000
#define NV3_PGRAPH_CLASSXX_CLIP_END             0x451FFF

#define NV3_PGRAPH_CLASS72_D3D5TRI_ZETA_START   0x570000    // Copy a direct3d 5.0 accelerated triangle to the zeta buffer
#define NV3_PGRAPH_CLASS72_D3D5TRI_ZETA_END     0x571FFF    // Copy a direct3d 5.0 accelerated triangle to the zeta buffer


// not done


// PCI config
#define NV3_PCI_CFG_VENDOR_ID                   0x0
#define NV3_PCI_CFG_DEVICE_ID                   0x2
#define NV3_PCI_CFG_CAPABILITIES                0x4
#define NV3_PCI_CFG_REVISION                    0x8

#define NV3_PCI_CFG_ENABLE_VBIOS                0x30
#define NV3_PCI_CFG_VBIOS_BASE                  0x32 ... 0x33
#define NV3_PCI_CFG_VBIOS_BASE_L                0x32
#define NV3_PCI_CFG_VBIOS_BASE_H                0x33

#define NV3_PCI_CFG_REVISION_A00                0x00 // nv3a January 1997 - engineering sample, had NV1 PAUDIO and other minor incompatibilities
#define NV3_PCI_CFG_REVISION_B00                0x10 // nv3b September 1997
#define NV3_PCI_CFG_REVISION_C00                0x20 // todo: verify this - nv3c (nv3t?) / RIVA 128 ZX

#define NV3_PCI_CFG_CLASS_CODE                  0x9

// Master Control

#define NV3_PMC_BOOT                            0x0 
#define NV3_PMC_INTERRUPT                       0x100
#define NV3_PMC_ENABLE                          0x200

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


#define NV3_CRTC_REGISTER_PIXELMODE_VGA         0x00 // vga 16-colours?
#define NV3_CRTC_REGISTER_PIXELMODE_8BPP        0x01
#define NV3_CRTC_REGISTER_PIXELMODE_16BPP       0x02
#define NV3_CRTC_REGISTER_PIXELMODE_32BPP       0x03 

#define NV3_CRTC_REGISTER_RMA                   0x38 // REAL MODE ACCESS!

// for 86box 8bit addressing
// get rid of this asap, replace with 32->8 macros
#define NV3_RMA_SIGNATURE_MSB                   0x65
#define NV3_RMA_SIGNATURE_BYTE2                 0xD0
#define NV3_RMA_SIGNATURE_BYTE1                 0x16
#define NV3_RMA_SIGNATURE_LSB                   0x2B

#define NV3_CRTC_REGISTER_RMA_MODE_MAX          0x0F

// Register value defines start here 

//todo: pixel format


// PRAMDAC
#define NV3_PRAMDAC_CLOCK_MEMORY                0x680504

#define NV3_PRAMDAC_CLOCK_MEMORY_VDIV           7:0
#define NV3_PRAMDAC_CLOCK_MEMORY_NDIV           15:8
#define NV3_PRAMDAC_CLOCK_MEMORY_PDIV           18:16
#define NV3_PRAMDAC_CLOCK_PIXEL                 0x680508
