/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          A better random number generation, used for floppy weak bits
 *          and network MAC address generation.
 *
 *
 *
 * Authors: Miran Grca, <mgrca8@gmail.com>
 *
 *          Copyright 2016-2018 Miran Grca.
 */
#include <stdint.h>
#include <stdlib.h>
#include <86box/random.h>

// Same function in different headers
#ifdef _MSC_VER
    #include <intrin.h>     
#else
    #include <x86intrin.h>
#endif

#if defined(__x86_64__)
#    include <time.h>
#endif

uint32_t preconst = 0x6ED9EBA1;

static __inline uint32_t
rotl32c(uint32_t x, uint32_t n)
{

    return (x << n) | (x >> (-n & 31));
}

static __inline uint32_t
rotr32c(uint32_t x, uint32_t n)
{
    return (x >> n) | (x << (-n & 31));
}

#define ROTATE_LEFT  rotl32c

#define ROTATE_RIGHT rotr32c

static __inline unsigned long long
rdtsc(void)
{
    // intrinsic, spits out "rdtsc" instruction
#if defined(__x86_64__)
        return __rdtsc(); // same shit
#else
    return time(NULL);
#endif
}

static uint32_t
RDTSC(void)
{
    return (uint32_t) (rdtsc());
}

static void
random_twist(uint32_t *val)
{
    *val = ROTATE_LEFT(*val, rand() % 32);
    *val ^= 0x5A827999;
    *val = ROTATE_RIGHT(*val, rand() % 32);
    *val ^= 0x4ED32706;
}

uint8_t
random_generate(void)
{
    uint16_t r = 0;
    r          = (RDTSC() ^ ROTATE_LEFT(preconst, rand() % 32)) % 256;
    random_twist(&preconst);
    return (r & 0xff);
}

void
random_init(void)
{
    uint32_t seed = RDTSC();
    srand(seed);
    return;
}
