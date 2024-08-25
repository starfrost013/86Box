/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Implementation of Socket 7 (Dual Voltage) machines.
 *
 *
 *
 * Authors: Miran Grca, <mgrca8@gmail.com>
 *
 *          Copyright 2016-2020 Miran Grca.
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <86box/86box.h>
#include <86box/mem.h>
#include <86box/io.h>
#include <86box/rom.h>
#include <86box/pci.h>
#include <86box/device.h>
#include <86box/chipset.h>
#include <86box/hdc.h>
#include <86box/hdc_ide.h>
#include <86box/keyboard.h>
#include <86box/flash.h>
#include <86box/sio.h>
#include <86box/sound.h>
#include <86box/hwm.h>
#include <86box/video.h>
#include <86box/spd.h>
#include "cpu.h"
#include <86box/machine.h>
#include <86box/timer.h>
#include <86box/fdd.h>
#include <86box/fdc.h>
#include <86box/nvr.h>
#include <86box/scsi_ncr53c8xx.h>
#include <86box/thread.h>
#include <86box/network.h>

/* The Sony VAIO is an AG430HX, I'm assuming it has the same configuration bits
   as the TC430HX, hence the #define. */
#define machine_at_ag430hx_gpio_init machine_at_tc430hx_gpio_init


static void
machine_at_tc430hx_gpio_init(void)
{
    uint32_t gpio = 0xffffe1ff;

    /* Register 0x0079: */
    /* Bit 7: 0 = Clear password, 1 = Keep password. */
    /* Bit 6: 0 = NVRAM cleared by jumper, 1 = NVRAM normal. */
    /* Bit 5: 0 = CMOS Setup disabled, 1 = CMOS Setup enabled. */
    /* Bit 4: External CPU clock (Switch 8). */
    /* Bit 3: External CPU clock (Switch 7). */
    /*        50 MHz: Switch 7 = Off, Switch 8 = Off. */
    /*        60 MHz: Switch 7 = On, Switch 8 = Off. */
    /*        66 MHz: Switch 7 = Off, Switch 8 = On. */
    /* Bit 2: 0 = On-board audio absent, 1 = On-board audio present. */
    /* Bit 1: 0 = Soft-off capable power supply present, 1 = Soft-off capable power supply absent. */
    /* Bit 0: 0 = Reserved. */
    /* NOTE: A bit is read as 1 if switch is off, and as 0 if switch is on. */
    if (cpu_busspeed <= 50000000)
        gpio |= 0xffff10ff;
    else if ((cpu_busspeed > 50000000) && (cpu_busspeed <= 60000000))
        gpio |= 0xffff18ff;
    else if (cpu_busspeed > 60000000)
        gpio |= 0xffff00ff;

    machine_set_gpio_default(gpio);
}


int
machine_at_infinia7200_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear_combined2("roms/machines/infinia7200/1008DH08.BIO",
                                     "roms/machines/infinia7200/1008DH08.BI1",
                                     "roms/machines/infinia7200/1008DH08.BI2",
                                     "roms/machines/infinia7200/1008DH08.BI3",
                                     "roms/machines/infinia7200/1008DH08.RCV",
                                     0x3a000, 128);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init_ex(model, 2);
    machine_at_tc430hx_gpio_init();

    pci_init(PCI_CONFIG_TYPE_1);
    pci_register_slot(0x00, PCI_CARD_NORTHBRIDGE, 0, 0, 0, 0);
    pci_register_slot(0x08, PCI_CARD_VIDEO, 4, 0, 0, 0);
    pci_register_slot(0x0D, PCI_CARD_NORMAL, 1, 2, 3, 4);
    pci_register_slot(0x0E, PCI_CARD_NORMAL, 2, 3, 4, 1);
    pci_register_slot(0x0F, PCI_CARD_NORMAL, 3, 4, 1, 2);
    pci_register_slot(0x10, PCI_CARD_NORMAL, 4, 1, 2, 3);
    pci_register_slot(0x07, PCI_CARD_SOUTHBRIDGE, 0, 0, 0, 0);

    if (gfxcard[0] == VID_INTERNAL)
        device_add(machine_get_vid_device(machine));

    device_add(&i430hx_device);
    device_add(&piix3_device);
    device_add(&keyboard_ps2_ami_pci_device);
    device_add(&pc87306_device);
    device_add(&intel_flash_bxt_ami_device);

    return ret;
}

