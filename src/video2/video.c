#include <86box/video2/video.h>

video_monitor_t monitors[MAX_MONITORS] = {0};
uint32_t num_monitors;

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

}

video_monitor_t* Video_AddMonitor(video_monitor_settings_t settings)
{

}

void Video_SetMonitorSize(uint32_t size_x, uint32_t size_y)
{

}

void Video_SetBPP(uint32_t bpp)
{

}

void Video_SetRefreshRate(uint32_t hz)
{

}

void Video_DestroyMonitor(video_monitor_t* monitor)
{

}