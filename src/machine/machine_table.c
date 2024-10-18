/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          Handling of the emulated machines.
 *
 * NOTES:   OpenAT wip for 286-class machine with open BIOS.
 *          PS2_M80-486 wip, pending receipt of TRM's for machine.
 *
 *
 *
 * Authors: Miran Grca, <mgrca8@gmail.com>
 *          Fred N. van Kempen, <decwiz@yahoo.com>
 *
 *          Copyright 2016-2020 Miran Grca.
 *          Copyright 2017-2020 Fred N. van Kempen.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <86box/86box.h>
#include "cpu.h"
#include <86box/mem.h>
#include <86box/rom.h>
#include <86box/device.h>
#include <86box/machine.h>
#include <86box/keyboard.h>
#include <86box/sound.h>
#include <86box/video.h>
#include <86box/plat_unused.h>
#include <86box/thread.h>
#include <86box/timer.h>
#include <86box/network.h>

// Temporarily here till we move everything out into the right files
extern const device_t pcjr_device;
extern const device_t m19_vid_device;
extern const device_t vid_device;
extern const device_t vid_device_hx;
extern const device_t t1000_video_device;
extern const device_t cga_device;
extern const device_t vid_1512_device;
extern const device_t vid_1640_device;
extern const device_t vid_pc2086_device;
extern const device_t vid_pc3086_device;
extern const device_t vid_200_device;
extern const device_t vid_ppc512_device;
extern const device_t vid_device_sl;
extern const device_t t1200_video_device;
extern const device_t compaq_plasma_device;
extern const device_t ps1_2011_device;

const machine_filter_t machine_types[] = {
    { "None",                             MACHINE_TYPE_NONE       },
    { "[1979] 8088",                      MACHINE_TYPE_8088       },
    { "[1978] 8086",                      MACHINE_TYPE_8086       },
    { "[1982] 80286",                     MACHINE_TYPE_286        },
    { "[1988] i386SX",                    MACHINE_TYPE_386SX      },
    { "[1992] 486SLC",                    MACHINE_TYPE_486SLC     },
    { "[1985] i386DX",                    MACHINE_TYPE_386DX      },
    { "[1989] i386DX/i486",               MACHINE_TYPE_386DX_486  },
    { "[1992] i486 (Socket 168 and 1)",   MACHINE_TYPE_486        },
    { "[1992] i486 (Socket 2)",           MACHINE_TYPE_486_S2     },
    { "[1994] i486 (Socket 3)",           MACHINE_TYPE_486_S3     },
    { "[1992] i486 (Miscellaneous)",      MACHINE_TYPE_486_MISC   },
    { "[1993] Socket 4",                  MACHINE_TYPE_SOCKET4    },
    { "[1994] Socket 5",                  MACHINE_TYPE_SOCKET5    },
    { "[1995] Socket 7 (Single Voltage)", MACHINE_TYPE_SOCKET7_3V },
    { "[1996] Socket 7 (Dual Voltage)",   MACHINE_TYPE_SOCKET7    },
    { "[1998] Super Socket 7",            MACHINE_TYPE_SOCKETS7   },
    { "[1995] Socket 8",                  MACHINE_TYPE_SOCKET8    },
    { "[1997] Slot 1",                    MACHINE_TYPE_SLOT1      },
    { "[1998] Slot 1/2",                  MACHINE_TYPE_SLOT1_2    },
    { "[1998] Slot 1/Socket 370",         MACHINE_TYPE_SLOT1_370  },
    { "[1998] Slot 2",                    MACHINE_TYPE_SLOT2      },
    { "[1998] Socket 370",                MACHINE_TYPE_SOCKET370  },
    { "Miscellaneous",                    MACHINE_TYPE_MISC       }
};

