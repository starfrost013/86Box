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

#include <86box/timer.h>
#include <86box/vid_svga.h>
#include <86box/vid_svga_render.h>

void nv_log(const char *fmt, ...);

// NV Base
typedef struct nv_base_s
{
    mem_mapping_t mmio;
    rom_t vbios;        // NVIDIA VBIOS
    svga_t svga;
    
} nv_base_t;

// Master Control
typedef struct nv3_device_pmc_s
{
    int32_t pmc_enable;
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

// Command submission to PGRAPH
typedef struct nv_device_pfifo_s
{

} nv3_device_pfifo_t;

// RAMDAC
typedef struct nv3_device_pramdac_s
{

} nv3_device_pramdac_t;

// Graphics Subsysttem
typedef struct nv3_device_pgraph_s
{

} nv3_device_pgraph_t;

typedef struct nv3_device_s
{
    nv_base_t nvbase;   // Base Nvidia structure
    
    // Config
    nv3_device_straps_t nvstraps;

    // Subsystems
    nv3_device_pfb_t pfb;
    nv3_device_pfifo_t pfifo;
    nv3_device_pramdac_t pramdac;
    nv3_device_pgraph_t pgraph;
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