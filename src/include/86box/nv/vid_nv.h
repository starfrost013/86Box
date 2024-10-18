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

// NV Base
typedef struct nv_base_s
{
    mem_mapping_t mmio; // mmio mapping (16mb ) 
    rom_t vbios;        // NVIDIA VBIOS
    svga_t svga;
    
} nv_base_t;

// Master Control
typedef struct nv3_device_pmc_s
{
    int32_t pmc_enable; // Determines which subsystems are enabled.

} nv3_device_pmc_t;

// add enums for eac
// Chip configuration
typedef struct nv3_device_straps_s
{

} nv3_device_straps_t;

// Framebuffer
typedef struct nv3_device_pfb_s
{

} nv3_device_pfb_t;

// Bus Configuration
typedef struct nv3_device_pbus_s
{

} nv3_device_pbus_t;

// Graphics object hashtable
typedef struct nv_device_pfifo_ramht_s
{

} nv3_device_pfifo_ramht_t;

// Anti-fuckup device
typedef struct nv_device_pfifo_ramro_s
{

} nv3_device_pfifo_ramro_s;

// context for unused channels
typedef struct nv_device_pfifo_ramfc_s
{

} nv3_device_pfifo_ramfc_s;


// ????? ram auxillary
typedef struct nv_device_pfifo_ramau_s
{

} nv3_device_pfifo_ramau_s;

// Command submission to PGRAPH
typedef struct nv_device_pfifo_s
{

} nv3_device_pfifo_t;

// RAMDAC
typedef struct nv3_device_pramdac_s
{

} nv3_device_pramdac_t;

// Graphics Subsystem
typedef struct nv3_device_pgraph_s
{

} nv3_device_pgraph_t;

// Graphics Subsystem
typedef struct nv3_device_pstraps_s
{

} nv3_device_pstraps_t;

typedef struct nv3_device_s
{
    nv_base_t nvbase;   // Base Nvidia structure
    
    // Config
    nv3_device_straps_t nvstraps;

    // Subsystems
    nv3_device_pmc_t pmc;               // Master Control
    nv3_device_pfb_t pfb;               // Framebuffer/VRAM
    nv3_device_pbus_t pbus;             // Bus Control
    nv3_device_pfifo_t pfifo;           // FIFO for command submisison
    nv3_device_pfifo_ramht_t ramht;     // hashtable for PGRAPH objects
    nv3_device_pfifo_ramro_t ramro;     // anti-fuckup mechanism for idiots who fucked up the FIFO submission
    nv3_device_pfifo_ramfc_t ramfc;     // context for unused channels
    nv3_device_pfifo_ramau_t ramau;     // auxillary weirdnes
    nv3_device_pramdac_t pramdac;       // RAMDAC (CLUT etc)
    nv3_device_pgraph_t pgraph;         // 2D/3D Graphics
    nv3_device_pstraps_t pstraps;       // Chip configuration

    //more here

} nv3_device_t;

// device objects
extern nv3_device_t* nv3;

// NV3
void*   nv3_init(const device_t *info);

void    nv3_close(void* priv);
void    nv3_speed_changed(void *priv);
void    nv3_force_redraw(void* priv);
#endif