const machine_filter_t machine_chipsets[] = {
    { "None",                       MACHINE_CHIPSET_NONE                },
    { "Discrete",                   MACHINE_CHIPSET_DISCRETE            },
    { "Proprietary",                MACHINE_CHIPSET_PROPRIETARY         },
    { "Headland GC103",             MACHINE_CHIPSET_GC103               },
    { "Headland HT18",              MACHINE_CHIPSET_HT18                },
    { "ACC 2168",                   MACHINE_CHIPSET_ACC_2168            },
    { "ALi M6117",                  MACHINE_CHIPSET_ALI_M6117           },
    { "ALi M1429",                  MACHINE_CHIPSET_ALI_M1429           },
    { "ALi M1429G",                 MACHINE_CHIPSET_ALI_M1429G          },
    { "ALi M1489",                  MACHINE_CHIPSET_ALI_M1489           },
    { "ALi ALADDiN IV+",            MACHINE_CHIPSET_ALI_ALADDIN_IV_PLUS },
    { "ALi ALADDiN V",              MACHINE_CHIPSET_ALI_ALADDIN_V       },
    { "ALi ALADDiN-PRO II",         MACHINE_CHIPSET_ALI_ALADDIN_PRO_II  },
    { "C&T 386",                    MACHINE_CHIPSET_CT_386              },
    { "C&T CS4031",                 MACHINE_CHIPSET_CT_CS4031           },
    { "Contaq 82C596",              MACHINE_CHIPSET_CONTAQ_82C596       },
    { "Contaq 82C597",              MACHINE_CHIPSET_CONTAQ_82C597       },
    { "IMS 8848",                   MACHINE_CHIPSET_IMS_8848            },
    { "Intel 420TX",                MACHINE_CHIPSET_INTEL_420TX         },
    { "Intel 420ZX",                MACHINE_CHIPSET_INTEL_420ZX         },
    { "Intel 420EX",                MACHINE_CHIPSET_INTEL_420EX         },
    { "Intel 430LX",                MACHINE_CHIPSET_INTEL_430LX         },
    { "Intel 430NX",                MACHINE_CHIPSET_INTEL_430NX         },
    { "Intel 430FX",                MACHINE_CHIPSET_INTEL_430FX         },
    { "Intel 430HX",                MACHINE_CHIPSET_INTEL_430HX         },
    { "Intel 430VX",                MACHINE_CHIPSET_INTEL_430VX         },
    { "Intel 430TX",                MACHINE_CHIPSET_INTEL_430TX         },
    { "Intel 450KX",                MACHINE_CHIPSET_INTEL_450KX         },
    { "Intel 440FX",                MACHINE_CHIPSET_INTEL_440FX         },
    { "Intel 440LX",                MACHINE_CHIPSET_INTEL_440LX         },
    { "Intel 440EX",                MACHINE_CHIPSET_INTEL_440EX         },
    { "Intel 440BX",                MACHINE_CHIPSET_INTEL_440BX         },
    { "Intel 440ZX",                MACHINE_CHIPSET_INTEL_440ZX         },
    { "Intel 440GX",                MACHINE_CHIPSET_INTEL_440GX         },
    { "OPTi 283",                   MACHINE_CHIPSET_OPTI_283            },
    { "OPTi 291",                   MACHINE_CHIPSET_OPTI_291            },
    { "OPTi 381",                   MACHINE_CHIPSET_OPTI_381            },
    { "OPTi 391",                   MACHINE_CHIPSET_OPTI_391            },
    { "OPTi 481",                   MACHINE_CHIPSET_OPTI_481            },
    { "OPTi 493",                   MACHINE_CHIPSET_OPTI_493            },
    { "OPTi 495",                   MACHINE_CHIPSET_OPTI_495            },
    { "OPTi 499",                   MACHINE_CHIPSET_OPTI_499            },
    { "OPTi 895/802G",              MACHINE_CHIPSET_OPTI_895_802G       },
    { "OPTi 547/597",               MACHINE_CHIPSET_OPTI_547_597        },
    { "SARC RC2016A",               MACHINE_CHIPSET_SARC_RC2016A        },
    { "SiS 310",                    MACHINE_CHIPSET_SIS_310             },
    { "SiS 401",                    MACHINE_CHIPSET_SIS_401             },
    { "SiS 460",                    MACHINE_CHIPSET_SIS_460             },
    { "SiS 461",                    MACHINE_CHIPSET_SIS_461             },
    { "SiS 471",                    MACHINE_CHIPSET_SIS_471             },
    { "SiS 496",                    MACHINE_CHIPSET_SIS_496             },
    { "SiS 501",                    MACHINE_CHIPSET_SIS_501             },
    { "SiS 5501",                   MACHINE_CHIPSET_SIS_5501            },
    { "SiS 5571",                   MACHINE_CHIPSET_SIS_5571            },
    { "SMSC VictoryBX-66",          MACHINE_CHIPSET_SMSC_VICTORYBX_66   },
    { "STPC Client",                MACHINE_CHIPSET_STPC_CLIENT         },
    { "STPC Consumer-II",           MACHINE_CHIPSET_STPC_CONSUMER_II    },
    { "STPC Elite",                 MACHINE_CHIPSET_STPC_ELITE          },
    { "STPC Atlas",                 MACHINE_CHIPSET_STPC_ATLAS          },
    { "Symphony SL82C460 Haydn II", MACHINE_CHIPSET_SYMPHONY_SL82C460   },
    { "UMC UM82C480",               MACHINE_CHIPSET_UMC_UM82C480        },
    { "UMC UM82C491",               MACHINE_CHIPSET_UMC_UM82C491        },
    { "UMC UM8881",                 MACHINE_CHIPSET_UMC_UM8881          },
    { "UMC UM8890BF",               MACHINE_CHIPSET_UMC_UM8890BF        },
    { "VIA VT82C495",               MACHINE_CHIPSET_VIA_VT82C495        },
    { "VIA VT82C496G",              MACHINE_CHIPSET_VIA_VT82C496G       },
    { "VIA Apollo VPX",             MACHINE_CHIPSET_VIA_APOLLO_VPX      },
    { "VIA Apollo VP3",             MACHINE_CHIPSET_VIA_APOLLO_VP3      },
    { "VIA Apollo MVP3",            MACHINE_CHIPSET_VIA_APOLLO_MVP3     },
    { "VIA Apollo Pro",             MACHINE_CHIPSET_VIA_APOLLO_PRO      },
    { "VIA Apollo Pro 133",         MACHINE_CHIPSET_VIA_APOLLO_PRO_133  },
    { "VIA Apollo Pro 133A",        MACHINE_CHIPSET_VIA_APOLLO_PRO_133A },
    { "VLSI SCAMP",                 MACHINE_CHIPSET_VLSI_SCAMP          },
    { "VLSI VL82C480",              MACHINE_CHIPSET_VLSI_VL82C480       },
    { "VLSI VL82C481",              MACHINE_CHIPSET_VLSI_VL82C481       },
    { "VLSI VL82C486",              MACHINE_CHIPSET_VLSI_VL82C486       },
    { "WD76C10",                    MACHINE_CHIPSET_WD76C10             }
};

/* Machines to add before machine freeze:
   - TMC Mycomp PCI54ST;
   - Zeos Quadtel 486.

   NOTE: The AMI MegaKey tests were done on a real Intel Advanced/ATX
     (thanks, MrKsoft for running my AMIKEY.COM on it), but the
     technical specifications of the other Intel machines confirm
     that the other boards also have the MegaKey.

   NOTE: The later (ie. not AMI Color) Intel AMI BIOS'es execute a
     sequence of commands (B8, BA, BB) during one of the very first
     phases of POST, in a way that is only valid on the AMIKey-3
     KBC firmware, that includes the Classic PCI/ED (Ninja) BIOS
     which otherwise does not execute any AMI KBC commands, which
     indicates that the sequence is a leftover of whatever AMI
     BIOS (likely a laptop one since the AMIKey-3 is a laptop KBC
     firmware!) Intel forked.

   NOTE: The VIA VT82C42N returns 0x46 ('F') in command 0xA1 (so it
     emulates the AMI KF/AMIKey KBC firmware), and 0x42 ('B') in
     command 0xAF.
     The version on the VIA VT82C686B southbridge also returns
     'F' in command 0xA1, but 0x45 ('E') in command 0xAF.
     The version on the VIA VT82C586B southbridge also returns
     'F' in command 0xA1, but 0x44 ('D') in command 0xAF.
     The version on the VIA VT82C586A southbridge also returns
     'F' in command 0xA1, but 0x43 ('C') in command 0xAF.

   NOTE: The AMI MegaKey commands blanked in the technical reference
     are CC and and C4, which are Set P14 High and Set P14 Low,
     respectively. Also, AMI KBC command C1, mysteriously missing
     from the technical references of AMI MegaKey and earlier, is
     Write Input Port, same as on AMIKey-3.
*/

