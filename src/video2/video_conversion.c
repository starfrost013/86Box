#include <86box/video2/video.h>

int32_t Video_Calc6to8(int32_t c)
{
    int32_t ic;
    int32_t i8;
    double  d8;

    ic = c;
    if (ic == 64)
        ic = 63;
    else
        ic &= 0x3f;
    d8 = (ic / 63.0) * 255.0;
    i8 = (int) d8;

    return (i8 & 0xff);
}

int32_t Video_Calc8to32(int32_t c)
{
    int32_t b;
    int32_t g;
    int32_t r;
    double  db;
    double  dg;
    double  dr;

    b  = (c & 3);
    g  = ((c >> 2) & 7);
    r  = ((c >> 5) & 7);
    db = (((double) b) / 3.0) * 255.0;
    dg = (((double) g) / 7.0) * 255.0;
    dr = (((double) r) / 7.0) * 255.0;
    b  = (int) db;
    g  = ((int) dg) << 8;
    r  = ((int) dr) << 16;

    return (b | g | r);
}

int32_t Video_Calc15to32(int32_t c)
{
    int32_t b;
    int32_t g;
    int32_t r;
    double  db;
    double  dg;
    double  dr;

    b  = (c & 31);
    g  = ((c >> 5) & 31);
    r  = ((c >> 10) & 31);
    db = (((double) b) / 31.0) * 255.0;
    dg = (((double) g) / 31.0) * 255.0;
    dr = (((double) r) / 31.0) * 255.0;
    b  = (int) db;
    g  = ((int) dg) << 8;
    r  = ((int) dr) << 16;

    return (b | g | r);
}

int32_t Video_Calc16to32(int32_t c)
{
    int32_t b;
    int32_t g;
    int32_t r;
    double  db;
    double  dg;
    double  dr;

    b  = (c & 31);
    g  = ((c >> 5) & 63);
    r  = ((c >> 11) & 31);
    db = (((double) b) / 31.0) * 255.0;
    dg = (((double) g) / 63.0) * 255.0;
    dr = (((double) r) / 31.0) * 255.0;
    b  = (int) db;
    g  = ((int) dg) << 8;
    r  = ((int) dr) << 16;

    return (b | g | r);
}