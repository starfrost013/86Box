/*
 * 86Box    A hypervisor and IBM PC system emulator that specializes in
 *          running old operating systems and software designed for IBM
 *          PC systems and compatibles from 1981 through fairly recent
 *          system designs based on the PCI bus.
 *
 *          This file is part of the 86Box distribution.
 *
 *          STG1732 (Van Gogh) / 1764 (Van Dyke; aka NVDAC64) 
 *          32-bit colour (64-bit pixel input on STG1764) DAC - SGS-Thomson/Nvidia
 *          Used for NVidia NV1. Based on 1702 code
 *
 * Authors: Sarah Walker, <https://pcem-emulator.co.uk/>
 *          Miran Grca, <mgrca8@gmail.com>
 *          Connor Hyde, <mario64crashed@gmail.com>
 *
 *          Copyright 2008-2018 Sarah Walker.
 *          Copyright 2016-2018 Miran Grca.
 *          Copyright 2026 starfrost
 *          This DAC has no vga interface
 */

#include "vid_ramdac_stg1732.h"
// temp
#include "../nv/nv1/nv1.h"

//
// DEFINES
// (local bitflags)
//
#define LOCAL_IS_STG1764            0x01
#define PALETTE_SIZE                256
#define LUT_SIZE                    768     // R,G,B components

typedef struct stg1732_ramdac_t {
    uint16_t index;
    uint32_t config_0, config_1;            // config registers 
    uint8_t write_pal_addr, read_pal_addr;  // read/write addr's for lut
    uint8_t write_lut_addr, read_lut_addr;  
    uint8_t palette[PALETTE_SIZE];
    uint8_t lut[LUT_SIZE];
    
    uint8_t data;       // need to save this because this is written separately and can receive data from several rgisters
    int     type;
} stg1732_ramdac_t;

//
// uPort I/O
//
uint8_t stg1732_ramdac_uport_read(uint8_t addr, void* priv, svga_t* svga)
{
    stg1732_ramdac_t *ramdac = (stg1732_ramdac_t *) priv;
    uint8_t ret = 0x00;

    switch (addr)
    {
        // i think this is how this is supposed tow ork
        case SGS_DAC_UPORT_READ_PAL_ADDR:
            ret = ramdac->palette[ramdac->read_pal_addr];
            ramdac->read_pal_addr++;
            ramdac->read_pal_addr &= (PALETTE_SIZE - 1);
            break;
        case SGS_DAC_UPORT_COLOR:
            ret = ramdac->lut[ramdac->read_lut_addr];
            ramdac->read_lut_addr++;
            ramdac->read_lut_addr &= (LUT_SIZE - 1);
            break;
        case SGS_DAC_UPORT_INDEX_HI:
            ret = (ramdac->index >> 8) & 0xFF;
            break;
        case SGS_DAC_UPORT_INDEX_LO:
            ret = (ramdac->index) & 0xFF;
            break;
        case SGS_DAC_UPORT_INDEX_DATA:
            ret = stg1732_ramdac_reg_read(ramdac->index, ramdac, svga);
            break;
        case SGS_DAC_UPORT_PIXEL_MASK:
            // i think this can be straight into here  
            ret = svga->dac_mask;
            break;
    }

    if (addr != SGS_DAC_UPORT_INDEX_DATA)
        nv_log("STG1764 uport read %02x from %02x\n", ret, addr);

    return ret; 
}

void stg1732_ramdac_uport_write(uint8_t addr, uint8_t val, void* priv, svga_t* svga)
{
    stg1732_ramdac_t *ramdac = (stg1732_ramdac_t *) priv;

    switch (addr)
    {
        // i think this is how this is supposed tow ork
        case SGS_DAC_UPORT_WRITE_PAL_ADDR:
            ramdac->palette[ramdac->write_pal_addr] = val;
            ramdac->write_pal_addr++;
            ramdac->write_pal_addr &= (PALETTE_SIZE - 1);
            break;
        case SGS_DAC_UPORT_COLOR:
            ramdac->lut[ramdac->write_lut_addr] = val;
            ramdac->write_lut_addr++;
            ramdac->write_lut_addr &= (LUT_SIZE - 1);
            break;
        case SGS_DAC_UPORT_INDEX_HI:    
            ramdac->index = (val << 8) | (ramdac->index & 0xFF);
            break;
        case SGS_DAC_UPORT_INDEX_LO:
            ramdac->index = (ramdac->index & 0xFF00) | (val & 0xFF);
            break;
        case SGS_DAC_UPORT_INDEX_DATA:
            stg1732_ramdac_reg_write(addr, val, priv, svga);
            break;
    }

    if (addr != SGS_DAC_UPORT_INDEX_DATA)
        nv_log("STG1764 uport write %02x to %02x\n", val, addr);
}

