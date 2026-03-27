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
#define DAC_IS_STG1764            0x01
#define PALETTE_SIZE                256
#define LUT_SIZE                    768     // R,G,B components

typedef struct stg1732_ramdac_t {
    uint16_t index;
    uint32_t config_0, config_1;            // config registers 
    uint8_t write_pal_addr, read_pal_addr;  // read/write addr's for lut
    uint8_t write_lut_addr, read_lut_addr;  

    // I don't think O factor is used (it's always 1)
    uint8_t vpll_m, vpll_n, vpll_o, vpll_p; // Video PLL
    uint8_t mpll_m, mpll_n, mpll_o, mpll_p; // Core & Memory PLL
    uint8_t apll_m, apll_n, apll_o, apll_p; // Audio PLL

    uint32_t vpll_hz;                       // VPLL clock speed in hertz
    uint32_t mpll_hz;                       // MPLL clock speed in hertz
    uint32_t apll_hz;                       // APLL clock speed in hertz

    uint8_t palette_control;                // controls the DAC state

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

    if (addr != SGS_DAC_UPORT_INDEX_DATA
    && addr != SGS_DAC_UPORT_INDEX_HI
    && addr != SGS_DAC_UPORT_INDEX_LO)
        nv_log("STG1764 uPort read 0x%02x from 0x%02x\n", ret, addr);

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
            stg1732_ramdac_reg_write(ramdac->index, val, priv, svga);
            break;
    }

    if (addr != SGS_DAC_UPORT_INDEX_DATA
    && addr != SGS_DAC_UPORT_INDEX_HI
    && addr != SGS_DAC_UPORT_INDEX_LO)
        nv_log("STG1764 uPort write 0x%02x to 0x%02x\n", val, addr);
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

// Set the clock speed 
void stg1732_set_clock_speed(uint16_t addr, void* priv)
{
    stg1732_ramdac_t *ramdac = (stg1732_ramdac_t *)priv;
    
    // OPLl does not matter and is always 1
    switch (addr)
    {
        case SGS_DAC_APLL_P:
            ramdac->apll_hz = ((STG1732_BASE_CLOCK * ramdac->apll_n) * (ramdac->apll_m << ramdac->apll_p));
            nv_log("Audio clock is now %d MHz\n", (float)(ramdac->apll_hz) / 1000.0f);
            break; 
        case SGS_DAC_VPLL_P:
            ramdac->vpll_hz = ((STG1732_BASE_CLOCK * ramdac->vpll_n) * (ramdac->vpll_m << ramdac->vpll_p));
            nv_log("Video clock is now %d MHz\n", (float)(ramdac->vpll_hz) / 1000.0f);
            break; 
        case SGS_DAC_MPLL_P:
            ramdac->mpll_hz = ((STG1732_BASE_CLOCK * ramdac->mpll_n) * (ramdac->mpll_m << ramdac->mpll_p));
            nv_log("Core & Memory clock is now %d MHz\n", (float)(ramdac->mpll_hz) / 1000.0f);
            break; 
    }
}

