#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include "log.h"

static LogLevel g_level = LOG_INFO;

void log_init(LogLevel level) {
    g_level = level;
}

static const char *level_name(LogLevel lv) {
    switch (lv) {
        case LOG_DEBUG: return "D";
        case LOG_INFO:  return "I";
        case LOG_WARN:  return "W";
        case LOG_ERROR: return "E";
        default:        return "?";
    }
}

void log_printf(LogLevel level, const char *module, const char *fmt, ...) {
    if (level < g_level) return;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);

    fprintf(stderr, "[%02d:%02d:%02d.%03d][%s][%s] ",
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            (int)(tv.tv_usec / 1000),
            level_name(level),
            module ? module : "-");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, "\n");
}