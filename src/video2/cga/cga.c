/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Emulation of the old and new IBM CGA graphics cards.
 *
 * Authors: Sarah Walker, <https://pcem-emulator.co.uk/>
 *          Miran Grca, <mgrca8@gmail.com>
 *          W. M. Martinez, <anikom15@outlook.com>
 *
 *          Copyright 2008-2019 Sarah Walker.
 *          Copyright 2016-2019 Miran Grca.
 *          Copyright 2023      W. M. Martinez
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <math.h>
#include <86box/86box.h>
#include "cpu.h"
#include <86box/io.h>
#include <86box/timer.h>
#include <86box/pit.h>
#include <86box/mem.h>
#include <86box/rom.h>
#include <86box/device.h>
#include <86box/video2/video.h>
#include <86box/video2/cga/cga.h>
//#include <86box/vid_cga_comp.h> TEMP
#include <86box/plat_unused.h>

void* cga_init(const device_t* dev)
{

}

void cga_close(void* priv)
{

}

void cga_speed_changed(void* priv)
{

}

device_t cga_device = {
    .name          = "New CGA - Testbed",
    .internal_name = "cga",
    .flags         = DEVICE_ISA,
    .local         = 0,
    .init          = cga_init,
    .close         = cga_close,
    .reset         = NULL,
    .available     = NULL,
    .speed_changed = cga_speed_changed,
    .force_redraw  = NULL,
    //.config        = cga_config
};
