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

#define NV_PCI_NUM_CFG_REGS     256     // number of pci config registers

// 0x0000 was probably the NV0 'Nvidia Hardware Simulator'
#define PCI_DEVICE_NV1          0x0008  // Nvidia NV1
#define PCI_DEVICE_NV1_VGA      0x0009  // Nvidia NV1 VGA core
#define PCI_DEVICE_NV2          0x0010  // Nvidia NV2 / Mutara V08 (cancelled)
#define PCI_DEVICE_NV3          0x0018  // Nvidia NV3 (Riva 128)
#define PCI_DEVICE_NV3T         0x0019  // Nvidia NV3T (Riva 128 ZX)
#define PCI_DEVICE_NV4          0x0020  // Nvidia NV4 (RIVA TNT)

#define CHIP_REVISION_NV1_A0    0x0000
#define CHIP_REVISION_NV1_B0    0x0010
#define CHIP_REVISION_NV1_C0    0x0020

#define CHIP_REVISION_NV3_A0    0x0000  // January 1997
#define CHIP_REVISION_NV3_B0    0x0010  // October 1997
#define CHIP_REVISION_NV3_C0    0x0020  // 1998

// Architecture IDs
#define NV_ARCHITECTURE_NV1     1
#define NV_ARCHITECTURE_NV2     2
#define NV_ARCHITECTURE_NV3     3


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
    rom_t vbios;                        // NVIDIA/OEm VBIOS
    // move to nv3_cio_t?
    svga_t svga;                        // SVGA core (separate to nv3)
    // stuff that doesn't fit in the svga structure
    uint32_t cio_read_bank;             // SVGA read bank
    uint32_t cio_write_bank;            // SVGA write bank

    mem_mapping_t svga_mapping;         // SVGA core memory mapping
    mem_mapping_t mmio;                 // mmio mapping (32MB unified MMIO) 
    uint8_t pci_slot;                   // pci slot number
    nv_bus_generation bus_generation;
} nv_base_t;

// Master Control
typedef struct nv3_pmc_s
{
    int32_t pmc_enable; // Determines which subsystems are enabled.

} nv3_pmc_t;

typedef struct nv3_pci_config_s
{
    uint8_t pci_regs[NV_PCI_NUM_CFG_REGS];  // The actual pci register values (not really used, just so they can be stored - they aren't very good for code readability)
    bool    vbios_enabled;                  // is the vbios enabled?
} nv3_pci_config_t;

// add enums for eac
// Chip configuration
typedef struct nv3_straps_s
{

} nv3_straps_t;

// Framebuffer
typedef struct nv3_pfb_s
{

} nv3_pfb_t;

// Access the GPU from real-mode
typedef struct nv3_pbus_rma_s
{
    uint32_t addr;              // Address to RMA to
    uint32_t data;              // Data to RMA
    uint8_t mode; 
} nv3_pbus_rma_t;

// Bus Configuration
typedef struct nv3_pbus_s
{
    nv3_pbus_rma_t rma;
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
typedef struct nv3_pfifo_ramfc_s
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
    // these should be uint8_t but C math is a lot better with this
    uint32_t memory_clock_m;     // memory clock M-divider
    uint32_t memory_clock_n;     // memory clock N-divider
    uint32_t memory_clock_p;     // memory clock P-divider
    uint32_t pixel_clock_m;     // pixel clock M-divider
    uint32_t pixel_clock_n;     // pixel clock N-divider
    uint32_t pixel_clock_p;     // pixel clock P-divider
    
} nv3_pramdac_t;

// Graphics Subsystem
typedef struct nv3_pgraph_s
{

} nv3_pgraph_t;

// Graphics Subsystem
typedef struct nv3_pstraps_s
{

} nv3_pstraps_t;

typedef struct nv3_ptimer_s
{

} nv3_ptimer_t;

typedef struct nv3_pramin_s
{

} nv3_pramin_t;

typedef struct nv3_s
{
    nv_base_t nvbase;   // Base Nvidia structure
    
    // Config
    nv3_straps_t straps;
    nv3_pci_config_t pci_config;

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
    nv3_ptimer_t ptimer;         // programmable interval timer
    nv3_pramin_t pramin;         // Ram for INput of DMA objects. Very important!

    //more here

} nv3_t;

// device objects
extern nv3_t* nv3;

// NV3
void*   nv3_init(const device_t *info);

void    nv3_close(void* priv);
void    nv3_speed_changed(void *priv);
void    nv3_force_redraw(void* priv);

uint8_t nv3_mmio_read8(uint32_t addr, void* priv); // Read 8-bit MMIO
uint16_t nv3_mmio_read16(uint32_t addr, void* priv); // Read 16-bit MMIO
uint32_t nv3_mmio_read32(uint32_t addr, void* priv); // Read 32-bit MMIO
void    nv3_mmio_write8(uint32_t addr, uint8_t val, void* priv); // Write 8-bit MMIO
void    nv3_mmio_write16(uint32_t addr, uint16_t val, void* priv); // Write 16-bit MMIO
void    nv3_mmio_write32(uint32_t addr, uint32_t val, void* priv); // Write 32-bit MMIO

// NV3 PBUS RMA - Real Mode Access for VBIOS
uint8_t nv3_pbus_rma_read(uint16_t addr);
void    nv3_pbus_rma_write(uint16_t addr, uint8_t val);

// NV3 PRAMDAC
void    nv3_pramdac_init();
void    nv3_pramdac_set_vram_clock();
void    nv3_pramdac_set_pixel_clock();
#endif