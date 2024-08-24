/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Implementation of 386DX and 486 machines.
 *
 *
 *
 * Authors: Miran Grca, <mgrca8@gmail.com>
 *
 *          Copyright 2016-2020 Miran Grca.
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
#include <86box/nvr.h>
#include <86box/pci.h>
#include <86box/dma.h>
#include <86box/fdd.h>
#include <86box/fdc.h>
#include <86box/fdc_ext.h>
#include <86box/gameport.h>
#include <86box/pic.h>
#include <86box/pit.h>
#include <86box/rom.h>
#include <86box/sio.h>
#include <86box/hdc.h>
#include <86box/port_6x.h>
#include <86box/port_92.h>
#include <86box/video.h>
#include <86box/flash.h>
#include <86box/scsi_ncr53c8xx.h>
#include <86box/hwm.h>
#include <86box/machine.h>
#include <86box/plat_unused.h>

int
machine_at_acc386_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/acc386/acc386.BIN",
                           0x000f0000, 65536, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init(model);
    device_add(&acc2168_device);
    device_add(&keyboard_at_ami_device);

    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_at_device);

    return ret;
}

int
machine_at_valuepoint433_init(const machine_t *model) // hangs without the PS/2 mouse
{
    int ret;

    ret = bios_load_linear("roms/machines/valuepoint433/$IMAGEP.FLH",
                           0x000e0000, 131072, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_ide_init(model);
    device_add(&sis_85c461_device);
    if (gfxcard[0] == VID_INTERNAL)
        device_add(&et4000w32_onboard_device);

    device_add(&keyboard_ps2_device);

    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_at_device);

    return ret;
}


static void
machine_at_sis401_common_init(const machine_t *model)
{
    machine_at_common_init(model);
    device_add(&sis_85c401_device);
    device_add(&keyboard_at_ami_device);

    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_at_device);
}

int
machine_at_sis401_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/sis401/SIS401-2.AMI",
                           0x000f0000, 65536, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_sis401_common_init(model);

    return ret;
}

int
machine_at_isa486_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/isa486/ISA-486.BIN",
                           0x000f0000, 65536, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_sis401_common_init(model);

    return ret;
}

int
machine_at_vect486vl_init(const machine_t *model) // has HDC problems
{
    int ret;

    ret = bios_load_linear("roms/machines/vect486vl/aa0500.ami",
                           0x000e0000, 131072, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_ide_init(model);

    device_add(&vl82c480_device);

    if (gfxcard[0] == VID_INTERNAL)
        device_add(&gd5428_onboard_device);

    device_add(&vl82c113_device);
    device_add(&fdc37c651_ide_device);

    return ret;
}

static void
machine_at_ali1429_common_init(const machine_t *model, int is_green)
{
    machine_at_common_init(model);

    if (is_green)
        device_add(&ali1429g_device);
    else
        device_add(&ali1429_device);

    device_add(&keyboard_at_ami_device);

    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_at_device);
}

int
machine_at_ali1429_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/ali1429/ami486.BIN",
                           0x000f0000, 65536, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_ali1429_common_init(model, 0);

    return ret;
}

int
machine_at_exp4349_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/exp4349/biosdump.bin",
                           0x000f0000, 65536, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init(model);

    device_add(&ali1429g_device);
    device_add(&keyboard_at_ami_device);

    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_at_device);
    return ret;
}

static void
machine_at_pc330_6573_common_init(const machine_t *model)
{
    machine_at_common_init_ex(model, 2);
    device_add(&ide_vlb_2ch_device);

    pci_init(PCI_CONFIG_TYPE_1);
    pci_register_slot(0x10, PCI_CARD_NORTHBRIDGE,  0,  0,  0,  0);
    pci_register_slot(0x0B, PCI_CARD_NORMAL,       1,  2,  3,  4);
    pci_register_slot(0x0C, PCI_CARD_NORMAL,       5,  6,  7,  8);
    pci_register_slot(0x0D, PCI_CARD_NORMAL,       9, 10, 11, 12);
    /* This is a guess because the BIOS always gives it a video BIOS
       and never gives it an IRQ, so it is impossible to known for
       certain until we obtain PCI readouts from the real machine. */
    pci_register_slot(0x0E, PCI_CARD_VIDEO,       13, 14, 15, 16);

    if (gfxcard[0] == VID_INTERNAL)
        device_add(machine_get_vid_device(machine));

    device_add(&opti602_device);
    device_add(&opti802g_device);
    device_add(&opti822_device);
    device_add(&keyboard_ps2_ami_device);
    device_add(&fdc37c665_ide_device);
    device_add(&ide_opti611_vlb_device);
    device_add(&intel_flash_bxt_device);
}

int
machine_at_pc330_6573_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/pc330_6573/$IMAGES.USF",
                           0x000e0000, 131072, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_pc330_6573_common_init(model);

    return ret;
}

