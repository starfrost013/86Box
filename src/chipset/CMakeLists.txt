#
# 86Box    A hypervisor and IBM PC system emulator that specializes in
#          running old operating systems and software designed for IBM
#          PC systems and compatibles from 1981 through fairly recent
#          system designs based on the PCI bus.
#
#          This file is part of the 86Box distribution.
#
#          CMake build script.
#
# Authors: David Hrdlička, <hrdlickadavid@outlook.com>
#
#          Copyright 2020-2021 David Hrdlička.
#

add_library(chipset OBJECT 82c100.c acc2168.c cs8230.c ali1429.c ali1489.c
    ali1541.c ali1543.c headland.c ims8848.c
    compaq_386.c contaq_82c59x.c cs4031.c intel_420ex.c intel_4x0.c intel_i450kx.c
    intel_sio.c intel_piix.c ../ioapic.c opti391.c
    opti602.c opti822.c opti895.c scamp.c sis_85c4xx.c
    sis_85c50x.c sis_5513_ide.c sis_5513_p2i.c sis_5571.c sis_5571_h2p.c sis_5572_usb.c sis_55xx.c 
    via_vt82c49x.c via_vt82c505.c
     stpc.c
    umc_hb4.c umc_8890.c via_apollo.c via_pipc.c vl82c480.c wd76c10.c)
