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
# Authors: Connor Hyde / starfrost, <mario64crashed@gmail.com>
#
#          Copyright 2025 starfrost
#

add_library(video2 OBJECT
    video_conversion.c
    video_core.c
)