static void
machine_at_sis_85c471_common_init(const machine_t *model)
{
    machine_at_common_init(model);

    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_at_device);

    device_add(&sis_85c471_device);
}

int
machine_at_greenb_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/greenb/4gpv31-ami-1993-8273517.bin",
                           0x000f0000, 65536, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init(model);

    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_at_device);

    device_add(&contaq_82c597_device);

    device_add(&keyboard_at_ami_device);

    return ret;
}


static void
machine_at_sis_85c496_common_init(UNUSED(const machine_t *model))
{
    device_add(&ide_pci_2ch_device);

    pci_init(PCI_CONFIG_TYPE_1 | FLAG_TRC_CONTROLS_CPURST);
    pci_register_slot(0x05, PCI_CARD_NORTHBRIDGE, 0, 0, 0, 0);

    pci_set_irq_routing(PCI_INTA, PCI_IRQ_DISABLED);
    pci_set_irq_routing(PCI_INTB, PCI_IRQ_DISABLED);
    pci_set_irq_routing(PCI_INTC, PCI_IRQ_DISABLED);
    pci_set_irq_routing(PCI_INTD, PCI_IRQ_DISABLED);
}

int
machine_at_alfredo_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear_combined("roms/machines/alfredo/1010AQ0_.BIO",
                                    "roms/machines/alfredo/1010AQ0_.BI1", 0x1c000, 128);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init(model);
    device_add(&ide_pci_device);

    pci_init(PCI_CONFIG_TYPE_2 | PCI_NO_IRQ_STEERING);
    pci_register_slot(0x00, PCI_CARD_NORTHBRIDGE, 0, 0, 0, 0);
    pci_register_slot(0x01, PCI_CARD_IDE,         0, 0, 0, 0);
    pci_register_slot(0x06, PCI_CARD_NORMAL,      3, 2, 1, 4);
    pci_register_slot(0x0E, PCI_CARD_NORMAL,      2, 1, 3, 4);
    pci_register_slot(0x0C, PCI_CARD_NORMAL,      1, 3, 2, 4);
    pci_register_slot(0x02, PCI_CARD_SOUTHBRIDGE, 0, 0, 0, 0);
    device_add(&keyboard_ps2_pci_device);
    device_add(&sio_device);
    device_add(&fdc37c663_device);
    device_add(&intel_flash_bxt_ami_device);

    device_add(&i420tx_device);

    return ret;
}

int
machine_at_486sp3g_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/486sp3g/PCI-I-486SP3G_0306.001 (Beta).bin",
                           0x000e0000, 131072, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init(model);
    device_add(&ide_isa_device);

    pci_init(PCI_CONFIG_TYPE_2);
    pci_register_slot(0x00, PCI_CARD_NORTHBRIDGE, 0, 0, 0, 0);
    pci_register_slot(0x01, PCI_CARD_SCSI,        1, 2, 3, 4); /* 01 = SCSI */
    pci_register_slot(0x06, PCI_CARD_NORMAL,      1, 2, 3, 4); /* 06 = Slot 1 */
    pci_register_slot(0x05, PCI_CARD_NORMAL,      2, 3, 4, 1); /* 05 = Slot 2 */
    pci_register_slot(0x04, PCI_CARD_NORMAL,      3, 4, 1, 2); /* 04 = Slot 3 */
    pci_register_slot(0x02, PCI_CARD_SOUTHBRIDGE, 0, 0, 0, 0);
    device_add(&keyboard_ps2_ami_pci_device);                  /* Uses the AMIKEY KBC */
    device_add(&sio_zb_device);
    device_add(&pc87332_398_ide_device);
    device_add(&sst_flash_29ee010_device);

    device_add(&i420zx_device);
    device_add(&ncr53c810_onboard_pci_device);

    return ret;
}

int
machine_at_win486pci_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/win486pci/v1hj3.BIN",
                           0x000e0000, 131072, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init(model);

    pci_init(PCI_CONFIG_TYPE_1);
    pci_register_slot(0x00, PCI_CARD_NORTHBRIDGE, 0, 0, 0, 0);
    pci_register_slot(0x03, PCI_CARD_NORMAL,      1, 2, 3, 4);
    pci_register_slot(0x04, PCI_CARD_NORMAL,      2, 3, 4, 1);
    pci_register_slot(0x05, PCI_CARD_NORMAL,      3, 4, 1, 2);

    device_add(&ali1489_device);
    device_add(&prime3b_device);
    device_add(&keyboard_at_ami_device);

    return ret;
}

int
machine_at_ga486l_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/ga486l/ga-486l_bios.bin",
                           0x000f0000, 65536, 0);

    if (bios_only || !ret)
        return ret;

    machine_at_common_init(model);
    device_add(&opti381_device);
    device_add(&keyboard_at_ami_device);

    if (fdc_current[0] == FDC_INTERNAL)
        device_add(&fdc_at_device);

    return ret;
}