int
machine_at_i430vx_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/430vx/55XWUQ0E.BIN",
                           0x000e0000, 131072, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init(model);

    pci_init(PCI_CONFIG_TYPE_1);
    pci_register_slot(0x00, PCI_CARD_NORTHBRIDGE, 0, 0, 0, 0);
    pci_register_slot(0x11, PCI_CARD_NORMAL,      2, 3, 4, 1);
    pci_register_slot(0x12, PCI_CARD_NORMAL,      1, 2, 3, 4);
    pci_register_slot(0x14, PCI_CARD_NORMAL,      3, 4, 1, 2);
    pci_register_slot(0x13, PCI_CARD_NORMAL,      4, 1, 2, 3);
    pci_register_slot(0x07, PCI_CARD_SOUTHBRIDGE, 0, 0, 0, 0);
    device_add(&i430vx_device);
    device_add(&piix3_device);
    device_add(&keyboard_ps2_pci_device);
    device_add(&um8669f_device);
    device_add(&intel_flash_bxt_device);

    return ret;
}

int
machine_at_tx97_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/tx97/0112.001",
                           0x000e0000, 131072, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init_ex(model, 2);

    pci_init(PCI_CONFIG_TYPE_1);
    pci_register_slot(0x00, PCI_CARD_NORTHBRIDGE, 0, 0, 0, 0);
    pci_register_slot(0x0C, PCI_CARD_NORMAL,      1, 2, 3, 4);
    pci_register_slot(0x0B, PCI_CARD_NORMAL,      2, 3, 4, 1);
    pci_register_slot(0x0A, PCI_CARD_NORMAL,      3, 4, 1, 2);
    pci_register_slot(0x09, PCI_CARD_NORMAL,      4, 1, 2, 3);
    pci_register_slot(0x01, PCI_CARD_SOUTHBRIDGE, 1, 2, 3, 4); /* PIIX4 */
    pci_register_slot(0x0D, PCI_CARD_NORMAL,      1, 2, 3, 4);
    pci_register_slot(0x08, PCI_CARD_NORMAL,      1, 2, 3, 4);
    device_add(&i430tx_device);
    device_add(&piix4_device);
    device_add(&keyboard_ps2_ami_pci_device);
    device_add(&w83877tf_acorp_device);
    device_add(&intel_flash_bxt_device);
    spd_register(SPD_TYPE_SDRAM, 0x3, 128);
    device_add(&w83781d_device);    /* fans: Chassis, CPU, Power; temperatures: MB, unused, CPU */
    hwm_values.temperatures[1] = 0; /* unused */
    /* CPU offset */
    if (hwm_values.temperatures[2] < 32) /* prevent underflow */
        hwm_values.temperatures[2] = 0;
    else
        hwm_values.temperatures[2] -= 32;

    return ret;
}


int
machine_at_r534f_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/r534f/r534f008.bin",
                           0x000e0000, 131072, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init_ex(model, 2);

    pci_init(PCI_CONFIG_TYPE_1 | FLAG_TRC_CONTROLS_CPURST);
    pci_register_slot(0x00, PCI_CARD_NORTHBRIDGE, 0, 0, 0, 0);
    pci_register_slot(0x01, PCI_CARD_SOUTHBRIDGE, 1, 2, 3, 4);
    pci_register_slot(0x0B, PCI_CARD_NORMAL,      1, 2, 3, 4);
    pci_register_slot(0x0D, PCI_CARD_NORMAL,      2, 3, 4, 1);
    pci_register_slot(0x0F, PCI_CARD_NORMAL,      3, 4, 1, 2);
    pci_register_slot(0x11, PCI_CARD_NORMAL,      4, 1, 2, 3);

    device_add(&sis_5571_device);
    device_add(&keyboard_ps2_ami_pci_device);
    device_add(&w83877f_device);
    device_add(&sst_flash_29ee010_device);

    return ret;
}