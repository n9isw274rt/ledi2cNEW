#include "source.h"
#include "effect.h"
#include "log.h"

static DataSource g_src = SRC_MIC;

void source_init(void) {
    g_src = SRC_MIC;
}

void source_set(DataSource src) {
    if (src != SRC_MIC && src != SRC_SYSTEM) {
        LOGW("source", "invalid src=%d", (int)src);
        return;
    }
    if (g_src == src) return;
    g_src = src;
    LOGI("source", "set to %s", src == SRC_MIC ? "mic" : "system");

    /* source 变化会影响 timerfd */
    effect_refresh_timer();
}

DataSource source_get(void) {
    return g_src;
}