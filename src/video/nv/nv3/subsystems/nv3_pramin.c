/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          NV3 PRAMIN - Basically, this is how we know what to render.
 *          Has a giant hashtable of all the submitted DMA objects using a pseudo-C++ class system
 *
 *
 *
 * Authors: Connor Hyde, <mario64crashed@gmail.com> I need a better email address ;^)
 *
 *          Copyright 2024 starfrost
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <86Box/86box.h>
#include <86Box/device.h>
#include <86Box/mem.h>
#include <86box/pci.h>
#include <86Box/rom.h> // DEPENDENT!!!
#include <86Box/video.h>
#include <86Box/nv/vid_nv.h>
#include <86Box/nv/vid_nv3.h>

// i believe the main loop is to walk the hashtable in RAMIN (last 0.5 MB of VRAM), 
// find the objects that were submitted from DMA 
// (going from software -> nvidia d3d / ogl implementation -> resource manager client -> nvapi -> nvrm -> GPU PFIFO -> GPU PBUS -> GPU PFB RAMIN) 
// and then rendering each of those
