#ifndef LOG_H
#define LOG_H

#include "ledd.h"

void log_init(LogLevel level);

void log_printf(LogLevel level, const char *module, const char *fmt, ...);

#define LOGD(mod, ...)  log_printf(LOG_DEBUG, mod, __VA_ARGS__)
#define LOGI(mod, ...)  log_printf(LOG_INFO,  mod, __VA_ARGS__)
#define LOGW(mod, ...)  log_printf(LOG_WARN,  mod, __VA_ARGS__)
#define LOGE(mod, ...)  log_printf(LOG_ERROR, mod, __VA_ARGS__)

#endif /* LOG_H */