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
#ifdef EMU_DEVICE_H // what

//TODO: split this all into nv1, nv3, nv4...
#include <86box/timer.h>
#include <86box/vid_svga.h>
#include <86box/vid_svga_render.h>

void nv_log(const char *fmt, ...);

// PCI IDs
#define PCI_VENDOR_NV           0x10DE  // NVidia PCI ID
#define PCI_VENDOR_SGS          0x104A  // SGS-Thompson
#define PCI_VENDOR_SGS_NV       0x12D2  // SGS-Thompson/NVidia joint venture

// 0x0000 was probably the NV0 'Nvidia Hardware Simulator'
#define PCI_DEVICE_NV1          0x0008  // Nvidia NV1
#define PCI_DEVICE_NV1_VGA      0x0009  // Nvidia NV1 VGA core
#define PCI_DEVICE_NV2          0x0010  // Nvidia NV2 / Mutara V08 cancelled
#define PCI_DEVICE_NV3          0x0018  // Nvidia NV3 (Riva 128)
#define PCI_DEVICE_NV3T         0x0019  // Nvidia NV3T (Riva 128 ZX)
#define PCI_DEVICE_NV4          0x0020  // Nvidia NV4 (RIVA TNT)

#define CHIP_REVISION_NV1_A0    0x0000
#define CHIP_REVISION_NV1_B0    0x0010
#define CHIP_REVISION_NV1_C0    0x0020

#define CHIP_REVISION_NV3_A0    0x0000  // January 1997
#define CHIP_REVISION_NV3_B0    0x0010  // October 1997
#define CHIP_REVISION_NV3_C0    0x0020  // 1998

// Architecture ID
#define NV_ARCHITECTURE_NV1   1
#define NV_ARCHITECTURE_NV2   2
#define NV_ARCHITECTURE_NV3   3

typedef enum nv_bus_generation_e
{
    // NV1
    // NV3
    nv_bus_pci = 0,

    // NV3
    nv_bus_agp_1x = 1,

    // NV3T
    // NV4
    nv_bus_agp_2x = 2,

} nv_bus_generation;

// NV Base
typedef struct nv_base_s
{
    mem_mapping_t mmio; // mmio mapping (16mb ) 
    rom_t vbios;        // NVIDIA VBIOS
    svga_t svga;
    uint8_t pci_slot;
    nv_bus_generation bus_generation;
    
} nv_base_t;

// Master Control
typedef struct nv3_pmc_s
{
    int32_t pmc_enable; // Determines which subsystems are enabled.

} nv3_pmc_t;

// add enums for eac
// Chip configuration
typedef struct nv3_straps_s
{

} nv3_straps_t;

// Framebuffer
typedef struct nv3_pfb_s
{

} nv3_pfb_t;

// Bus Configuration
typedef struct nv3_pbus_s
{

} nv3_pbus_t;

// Graphics object hashtable
typedef struct nv3_pfifo_ramht_s
{

} nv3_pfifo_ramht_t;

// Anti-fuckup device
typedef struct nv3_pfifo_ramro_s
{

} nv3_pfifo_ramro_t;

// context for unused channels
typedef struct nv3_dfifo_ramfc_s
{

} nv3_pfifo_ramfc_t;

// ????? ram auxillary
typedef struct nv_pfifo_ramau_s
{

} nv3_pfifo_ramau_t;

// Command submission to PGRAPH
typedef struct nv_pfifo_s
{

} nv3_pfifo_t;

// RAMDAC
typedef struct nv3_pramdac_s
{

} nv3_pramdac_t;

// Graphics Subsystem
typedef struct nv3_pgraph_s
{

} nv3_pgraph_t;

// Graphics Subsystem
typedef struct nv3_pstraps_s
{

} nv3_pstraps_t;

typedef struct nv3_s
{
    nv_base_t nvbase;   // Base Nvidia structure
    
    // Config
    nv3_straps_t nvstraps;

    // Subsystems
    nv3_pmc_t pmc;               // Master Control
    nv3_pfb_t pfb;               // Framebuffer/VRAM
    nv3_pbus_t pbus;             // Bus Control
    nv3_pfifo_t pfifo;           // FIFO for command submisison
    nv3_pfifo_ramht_t ramht;     // hashtable for PGRAPH objects
    nv3_pfifo_ramro_t ramro;     // anti-fuckup mechanism for idiots who fucked up the FIFO submission
    nv3_pfifo_ramfc_t ramfc;     // context for unused channels
    nv3_pfifo_ramau_t ramau;     // auxillary weirdnes
    nv3_pramdac_t pramdac;       // RAMDAC (CLUT etc)
    nv3_pgraph_t pgraph;         // 2D/3D Graphics
    nv3_pstraps_t pstraps;       // Chip configuration

    //more here

} nv3_t;

// device objects
extern nv3_t* nv3;

// NV3
void*   nv3_init(const device_t *info);

void    nv3_close(void* priv);
void    nv3_speed_changed(void *priv);
void    nv3_force_redraw(void* priv);
#endif