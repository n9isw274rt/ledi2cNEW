#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "command.h"
#include "effect.h"
#include "source.h"
#include "audio.h"
#include "i2c.h"
#include "util.h"
#include "log.h"
#include "ledd.h"

static int cmd_static(const char *args) {
    int r, g, b;
    if (sscanf(args, "%d:%d:%d", &r, &g, &b) != 3) return -1;
    r = clampi(r,0,255); g = clampi(g,0,255); b = clampi(b,0,255);
    effect_set_target(r, g, b);
    effect_set_mode(E_STATIC);
    return 0;
}

static int cmd_fade(const char *args) {
    int r, g, b, bright = 0, ms = 500;
    int n = sscanf(args, "%d:%d:%d:%d:%d", &r, &g, &b, &bright, &ms);
    if (n < 3) return -1;
    r = clampi(r,0,255); g = clampi(g,0,255); b = clampi(b,0,255);
    effect_set_target(r, g, b);
    if (n >= 5) effect_set_fade_dur(ms);
    effect_set_mode(E_FADE);
    return 0;
}

static int cmd_breath(const char *args) {
    int r, g, b, period = 2000, bmin = 0, bmax = 255;
    int n = sscanf(args, "%d:%d:%d:%d:%d:%d", &r, &g, &b, &period, &bmin, &bmax);
    if (n < 3) return -1;
    r = clampi(r,0,255); g = clampi(g,0,255); b = clampi(b,0,255);
    effect_set_target(r, g, b);
    effect_set_breath(period, bmin, bmax);
    effect_set_mode(E_BREATH);
    return 0;
}

static int cmd_strobe(const char *args) {
    int r, g, b, on = 100, off = 200, edge = 20;
    int n = sscanf(args, "%d:%d:%d:%d:%d:%d", &r, &g, &b, &on, &off, &edge);
    if (n < 3) return -1;
    r = clampi(r,0,255); g = clampi(g,0,255); b = clampi(b,0,255);
    effect_set_target(r, g, b);
    effect_set_strobe(on, off, edge);
    effect_set_mode(E_STROBE);
    return 0;
}

static int cmd_rainbow(const char *args) {
    int step = 200;
    float s = 0.9f, v = 0.9f;
    int n = sscanf(args, "%d:%f:%f", &step, &s, &v);
    if (n >= 1) effect_set_rainbow(step, s, v);
    effect_set_mode(E_RAINBOW);
    return 0;
}

static int cmd_pulse(const char *args) {
    int r, g, b, period = 1000, bmin = 0, bmax = 255;
    int n = sscanf(args, "%d:%d:%d:%d:%d:%d", &r, &g, &b, &period, &bmin, &bmax);
    if (n < 3) return -1;
    r = clampi(r,0,255); g = clampi(g,0,255); b = clampi(b,0,255);
    effect_set_target(r, g, b);
    effect_set_pulse(period, bmin, bmax);
    effect_set_mode(E_PULSE);
    return 0;
}

static int cmd_random(const char *args) {
    int interval = 1000, trans = 300;
    float s = 0.9f;
    int n = sscanf(args, "%d:%d:%f", &interval, &trans, &s);
    if (n >= 1) effect_set_random(interval, trans, s);
    effect_set_mode(E_RANDOM);
    return 0;
}

/* OFF：独立，关灯，不影响 source */
static int cmd_off(const char *args) {
    (void)args;
    effect_set_mode(E_OFF);
    return 0;
}

/* SOURCE:mic|system */
static int cmd_source(const char *args) {
    if (strcasecmp(args, "mic") == 0) {
        source_set(SRC_MIC);
    } else if (strcasecmp(args, "system") == 0) {
        source_set(SRC_SYSTEM);
    } else {
        return -1;
    }
    return 0;
}

static int cmd_audio(const char *args) {
    audio_handle(args);
    return 0;
}

static int cmd_status(const char *args) {
    (void)args;
    int r, g, b;
    effect_get_current(&r, &g, &b);
    LOGI("cmd", "STATUS source=%s effect=%s color=%d,%d,%d",
         source_get() == SRC_MIC ? "mic" : "system",
         effect_get_mode_name(), r, g, b);
    return 0;
}

static int cmd_version(const char *args) {
    (void)args;
    LOGI("cmd", "VERSION %s", LEDD_VERSION);
    return 0;
}

typedef struct {
    const char *name;
    int (*handler)(const char *args);
} CmdEntry;

static const CmdEntry g_cmds[] = {
    { "STATIC",  cmd_static  },
    { "RGB",     cmd_static  },
    { "FADE",    cmd_fade    },
    { "BREATH",  cmd_breath  },
    { "STROBE",  cmd_strobe  },
    { "RAINBOW", cmd_rainbow },
    { "PULSE",   cmd_pulse   },
    { "RANDOM",  cmd_random  },
    { "OFF",     cmd_off     },
    { "SOURCE",  cmd_source  },
    { "AUDIO",   cmd_audio   },
    { "STATUS",  cmd_status  },
    { "VERSION", cmd_version },
};

static const int g_cmd_count = sizeof(g_cmds) / sizeof(g_cmds[0]);

void command_init(void) {
    LOGI("cmd", "init ok, %d commands", g_cmd_count);
}

void command_handle(const char *line) {
    if (!line || !*line) return;

    const char *colon = strchr(line, ':');
    char name[32];
    const char *args = "";

    if (colon) {
        int len = colon - line;
        if (len >= (int)sizeof(name)) len = sizeof(name) - 1;
        memcpy(name, line, len);
        name[len] = 0;
        args = colon + 1;
    } else {
        strncpy(name, line, sizeof(name) - 1);
        name[sizeof(name) - 1] = 0;
    }

    for (int i = 0; i < g_cmd_count; i++) {
        if (strcasecmp(name, g_cmds[i].name) == 0) {
            int ret = g_cmds[i].handler(args);
            if (ret != 0) LOGW("cmd", "cmd %s failed", name);
            return;
        }
    }

    LOGW("cmd", "unknown cmd: %s", name);
}