uint8_t
stg1732_ramdac_reg_read(uint16_t addr, void *priv, svga_t *svga)
{
    stg1732_ramdac_t *ramdac = (stg1732_ramdac_t *) priv;
    uint8_t       ret   = 0xff;

    // considering how the nv1 works idk if we need to actually sync the clocks 

    switch (addr)
    {
        case SGS_DAC_VENDOR_ID:
            ret = SGS_DAC_VENDOR_ID_SGS;
            break; 
        case SGS_DAC_DEVICE_ID:
            if (ramdac->type == DAC_IS_STG1764)
                ret = SGS_DAC_DEVICE_ID_VAN_DYKE;
            else
                ret = SGS_DAC_DEVICE_ID_VAN_GOGH;
            break;
        case SGS_DAC_CONFIG_0:
            ret = ramdac->config_0;

            if (ramdac->type == DAC_IS_STG1764)
                ret |= (SGS_DAC_CONFIG_0_PORT_WIDTH_64BIT << SGS_DAC_CONFIG_0_PORT_WIDTH);
            else
                ret |= (SGS_DAC_CONFIG_0_PORT_WIDTH_32BIT << SGS_DAC_CONFIG_0_PORT_WIDTH);

            break;
        case SGS_DAC_CONFIG_1:
            ret = ramdac->config_1;
            break;
        case SGS_DAC_RGB_EXT_PAL_CTRL:
            ret = ramdac->palette_control;
            break;
        // compressed to not waste tons of lines on trivial code
        case SGS_DAC_APLL_M: ret = ramdac->apll_m; break;
        case SGS_DAC_APLL_N: ret = ramdac->apll_n; break;
        case SGS_DAC_APLL_O: ret = ramdac->apll_o; break;
        case SGS_DAC_APLL_P: ret = ramdac->apll_p; break; 
        case SGS_DAC_MPLL_M: ret = ramdac->mpll_m; break;
        case SGS_DAC_MPLL_N: ret = ramdac->mpll_n; break;
        case SGS_DAC_MPLL_O: ret = ramdac->mpll_o; break;
        case SGS_DAC_MPLL_P: ret = ramdac->mpll_p; break; 
        case SGS_DAC_VPLL_M: ret = ramdac->vpll_m; break;
        case SGS_DAC_VPLL_N: ret = ramdac->vpll_n; break;
        case SGS_DAC_VPLL_O: ret = ramdac->vpll_o; break;
        case SGS_DAC_VPLL_P: ret = ramdac->vpll_p; break;
    }
    
    nv_log("STG1764 register read 0x%02x from 0x%02x\n", ret, addr);
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

            // not strictly needed but helpful while debugging
            if (ramdac->type == DAC_IS_STG1764)
                ramdac->config_0 |= (SGS_DAC_CONFIG_0_PORT_WIDTH_64BIT << SGS_DAC_CONFIG_0_PORT_WIDTH);
            else
            {
                ramdac->config_0 |= (SGS_DAC_CONFIG_0_PORT_WIDTH_32BIT << SGS_DAC_CONFIG_0_PORT_WIDTH);
                // STG1732 has vesa connector
                ramdac->config_0 |= (SGS_DAC_CONFIG_0_VESA_CONNECTOR_ENABLED << SGS_DAC_CONFIG_0_VESA_CONNECTOR);
            }
                
            break;
        case SGS_DAC_CONFIG_1:
            ramdac->config_1 = val;
            break;
        case SGS_DAC_RGB_EXT_PAL_CTRL:
            ramdac->palette_control = val; 
            break;
        // compressed to not waste tons of lines on trivial code
        case SGS_DAC_APLL_M: ramdac->apll_m = val; break;
        case SGS_DAC_APLL_N: ramdac->apll_n = val; break;
        case SGS_DAC_APLL_O: ramdac->apll_o = val; break;
        case SGS_DAC_APLL_P: 
            ramdac->apll_p = val; 
            stg1732_set_clock_speed(addr, priv);
            break;
        case SGS_DAC_MPLL_M: ramdac->mpll_m = val; break;
        case SGS_DAC_MPLL_N: ramdac->mpll_n = val; break;
        case SGS_DAC_MPLL_O: ramdac->mpll_o = val; break;
        case SGS_DAC_MPLL_P: 
            ramdac->mpll_p = val; 
            stg1732_set_clock_speed(addr, priv);
            break;
        case SGS_DAC_VPLL_M: ramdac->vpll_m = val; break;
        case SGS_DAC_VPLL_N: ramdac->vpll_n = val; break;
        case SGS_DAC_VPLL_O: ramdac->vpll_o = val; break;
        case SGS_DAC_VPLL_P:
            ramdac->vpll_p = val; 
            stg1732_set_clock_speed(addr, priv);
            break;
    }

    if (addr >= SGS_DAC_MPLL_M && addr <= SGS_DAC_VPLL_P)
        svga_recalctimings(svga);
        
    nv_log("STG1764 register write 0x%02x to 0x%02x\n", val, addr);
}
static void *
stg1732_ramdac_init(UNUSED(const device_t *info))
{
    stg1732_ramdac_t *ramdac = (stg1732_ramdac_t *) calloc(1, sizeof(stg1732_ramdac_t));
    ramdac->type = info->local & 0xff;

    /* 
        NVidia default PLL values for 50MHz card
        Algo is same as riva 128
    */  
    ramdac->mpll_m = 11;
    ramdac->mpll_n = 91;
    ramdac->mpll_o = 1;
    ramdac->mpll_p = 1;
                // not strictly needed but helpful while debugging

    // hardcoded stuff
    if (ramdac->type == DAC_IS_STG1764)
        ramdac->config_0 |= (SGS_DAC_CONFIG_0_PORT_WIDTH_64BIT << SGS_DAC_CONFIG_0_PORT_WIDTH);
    else
    {
        ramdac->config_0 |= (SGS_DAC_CONFIG_0_PORT_WIDTH_32BIT << SGS_DAC_CONFIG_0_PORT_WIDTH);
        // STG1732 has vesa connector
        ramdac->config_0 |= (SGS_DAC_CONFIG_0_VESA_CONNECTOR_ENABLED << SGS_DAC_CONFIG_0_VESA_CONNECTOR);
    }
                
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
    .local         = DAC_IS_STG1764,
    .init          = stg1732_ramdac_init,
    .close         = stg1732_ramdac_close,
    .reset         = NULL,
    .available     = NULL,
    .speed_changed = NULL,
    .force_redraw  = NULL,
    .config        = NULL
};
