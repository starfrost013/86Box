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

// NV Base
typedef struct nv_base_s
{
    rom_t vbios;        // NVIDIA VBIOS
} nv_base_t;

typedef struct nv_device_nv3_s
{
    nv_base_t nvbase;   // Base Nvidia structure
} nv_device_nv3_t;

// device objects
extern nv_device_nv3_t* nv3;

// NV3
void*   nv3_init(const device_t *info);

void    nv3_close(void* priv);
void    nv3_speed_changed(void *priv);
void    nv3_force_redraw(void* priv);
#endif