const machine_t machines[] = {
  // clang-format off
    /* 8088 Machines */
    {
        .name = "[8088] IBM PC (1981)",
        .internal_name = "ibmpc",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_pc_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC5150,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 16,
            .max = 64,
            .step = 16
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_pc_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8088] IBM PC (1982)",
        .internal_name = "ibmpc82",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_pc82_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC5150,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 64,
            .max = 256,
            .step = 64
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_pc82_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8088] IBM PCjr",
        .internal_name = "ibmpcjr",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_pcjr_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 4772728,
            .max_bus = 4772728,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PCJR,
        .flags = MACHINE_VIDEO_FIXED,
        .ram = {
            .min = 64,
            .max = 640,
            .step = 64
        },
        .nvrmask = 0,
        .kbc_device = NULL, /* TODO: No specific kbd_device yet */
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = &pcjr_device,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8088] IBM XT (1982)",
        .internal_name = "ibmxt",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_xt_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 64,
            .max = 256,
            .step = 64
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_xt_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8088] IBM XT (1986)",
        .internal_name = "ibmxt86",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_xt86_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 256,
            .max = 640,
            .step = 64
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_xt86_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8088] Compaq Portable",
        .internal_name = "portable",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_xt_compaq_portable_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 128,
            .max = 640,
            .step = 128
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_xt_compaq_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8088] Generic XT clone",
        .internal_name = "genxt",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_genxt_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 64,
            .max = 640,
            .step = 64
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_xt_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8088] OpenXT",
        .internal_name = "openxt",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_xt_openxt_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 64,
            .max = 640,
            .step = 64
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_xtclone_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8088] Phoenix XT clone",
        .internal_name = "pxxt",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_xt_pxxt_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 64,
            .max = 640,
            .step = 64
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_xtclone_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8088] Pravetz 16 / IMKO-4",
        .internal_name = "pravetz16",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_xt_pravetz16_imko4_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 64,
            .max = 640,
            .step = 64
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_pravetz_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8088] Pravetz 16S / CPU12 Plus",
        .internal_name = "pravetz16s",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_xt_pravetz16s_cpu12p_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 4772728,
            .max_bus = 12000000,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 512,
            .max = 1024,
            .step = 128
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_xt_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Tandy stuff */
    {
        .name = "[8088] Tandy 1000 HX",
        .internal_name = "tandy1000hx",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_PROPRIETARY,
        .init = machine_tandy1000hx_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_VIDEO_FIXED,
        .ram = {
            .min = 256,
            .max = 640,
            .step = 128
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_tandy_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = &vid_device_hx,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8088] Toshiba T1000",
        .internal_name = "t1000",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_PROPRIETARY,
        .init = machine_xt_t1000_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_VIDEO,
        .ram = {
            .min = 512,
            .max = 1280,
            .step = 768
        },
        .nvrmask = 63,
        .kbc_device = &keyboard_xt_t1x00_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = &t1000_video_device,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[V20] PC-XT",
        .internal_name = "v20xt",
        .type = MACHINE_TYPE_8088,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_xt_v20xt_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8088,
            .block = CPU_BLOCK(CPU_8088),
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 64,
            .max = 640,
            .step = 64
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_xtclone_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8086] Compaq Deskpro",
        .internal_name = "deskpro",
        .type = MACHINE_TYPE_8086,
        .chipset = MACHINE_CHIPSET_PROPRIETARY,
        .init = machine_xt_compaq_deskpro_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8086,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 128,
            .max = 640,
            .step = 128
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_xt_compaq_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8086] Schetmash Iskra-3104",
        .internal_name = "iskra3104",
        .type = MACHINE_TYPE_8086,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_xt_iskra3104_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8086,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 128,
            .max = 640,
            .step = 128
        },
        .nvrmask = 0,
        .kbc_device = &keyboard_xtclone_device,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[8086] Tandy 1000 SL/2",
        .internal_name = "tandy1000sl2",
        .type = MACHINE_TYPE_8086,
        .chipset = MACHINE_CHIPSET_PROPRIETARY,
        .init = machine_tandy1000sl2_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_8086,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PC,
        .flags = MACHINE_VIDEO_FIXED,
        .ram = {
            .min = 512,
            .max = 768,
            .step = 128
        },
        .nvrmask = 0,
        .kbc_device = NULL /* TODO: No specific kbd_device yet */,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = &vid_device_sl,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* 286 AT machines */
    /* Has IBM AT KBC firmware. */
    {
        .name = "[ISA] IBM AT",
        .internal_name = "ibmat",
        .type = MACHINE_TYPE_286,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_at_ibm_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_286,
            .block = CPU_BLOCK_NONE,
            .min_bus = 6000000,
            .max_bus = 8000000,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_AT,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 256,
            .max = 512,
            .step = 256
        },
        .nvrmask = 63,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Uses Compaq KBC firmware. */
    {
        .name = "[ISA] Compaq Portable II",
        .internal_name = "portableii",
        .type = MACHINE_TYPE_286,
        .chipset = MACHINE_CHIPSET_PROPRIETARY,
        .init = machine_at_portableii_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_286,
            .block = CPU_BLOCK_NONE,
            .min_bus = 6000000,
            .max_bus = 16000000,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_AT,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 640,
            .max = 16384,
            .step = 128
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Uses Compaq KBC firmware. */
    {
        .name = "[ISA] Compaq Portable III",
        .internal_name = "portableiii",
        .type = MACHINE_TYPE_286,
        .chipset = MACHINE_CHIPSET_PROPRIETARY,
        .init = machine_at_portableiii_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_286,
            .block = CPU_BLOCK_NONE,
            .min_bus = 6000000,
            .max_bus = 16000000,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_AT,
        .flags = MACHINE_IDE | MACHINE_VIDEO,
        .ram = {
            .min = 640,
            .max = 16384,
            .step = 128
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = &compaq_plasma_device,
        .snd_device = NULL,
        .net_device = NULL
    }, 
    /* Has IBM AT KBC firmware. */
    {
        .name = "[ISA] Phoenix IBM AT",
        .internal_name = "ibmatpx",
        .type = MACHINE_TYPE_286,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_at_ibmatpx_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_286,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_AT,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 256,
            .max = 512,
            .step = 256
        },
        .nvrmask = 63,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Has IBM PS/2 Type 2 KBC firmware. */
    {
        .name = "[MCA] IBM PS/2 model 60",
        .internal_name = "ibmps2_m60",
        .type = MACHINE_TYPE_286,
        .chipset = MACHINE_CHIPSET_PROPRIETARY,
        .init = machine_ps2_model_60_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_286 | CPU_PKG_486SLC_IBM,
            .block = CPU_BLOCK_NONE,
            .min_bus = 10000000,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PS2_MCA,
        .flags = MACHINE_VIDEO,
        .ram = {
            .min = 1024,
            .max = 10240,
            .step = 1024
        },
        .nvrmask = 63,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* 386SX machines */
    /* ISA slots available because an official IBM expansion for that existed. */
    /* Has IBM PS/2 Type 1 KBC firmware. */
    {
        .name = "[ISA] IBM PS/1 model 2121",
        .internal_name = "ibmps1_2121",
        .type = MACHINE_TYPE_386SX,
        .chipset = MACHINE_CHIPSET_PROPRIETARY,
        .init = machine_ps1_m2121_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_386SX,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PS2,
        .flags = MACHINE_IDE | MACHINE_VIDEO,
        .ram = {
            .min = 2048,
            .max = 6144,
            .step = 1024
        },
        .nvrmask = 63,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Has Quadtel KBC firmware. */
    {
        .name = "[ISA] QTC-SXM KT X20T02/HI",
        .internal_name = "quadt386sx",
        .type = MACHINE_TYPE_386SX,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_at_quadt386sx_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_386SX,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_AT,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 1024,
            .max = 16384,
            .step = 128
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Uses Compaq KBC firmware. */
    {
        .name = "[ISA] Compaq Deskpro 386 (September 1986)",
        .internal_name = "deskpro386",
        .type = MACHINE_TYPE_386DX,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_at_deskpro386_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_386DX_DESKPRO386,
            .block = CPU_BLOCK(CPU_486DLC, CPU_RAPIDCAD),
            .min_bus = 16000000,
            .max_bus = 25000000,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_AT,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 1024,
            .max = 16384,
            .step = 1024
        },
        .nvrmask = 63,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[ISA] Compaq Deskpro 386 (May 1988)",
        .internal_name = "deskpro386_05_1988",
        .type = MACHINE_TYPE_386DX,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_at_deskpro386_05_1988_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_386DX_DESKPRO386,
            .block = CPU_BLOCK(CPU_486DLC, CPU_RAPIDCAD),
            .min_bus = 16000000,
            .max_bus = 25000000,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_AT,
        .flags = MACHINE_FLAGS_NONE,
        .ram = {
            .min = 1024,
            .max = 16384,
            .step = 1024
        },
        .nvrmask = 63,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    {
        .name = "[ISA] Compaq Portable III (386)",
        .internal_name = "portableiii386",
        .type = MACHINE_TYPE_386DX,
        .chipset = MACHINE_CHIPSET_DISCRETE,
        .init = machine_at_portableiii386_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_386DX,
            .block = CPU_BLOCK_NONE,
            .min_bus = 20000000,
            .max_bus = 20000000,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_AT,
        .flags = MACHINE_IDE | MACHINE_VIDEO,
        .ram = {
            .min = 1024,
            .max = 14336,
            .step = 1024
        },
        .nvrmask = 63,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = &compaq_plasma_device,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Has IBM PS/2 Type 1 KBC firmware. */
    {
        .name = "[MCA] IBM PS/2 model 80 (type 3)",
        .internal_name = "ibmps2_m80_type3",
        .type = MACHINE_TYPE_386DX_486,
        .chipset = MACHINE_CHIPSET_PROPRIETARY,
        .init = machine_ps2_model_80_axx_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_386DX | CPU_PKG_486BL | CPU_PKG_SOCKET1,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PS2_MCA,
        .flags = MACHINE_VIDEO | MACHINE_APM,
        .ram = {
            .min = 2048,
            .max = 65536,
            .step = 2048
        },
        .nvrmask = 63,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* 486 machines - Socket 1 */
    /* Has AMI KF KBC firmware. */
    {
        .name = "[OPTi 381] Gigabyte GA-486L",
        .internal_name = "ga486l",
        .type = MACHINE_TYPE_486,
        .chipset = MACHINE_CHIPSET_OPTI_381,
        .init = machine_at_ga486l_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET1,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_AT,
        .flags = MACHINE_APM,
        .ram = {
            .min = 1024,
            .max = 16384,
            .step = 1024
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Has IBM PS/2 Type 1 KBC firmware. */
    {
        .name = "[MCA] IBM PS/2 model 70 (type 4)",
        .internal_name = "ibmps2_m70_type4",
        .type = MACHINE_TYPE_486,
        .chipset = MACHINE_CHIPSET_PROPRIETARY,
        .init = machine_ps2_model_70_type4_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET1,
            .block = CPU_BLOCK(CPU_i486SX, CPU_i486SX_SLENH, CPU_Am486SX, CPU_Cx486S),
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PS2_MCA,
        .flags = MACHINE_VIDEO | MACHINE_SOFTFLOAT_ONLY,
        .ram = {
            .min = 2048,
            .max = 65536,
            .step = 2048
        },
        .nvrmask = 63,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* 486 machines - Socket 2*/
    /* The BIOS does not send any non-standard keyboard controller commands and wants
       a PS/2 mouse, so it's an IBM PS/2 KBC (Type 1) firmware. */
    {
        .name = "[SiS 461] IBM PS/ValuePoint 433DX/Si",
        .internal_name = "valuepoint433",
        .type = MACHINE_TYPE_486_S2,
        .chipset = MACHINE_CHIPSET_SIS_461,
        .init = machine_at_valuepoint433_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET3,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PS2,
        .flags = MACHINE_IDE | MACHINE_VIDEO | MACHINE_APM,
        .ram = {
            .min = 1024,
            .max = 65536,
            .step = 1024
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* 486 machines - Socket 3 */
    /* Uses an NEC 90M002A (UPD82C42C, 8042 clone) with unknown firmware. */
    {
        .name = "[SiS 461] Acer V10",
        .internal_name = "acerv10",
        .type = MACHINE_TYPE_486_S3,
        .chipset = MACHINE_CHIPSET_SIS_461,
        .init = machine_at_acerv10_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET3,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PS2_VLB,
        .flags = MACHINE_IDE | MACHINE_APM, /* Machine has internal SCSI: Adaptec AIC-6360 */
        .ram = {
            .min = 1024,
            .max = 32768,
            .step = 1024
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Has the ALi M1487/9's on-chip keyboard controller which clones a standard AT
       KBC.
       The BIOS string always ends in -U, but the BIOS will send AMIKey commands 0xCA
       and 0xCB if command 0xA1 returns a letter in the 0x5x or 0x7x ranges, so I'm
       going to give it an AMI 'U' KBC. */
    {
        .name = "[ALi M1489] AMI WinBIOS 486 PCI",
        .internal_name = "win486pci",
        .type = MACHINE_TYPE_486_S3,
        .chipset = MACHINE_CHIPSET_ALI_M1489,
        .init = machine_at_win486pci_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET3,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PCI,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM,
        .ram = {
            .min = 1024,
            .max = 65536,
            .step = 1024
        },
        .nvrmask = 255,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Has IBM PS/2 Type 1 KBC firmware. */
    {
        .name = "[OPTi 802G] IBM PC 330 (type 6573)",
        .internal_name = "pc330_6573",
        .type = MACHINE_TYPE_486_S3,
        .chipset = MACHINE_CHIPSET_OPTI_895_802G,
        .init = machine_at_pc330_6573_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET3_PC330,
            .block = CPU_BLOCK_NONE,
            .min_bus = 25000000,
            .max_bus = 33333333,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 2.0,
            .max_multi = 3.0
        },
        .bus_flags = MACHINE_PS2_PCI,
        .flags = MACHINE_IDE | MACHINE_VIDEO | MACHINE_APM,
        .ram = {
            .min = 1024,
            .max = 65536,
            .step = 1024
        },
        .nvrmask = 255,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = &gd5430_onboard_vlb_device,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* This has the Phoenix MultiKey KBC firmware. */
    {
        .name = "[i420TX] Intel Classic/PCI",
        .internal_name = "alfredo",
        .type = MACHINE_TYPE_486_S3,
        .chipset = MACHINE_CHIPSET_INTEL_420TX,
        .init = machine_at_alfredo_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET3,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PS2_PCI,
        .flags = MACHINE_IDE | MACHINE_APM,
        .ram = {
            .min = 2048,
            .max = 131072,
            .step = 2048
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* This has an AMIKey-2, which is an updated version of type 'H'. Also has a
       SST 29EE010 Flash chip. */
    {
        .name = "[i420ZX] ASUS PCI/I-486SP3G",
        .internal_name = "486sp3g",
        .type = MACHINE_TYPE_486_S3,
        .chipset = MACHINE_CHIPSET_INTEL_420ZX,
        .init = machine_at_486sp3g_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET3,
            .block = CPU_BLOCK_NONE,
            .min_bus = 0,
            .max_bus = 0,
            .min_voltage = 0,
            .max_voltage = 0,
            .min_multi = 0,
            .max_multi = 0
        },
        .bus_flags = MACHINE_PS2_PCI,
        .flags = MACHINE_IDE | MACHINE_SCSI | MACHINE_APM,
        .ram = {
            .min = 1024,
            .max = 131072,
            .step = 1024
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Socket 4 machines */
    /* 430LX */
    /* Has IBM PS/2 Type 1 KBC firmware. */
    {
        .name = "[i430LX] Dell OptiPlex 560/L",
        .internal_name = "opti560l",
        .type = MACHINE_TYPE_SOCKET4,
        .chipset = MACHINE_CHIPSET_INTEL_430LX,
        .init = machine_at_opti560l_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET4,
            .block = CPU_BLOCK_NONE,
            .min_bus = 60000000,
            .max_bus = 66666667,
            .min_voltage = 5000,
            .max_voltage = 5000,
            .min_multi = MACHINE_MULTIPLIER_FIXED,
            .max_multi = MACHINE_MULTIPLIER_FIXED
        },
        .bus_flags = MACHINE_PS2_PCI,
        .flags = MACHINE_IDE | MACHINE_APM,
        .ram = {
            .min = 2048,
            .max = 131072,
            .step = 2048
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Socket 5 machines */
    /* 430NX */
    /* This has the Phoenix MultiKey KBC firmware. */
    {
        .name = "[i430NX] Intel Premiere/PCI II",
        .internal_name = "plato",
        .type = MACHINE_TYPE_SOCKET5,
        .chipset = MACHINE_CHIPSET_INTEL_430NX,
        .init = machine_at_plato_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET5_7,
            .block = CPU_BLOCK_NONE,
            .min_bus = 50000000,
            .max_bus = 66666667,
            .min_voltage = 3520,
            .max_voltage = 3520,
            .min_multi = 1.5,
            .max_multi = 1.5
        },
        .bus_flags = MACHINE_PS2_PCI,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM,
        .ram = {
            .min = 2048,
            .max = 131072,
            .step = 2048
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Socket 7 (Single Voltage) machines */
    /* 430FX */
    /* This has an AMIKey-2, which is an updated version of type 'H'.
       This also seems to be revision 2.1 with the FDC37C665 SIO. */
    {
        .name = "[i430FX] ASUS P/I-P55TP4XE",
        .internal_name = "p54tp4xe",
        .type = MACHINE_TYPE_SOCKET7_3V,
        .chipset = MACHINE_CHIPSET_INTEL_430FX,
        .init = machine_at_p54tp4xe_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET5_7,
            .block = CPU_BLOCK_NONE,
            .min_bus = 50000000,
            .max_bus = 66666667,
            .min_voltage = 3380,
            .max_voltage = 3600,
            .min_multi = 1.5,
            .max_multi = 3.0
        },
        .bus_flags = MACHINE_PS2_PCI,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM,
        .ram = {
            .min = 8192,
            .max = 131072,
            .step = 8192
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* 430VX */
    /* Has a SM(S)C FDC37C932FR Super I/O chip with on-chip KBC with AMI
       MegaKey (revision '5') KBC firmware. */
    {
        .name = "[i430VX] Gateway 2000 Tigereye",
        .internal_name = "gw2kte",
        .type = MACHINE_TYPE_SOCKET7_3V,
        .chipset = MACHINE_CHIPSET_INTEL_430VX,
        .init = machine_at_gw2kte_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET5_7,
            .block = CPU_BLOCK_NONE,
            .min_bus = 50000000,
            .max_bus = 66666667,
            .min_voltage = 3380,
            .max_voltage = 3520,
            .min_multi = 1.5,
            .max_multi = 3.0
        },
        .bus_flags = MACHINE_PS2_PCI | MACHINE_BUS_USB,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM | MACHINE_GAMEPORT | MACHINE_USB,
        .ram = {
            .min = 8192,
            .max = 131072,
            .step = 8192
        },
        .nvrmask = 511,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Socket 7 (Dual Voltage) machines */
    /* OEM version of Intel TC430HX, has AMI MegaKey KBC firmware on the PC87306 Super I/O chip. */
    {
        .name = "[i430HX] Toshiba Infinia 7200",
        .internal_name = "infinia7200",
        .type = MACHINE_TYPE_SOCKET7,
        .chipset = MACHINE_CHIPSET_INTEL_430HX,
        .init = machine_at_infinia7200_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET5_7,
            .block = CPU_BLOCK_NONE,
            .min_bus = 50000000,
            .max_bus = 66666667,
            .min_voltage = 2800,
            .max_voltage = 3520,
            .min_multi = 1.5,
            .max_multi = 3.0
        },
        .bus_flags = MACHINE_PS2_PCI | MACHINE_BUS_USB,
        .flags = MACHINE_VIDEO | MACHINE_IDE_DUAL | MACHINE_APM | MACHINE_GAMEPORT | MACHINE_USB, /* Has internal sound: Yamaha YMF701-S */
        .ram = {
            .min = 8192,
            .max = 131072,
            .step = 8192
        },
        .nvrmask = 255,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = &s3_virge_375_pci_device,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* This has a Holtek KBC and the BIOS does not send a single non-standard KBC command, so it
       must be an ASIC that clones the standard IBM PS/2 KBC. */
    {
        .name = "[i430VX] Shuttle HOT-557",
        .internal_name = "430vx",
        .type = MACHINE_TYPE_SOCKET7,
        .chipset = MACHINE_CHIPSET_INTEL_430VX,
        .init = machine_at_i430vx_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET5_7,
            .block = CPU_BLOCK_NONE,
            .min_bus = 50000000,
            .max_bus = 66666667,
            .min_voltage = 2500,
            .max_voltage = 3520,
            .min_multi = 1.5,
            .max_multi = 3.0
        },
        .bus_flags = MACHINE_PS2_PCI | MACHINE_BUS_USB,
        .flags = MACHINE_IDE_DUAL | MACHINE_GAMEPORT | MACHINE_APM | MACHINE_USB,
        .ram = {
            .min = 8192,
            .max = 131072,
            .step = 8192
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },

    /* 430TX */
    /* This has the AMIKey KBC firmware, which is an updated 'F' type (YM430TX is based on the TX97). */
    {
        .name = "[i430TX] ASUS TX97",
        .internal_name = "tx97",
        .type = MACHINE_TYPE_SOCKET7,
        .chipset = MACHINE_CHIPSET_INTEL_430TX,
        .init = machine_at_tx97_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET5_7,
            .block = CPU_BLOCK_NONE,
            .min_bus = 50000000,
            .max_bus = 75000000,
            .min_voltage = 2500,
            .max_voltage = 3520,
            .min_multi = 1.5,
            .max_multi = 3.0
        },
        .bus_flags = MACHINE_PS2_PCI | MACHINE_BUS_USB,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM | MACHINE_ACPI | MACHINE_USB,
        .ram = {
            .min = 8192,
            .max = 262144,
            .step = 8192
        },
        .nvrmask = 255,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Has the SiS 5571 chipset with on-chip KBC. */
    {
        .name = "[SiS 5571] Rise R534F",
        .internal_name = "r534f",
        .type = MACHINE_TYPE_SOCKET7,
        .chipset = MACHINE_CHIPSET_SIS_5571,
        .init = machine_at_r534f_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET5_7,
            .block = CPU_BLOCK_NONE,
            .min_bus = 55000000,
            .max_bus = 83333333,
            .min_voltage = 2500,
            .max_voltage = 3520,
            .min_multi = 1.5,
            .max_multi = 3.0
        },
        .bus_flags = MACHINE_PS2_PCI | MACHINE_BUS_USB,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM | MACHINE_USB,
        .ram = {
            .min = 8192,
            .max = 393216,
            .step = 8192
        },
        .nvrmask = 255,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Super Socket 7 machines */
    /* M1534c kbc */
    {
        .name = "[ALi ALADDiN V] Gateway Lucas",
        .internal_name = "gwlucas",
        .type = MACHINE_TYPE_SOCKETS7,
        .chipset = MACHINE_CHIPSET_ALI_ALADDIN_V,
        .init = machine_at_gwlucas_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET5_7,
            .block = CPU_BLOCK_NONE,
            .min_bus = 66666667,
            .max_bus = 100000000,
            .min_voltage = 2000,
            .max_voltage = 3520,
            .min_multi = 1.5,
            .max_multi = 5.5
        },
        .bus_flags = MACHINE_PS2_PCIONLY | MACHINE_BUS_USB,
        .flags = MACHINE_IDE_DUAL | MACHINE_SOUND | MACHINE_APM | MACHINE_ACPI | MACHINE_GAMEPORT | MACHINE_USB, /* Has internal video: ATI 3D Rage Pro Turbo AGP and sound: Ensoniq ES1373 */
        .ram = {
            .min = 8192,
            .max = 262144,
            .step = 8192
        },
        .nvrmask = 255,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = &es1373_onboard_device,
        .net_device = NULL
    },
    /* Slot 1 machines */
    /* 440FX */
    /* The base board has a Holtek HT6542B KBC with AMIKey-2 (updated 'H') KBC firmware. */
    {
        .name = "[i440FX] ASUS P/I-P65UP5 (C-PKND)",
        .internal_name = "p65up5_cpknd",
        .type = MACHINE_TYPE_SLOT1,
        .chipset = MACHINE_CHIPSET_INTEL_440FX,
        .init = machine_at_p65up5_cpknd_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SLOT1,
            .block = CPU_BLOCK_NONE,
            .min_bus = 50000000,
            .max_bus = 66666667,
            .min_voltage = 1800,
            .max_voltage = 3500,
            .min_multi = 1.5,
            .max_multi = 8.0
        },
        .bus_flags = MACHINE_PS2_PCI | MACHINE_BUS_USB,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM | MACHINE_USB,
        .ram = {
            .min = 8192,
            .max = 1048576,
            .step = 8192
        },
        .nvrmask = 127,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Has a Winbond W83977TF Super I/O chip with on-chip KBC with AMIKey-2 KBC
       firmware. */
    {
        .name = "[i440BX] Gigabyte GA-686BX",
        .internal_name = "686bx",
        .type = MACHINE_TYPE_SLOT1,
        .chipset = MACHINE_CHIPSET_INTEL_440BX,
        .init = machine_at_686bx_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SLOT1,
            .block = CPU_BLOCK_NONE,
            .min_bus = 66666667,
            .max_bus = 100000000,
            .min_voltage = 1800,
            .max_voltage = 3500,
            .min_multi = 1.5,
            .max_multi = 8.0
        },
        .bus_flags = MACHINE_PS2_AGP | MACHINE_BUS_USB,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM | MACHINE_ACPI | MACHINE_USB,
        .ram = {
            .min = 8192,
            .max = 1048576,
            .step = 8192
        },
        .nvrmask = 255,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* VIA Apollo Pro */
    /* Has a Winbond W83977EF Super I/O chip with on-chip KBC with AMIKey-2 KBC
       firmware. */
    {
        .name = "[VIA Apollo Pro 133A] ASUS P3V4X",
        .internal_name = "p3v4x",
        .type = MACHINE_TYPE_SLOT1,
        .chipset = MACHINE_CHIPSET_VIA_APOLLO_PRO_133A,
        .init = machine_at_p3v4x_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SLOT1,
            .block = CPU_BLOCK_NONE,
            .min_bus = 66666667,
            .max_bus = 150000000,
            .min_voltage = 1300,
            .max_voltage = 3500,
            .min_multi = 1.5,
            .max_multi = 8.0
        },
        .bus_flags = MACHINE_PS2_AGP | MACHINE_BUS_USB,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM | MACHINE_ACPI | MACHINE_USB,
        .ram = {
            .min = 8192,
            .max = 2097152,
            .step = 8192
        },
        .nvrmask = 255,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* Slot 2 machines */
    /* 440GX */
    /* Has a Winbond W83977EF Super I/O chip with on-chip KBC with AMIKey-2 KBC
       firmware. */
    {
        .name = "[i440GX] Gigabyte GA-6GXU",
        .internal_name = "6gxu",
        .type = MACHINE_TYPE_SLOT2,
        .chipset = MACHINE_CHIPSET_INTEL_440GX,
        .init = machine_at_6gxu_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SLOT2,
            .block = CPU_BLOCK_NONE,
            .min_bus = 100000000,
            .max_bus = 133333333,
            .min_voltage = 1800,
            .max_voltage = 3500,
            .min_multi = 1.5,
            .max_multi = 8.0
        },
        .bus_flags = MACHINE_PS2_AGP | MACHINE_BUS_USB,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM | MACHINE_ACPI | MACHINE_USB, /* Machine has internal SCSI */
        .ram = {
            .min = 16384,
            .max = 2097152,
            .step = 16384
        },
        .nvrmask = 511,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },

    /* Has a Winbond W83977EF Super I/O chip with on-chip KBC with AMIKey-2 KBC
       firmware. */
    {
        .name = "[i440BX] ASUS CUBX",
        .internal_name = "cubx",
        .type = MACHINE_TYPE_SOCKET370,
        .chipset = MACHINE_CHIPSET_INTEL_440BX,
        .init = machine_at_cubx_init,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = MACHINE_AVAILABLE,
        .gpio_acpi_handler = NULL,
        .cpu = {
            .package = CPU_PKG_SOCKET370,
            .block = CPU_BLOCK_NONE,
            .min_bus = 66666667,
            .max_bus = 150000000,
            .min_voltage = 1300,
            .max_voltage = 3500,
            .min_multi = 1.5,
            .max_multi = 8.0
        },
        .bus_flags = MACHINE_PS2_AGP | MACHINE_BUS_USB,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM | MACHINE_ACPI | MACHINE_USB, /* Machine has quad channel IDE with internal controller: CMD PCI-0648 */
        .ram = {
            .min = 8192,
            .max = 1048576,
            .step = 8192
        },
        .nvrmask = 255,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
    /* internal_name null in last entry required to not crash wtf?? */
    {
        .name = NULL,
        .internal_name = NULL,
        .type = MACHINE_TYPE_NONE,
        .chipset = MACHINE_CHIPSET_NONE,
        .init = NULL,
        .p1_handler = NULL,
        .gpio_handler = NULL,
        .available_flag = 0,
        .gpio_acpi_handler = NULL,
        .cpu = { // bogus
            .package = CPU_PKG_SOCKET370,
            .block = CPU_BLOCK_NONE,
            .min_bus = 66666667,
            .max_bus = 150000000,
            .min_voltage = 1300,
            .max_voltage = 3500,
            .min_multi = 1.5,
            .max_multi = 8.0
        },
        .bus_flags = MACHINE_PS2_AGP | MACHINE_BUS_USB,
        .flags = MACHINE_IDE_DUAL | MACHINE_APM | MACHINE_ACPI | MACHINE_USB, /* Machine has quad channel IDE with internal controller: CMD PCI-0648 */
        .ram = {
            .min = 8192,
            .max = 1048576,
            .step = 8192
        },
        .nvrmask = 255,
        .kbc_device = NULL,
        .kbc_p1 = 0xff,
        .gpio = 0xffffffff,
        .gpio_acpi = 0xffffffff,
        .device = NULL,
        .fdc_device = NULL,
        .sio_device = NULL,
        .vid_device = NULL,
        .snd_device = NULL,
        .net_device = NULL
    },
  // clang-format on
};

/* Saved copies - jumpers get applied to these.
   We use also machine_gpio to store IBM PC/XT jumpers as they need more than one byte. */
static uint8_t machine_p1_default;
static uint8_t machine_p1;

static uint32_t machine_gpio_default;
static uint32_t machine_gpio;

static uint32_t machine_gpio_acpi_default;
static uint32_t machine_gpio_acpi;

void *machine_snd = NULL;

uint8_t
machine_get_p1_default(void)
{
    return machine_p1_default;
}

uint8_t
machine_get_p1(void)
{
    return machine_p1;
}

void
machine_set_p1_default(uint8_t val)
{
    machine_p1 = machine_p1_default = val;
}

void
machine_set_p1(uint8_t val)
{
    machine_p1 = val;
}

void
machine_and_p1(uint8_t val)
{
    machine_p1 = machine_p1_default & val;
}

uint8_t
machine_handle_p1(uint8_t write, uint8_t val)
{
    uint8_t ret = 0xff;

    if (machines[machine].p1_handler)
        ret = machines[machine].p1_handler(write, val);
    else {
        if (write)
            machine_p1 = machine_p1_default & val;
        else
            ret = machine_p1;
    }

    return ret;
}

void
machine_init_p1(void)
{
    machine_p1 = machine_p1_default = machines[machine].kbc_p1;
}

uint32_t
machine_get_gpio_default(void)
{
    return machine_gpio_default;
}

uint32_t
machine_get_gpio(void)
{
    return machine_gpio;
}

void
machine_set_gpio_default(uint32_t val)
{
    machine_gpio = machine_gpio_default = val;
}

void
machine_set_gpio(uint32_t val)
{
    machine_gpio = val;
}

void
machine_and_gpio(uint32_t val)
{
    machine_gpio = machine_gpio_default & val;
}

uint32_t
machine_handle_gpio(uint8_t write, uint32_t val)
{
    uint32_t ret = 0xffffffff;

    if (machines[machine].gpio_handler)
        ret = machines[machine].gpio_handler(write, val);
    else {
        if (write)
            machine_gpio = machine_gpio_default & val;
        else
            ret = machine_gpio;
    }

    return ret;
}

void
machine_init_gpio(void)
{
    machine_gpio = machine_gpio_default = machines[machine].gpio;
}

uint32_t
machine_get_gpio_acpi_default(void)
{
    return machine_gpio_acpi_default;
}

uint32_t
machine_get_gpio_acpi(void)
{
    return machine_gpio_acpi;
}

void
machine_set_gpio_acpi_default(uint32_t val)
{
    machine_gpio_acpi = machine_gpio_acpi_default = val;
}

void
machine_set_gpio_acpi(uint32_t val)
{
    machine_gpio_acpi = val;
}

void
machine_and_gpio_acpi(uint32_t val)
{
    machine_gpio_acpi = machine_gpio_acpi_default & val;
}

uint32_t
machine_handle_gpio_acpi(uint8_t write, uint32_t val)
{
    uint32_t ret = 0xffffffff;

    if (machines[machine].gpio_acpi_handler)
        ret = machines[machine].gpio_acpi_handler(write, val);
    else {
        if (write)
            machine_gpio_acpi = machine_gpio_acpi_default & val;
        else
            ret = machine_gpio_acpi;
    }

    return ret;
}

void
machine_init_gpio_acpi(void)
{
    machine_gpio_acpi = machine_gpio_acpi_default = machines[machine].gpio_acpi;
}

int
machine_count(void)
{
    return ((sizeof(machines) / sizeof(machine_t)) - 1);
}

const char *
machine_getname(void)
{
    return (machines[machine].name);
}

const char *
machine_getname_ex(int m)
{
    return (machines[m].name);
}

const device_t *
machine_get_kbc_device(int m)
{
    if (machines[m].kbc_device)
        return (machines[m].kbc_device);

    return (NULL);
}

const device_t *
machine_get_device(int m)
{
    if (machines[m].device)
        return (machines[m].device);

    return (NULL);
}

const device_t *
machine_get_fdc_device(int m)
{
    if (machines[m].fdc_device)
        return (machines[m].fdc_device);

    return (NULL);
}

const device_t *
machine_get_sio_device(int m)
{
    if (machines[m].sio_device)
        return (machines[m].sio_device);

    return (NULL);
}

const device_t *
machine_get_vid_device(int m)
{
    if (machines[m].vid_device)
        return (machines[m].vid_device);

    return (NULL);
}

const device_t *
machine_get_snd_device(int m)
{
    if (machines[m].snd_device)
        return (machines[m].snd_device);

    return (NULL);
}

const device_t *
machine_get_net_device(int m)
{
    if (machines[m].net_device)
        return (machines[m].net_device);

    return (NULL);
}

const char *
machine_get_internal_name(void)
{
    return (machines[machine].internal_name);
}

const char *
machine_get_internal_name_ex(int m)
{
    return (machines[m].internal_name);
}

int
machine_get_nvrmask(int m)
{
    return (machines[m].nvrmask);
}

int
machine_has_flags(int m, int flags)
{
    return (machines[m].flags & flags);
}

int
machine_has_bus(int m, int bus_flags)
{
    return (machines[m].bus_flags & bus_flags);
}

int
machine_has_cartridge(int m)
{
    return (machine_has_bus(m, MACHINE_CARTRIDGE) ? 1 : 0);
}

int
machine_get_min_ram(int m)
{
    return (machines[m].ram.min);
}

int
machine_get_max_ram(int m)
{
#if (!(defined __amd64__ || defined _M_X64 || defined __aarch64__ || defined _M_ARM64))
    return MIN(((int) machines[m].ram.max), 2097152);
#else
    return MIN(((int) machines[m].ram.max), 3145728);
#endif
}

int
machine_get_ram_granularity(int m)
{
    return (machines[m].ram.step);
}

int
machine_get_type(int m)
{
    return (machines[m].type);
}

int
machine_get_machine_from_internal_name(const char *s)
{
    int c = 0;

    while (machines[c].init != NULL) {
        if (!strcmp(machines[c].internal_name, s))
            return c;
        c++;
    }

    return 0;
}

int
machine_has_mouse(void)
{
    return (machines[machine].flags & MACHINE_MOUSE);
}
