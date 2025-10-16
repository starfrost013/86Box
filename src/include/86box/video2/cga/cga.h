/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          New CGA device
 *
 * Authors: Connor Hyde, <mario64crashed@gmail.com>
 * 
 *          Copyright 2025 Connor Hyde.
 */

#pragma once

#include <86box/video2/base/mc6845.h>

/* I am CGA! */

typedef struct cga_s
{
    uint8_t crtc[CRTC_NUM_REGISTERS];
    pc_timer_t character_clock_timer;
} cga_t;