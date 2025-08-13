#include <86box/video2/video.h>


// Defines only used in this translation unit

#define VIDEO_TRANSLATION_TABLE_6TO8_SIZE       4 * 256         // Size of the 6-to-8bpp conversion table.
#define VIDEO_TRANSLATION_TABLE_8TOGS_SIZE      4 * 256         // Size of the 8 to RGB528 conversion table.
#define VIDEO_TRANSLATION_TABLE_8TO32_SIZE      4 * 256         // Size of the 8-to-32bpp conversion table.
#define VIDEO_TRANSLATION_TABLE_15TO32_SIZE     4 * 65536       // Size of the 15-to-32bpp conversion table.
#define VIDEO_TRANSLATION_TABLE_16TO32_SIZE     4 * 65536       // Size of the 16-to-32bpp conversion table.

// Globals
video_monitor_t video_monitors[MAX_MONITORS] = {0};
video_device_t video_devices[MAX_DEVICES] = {0};
uint32_t num_monitors;
uint32_t num_devices;

uint32_t* conversion_table_6to8; 
uint32_t* conversion_table_8togs; 
uint32_t* conversion_table_8to32; 
uint32_t* conversion_table_15to32; 
uint32_t* conversion_table_16to32; 

#ifdef ENABLE_VIDEO_LOG
int video_do_log = ENABLE_VIDEO_LOG;

static void
Video_Log(const char *fmt, ...)
{
    va_list ap;

    if (video_do_log) {
        va_start(ap, fmt);
        pclog_ex(fmt, ap);
        va_end(ap);
    }
}
#else
#    define Video_Log(fmt, ...)
#endif


// Functions
void Video_Init()
{
    // Is this the best approach?
    conversion_table_6to8 = (uint32_t*)calloc(1, VIDEO_TRANSLATION_TABLE_6TO8_SIZE);
    conversion_table_8togs = (uint32_t*)calloc(1, VIDEO_TRANSLATION_TABLE_8TOGS_SIZE);
    conversion_table_8to32 = (uint32_t*)calloc(1, VIDEO_TRANSLATION_TABLE_8TO32_SIZE);
    conversion_table_15to32 = (uint32_t*)calloc(1, VIDEO_TRANSLATION_TABLE_15TO32_SIZE);
    conversion_table_16to32 = (uint32_t*)calloc(1, VIDEO_TRANSLATION_TABLE_16TO32_SIZE);
}

void Video_AddDevice(device_t* video)
{
    if (num_devices >= MAX_DEVICES)
        return;

    video_devices[num_devices].device = video;
    num_devices++;
}

video_monitor_t* Video_AddMonitor(video_monitor_settings_t settings)
{
    if (num_monitors >= MAX_MONITORS)
        return;

    video_monitors[num_monitors].settings = settings;
 
    num_monitors++;

}


void Video_SetMonitorSize(video_monitor_t* monitor, uint32_t size_x, uint32_t size_y)
{
    if (!monitor)
        return; 

    monitor->settings.size_x = size_x;
    monitor->settings.size_y = size_y;
}

void Video_SetBPP(video_monitor_t* monitor, uint32_t bpp)
{
    if (!monitor)
        return; 

    monitor->settings.bpp;
}

void Video_SetRefreshRate(video_monitor_t* monitor, uint32_t hz)
{
    if (!monitor)
        return; 

    monitor->settings.hz = hz;
}

uint32_t Video_DefaultConvert8to32(uint8_t colour)
{

}

uint32_t Video_DefaultConvert15to32(uint16_t colour)
{

}

uint32_t Video_DefaultConvert16to32(uint16_t colour)
{
    
}

void Video_DestroyMonitor(video_monitor_t* monitor)
{

}

void Video_Blit_MemToScreen(video_blit_info_t blit_info)
{
    
}

void Video_Blit_MemToMem(mem2mem_blit_info_t blit_info)
{

}

void Video_Reset()
{

}

void Video_Shutdown()
{

}