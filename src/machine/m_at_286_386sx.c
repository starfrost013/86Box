/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Implementation of 286 and 386SX machines.
 *
 *
 *
 * Authors: Miran Grca, <mgrca8@gmail.com>
 *          EngiNerd <webmaster.crrc@yahoo.it>
 *
 *          Copyright 2016-2019 Miran Grca.
 *          Copyright 2020 EngiNerd.
 */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#define HAVE_STDARG_H
#include <86box/86box.h>
#include "cpu.h"
#include <86box/timer.h>
#include <86box/io.h>
#include <86box/device.h>
#include <86box/chipset.h>
#include <86box/keyboard.h>
#include <86box/mem.h>
#include <86box/rom.h>
#include <86box/fdd.h>
#include <86box/fdc.h>
#include <86box/fdc_ext.h>
#include <86box/hdc.h>
#include <86box/nvr.h>
#include <86box/port_6x.h>
#include <86box/sio.h>
#include <86box/serial.h>
#include <86box/video.h>
#include <86box/vid_cga.h>
#include <86box/flash.h>
#include <86box/machine.h>

static void
machine_at_headland_common_init(int type)
{
    device_add(&keyboard_at_ami_device);

    if ((type != 2) && (fdc_current[0] == FDC_INTERNAL))
        device_add(&fdc_at_device);

    if (type == 2)
        device_add(&headland_ht18b_device);
    else if (type == 1)
        device_add(&headland_gc113_device);
    else
        device_add(&headland_gc10x_device);
}

int
machine_at_quadt386sx_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/quadt386sx/QTC-SXM-EVEN-U3-05-07.BIN",
                                "roms/machines/quadt386sx/QTC-SXM-ODD-U3-05-07.BIN",
                                0x000f0000, 65536, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init(model);
    device_add(&keyboard_at_device);

    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_at_device);

    device_add(&headland_gc10x_device);

    return ret;
}

static void
machine_at_scat_init(const machine_t *model, int is_v4, int is_ami)
{
    machine_at_common_init(model);

    if (machines[machine].bus_flags & MACHINE_BUS_PS2) {
        if (is_ami)
            device_add(&keyboard_ps2_ami_device);
        else
            device_add(&keyboard_ps2_device);
    } else {
        if (is_ami)
            device_add(&keyboard_at_ami_device);
        else
            device_add(&keyboard_at_device);
    }

    if (is_v4)
        device_add(&scat_4_device);
    else
        device_add(&scat_device);
}

static void
machine_at_scatsx_init(const machine_t *model)
{
    machine_at_common_init(model);

    device_add(&keyboard_at_ami_device);

    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_at_device);

    device_add(&scat_sx_device);
}

int
machine_at_wd76c10_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/megapc/41651-bios lo.u18",
                                "roms/machines/megapc/211253-bios hi.u19",
                                0x000f0000, 65536, 0x08000);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init_ex(model, 2);

    if (gfxcard[0] == VID_INTERNAL)
        device_add(&paradise_wd90c11_megapc_device);

    device_add(&keyboard_ps2_quadtel_device);

    device_add(&wd76c10_device);

    return ret;
}

static void
machine_at_scamp_common_init(const machine_t *model, int is_ps2)
{
    machine_at_common_ide_init(model);

    if (is_ps2)
        device_add(&keyboard_ps2_ami_device);
    else
        device_add(&keyboard_at_ami_device);

    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_at_device);

    device_add(&vlsi_scamp_device);
}