void
stg1732_ramdac_set_bpp(svga_t *svga, stg1732_ramdac_t *ramdac)
{
    switch (ramdac->config_0 & 0x03)
    {
        case 0:
            svga->bpp = 4;
            break;
        case 1:
            svga->bpp = 8;
            break;
        case 2:
            svga->bpp = 16;
            break;
        case 3:
            svga->bpp = 32;
            break;
    }

    svga_recalctimings(svga);
}

uint8_t
stg1732_ramdac_reg_read(uint16_t addr, void *priv, svga_t *svga)
{
    stg1732_ramdac_t *ramdac = (stg1732_ramdac_t *) priv;
    uint8_t       ret   = 0xff;

    switch (addr)
    {
        case SGS_DAC_VENDOR_ID:
            ret = SGS_DAC_VENDOR_ID_SGS;
            break; 
        case SGS_DAC_DEVICE_ID:
            if (ramdac->type == LOCAL_IS_STG1764)
                ret = SGS_DAC_DEVICE_ID_VAN_DYKE;
            else
                ret = SGS_DAC_DEVICE_ID_VAN_GOGH;
            break;
        case SGS_DAC_CONFIG_0:
            ret = ramdac->config_0;
            break;
        case SGS_DAC_CONFIG_1:
            ret = ramdac->config_1;
            break;
    }
    
    nv_log("STG1764 register read %02x from %02x\n", ret, addr);
    return ret;
}

void
stg1732_ramdac_reg_write(uint16_t addr, uint8_t val, void *priv, svga_t *svga)
{
    stg1732_ramdac_t *ramdac = (stg1732_ramdac_t *) priv;

    switch (addr) 
    {
        case SGS_DAC_CONFIG_0:
            ramdac->config_0 = val;
            // todo: act on other stuff
            stg1732_ramdac_set_bpp(svga, ramdac);
            break;
        case SGS_DAC_CONFIG_1:
            ramdac->config_1 = val;
            break;
    }

    nv_log("STG1764 register write %02x to %02x\n", val, addr);
}

float
stg1732_getclock(int clock, void *priv)
{
    stg1732_ramdac_t   *ramdac = (stg1732_ramdac_t *) priv;
    float           t;
    int             m;
    int             n;
    int             d;
    int             d2;
    uint16_t        c;

    /*
    if (clock == 0)
        return 25175000.0;
    if (clock == 1)
        return 28322000.0;

    c  = ramdac->regs[0x20 + (clock << 1)];
    c |= (ramdac->regs[0x21 + (clock << 1)] << 8);
    m  = (c & 0xff) + 2;        /* B+2 
    n  = ((c >> 8) & 0x1f) + 2; /* N1+2 
    d = ((c >> 13) & 0x07);    /* D 
    d2 = (1 << d);
    t  = (14318184.0f * (float) m) / (float) (n * d2);
*/
    //pclog("RAMDAC vclk val=0x%02x, vclk v=%d low, D=%d, t=%f.\n", ramdac->regs[0x20 + (clock << 1)], clock, d2, t);
    return t;
}

static void *
stg1732_ramdac_init(UNUSED(const device_t *info))
{
    stg1732_ramdac_t *ramdac = (stg1732_ramdac_t *) calloc(1, sizeof(stg1732_ramdac_t));
    ramdac->type = info->local & 0xff;
    return ramdac;
}

static void
stg1732_ramdac_close(void *priv)
{
    stg1732_ramdac_t *ramdac = (stg1732_ramdac_t *) priv;

    if (ramdac)
        free(ramdac);
}

const device_t stg1732_ramdac_device = {
    .name          = "SGS-Thompson/nVIDIA STG1732 \"Van Gogh\" RAMDAC",
    .internal_name = "stg1732_ramdac",
    .flags         = 0,
    .local         = 0,
    .init          = stg1732_ramdac_init,
    .close         = stg1732_ramdac_close,
    .reset         = NULL,
    .available     = NULL,
    .speed_changed = NULL,
    .force_redraw  = NULL,
    .config       = NULL
};

const device_t stg1764_ramdac_device = {
    .name          = "SGS-Thompson/nVIDIA STG1764X (NVDAC64) \"Van Dyke\" RAMDAC",
    .internal_name = "stg1732_ramdac",
    .flags         = 0,
    .local         = LOCAL_IS_STG1764,
    .init          = stg1732_ramdac_init,
    .close         = stg1732_ramdac_close,
    .reset         = NULL,
    .available     = NULL,
    .speed_changed = NULL,
    .force_redraw  = NULL,
    .config        = NULL
};
