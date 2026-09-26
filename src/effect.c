#include <string.h>
#include "effect.h"
#include "i2c.h"
#include "timer.h"
#include "source.h"
#include "log.h"

static EffectMode   g_mode = E_OFF;
static EffectParams g_params;
static int g_R = 0, g_G = 0, g_B = 0;
static int g_cur_R = 0, g_cur_G = 0, g_cur_B = 0;

static const EffectOps g_effects[] = {
    [E_OFF]     = { "off",     0, NULL,              NULL,              NULL },
    [E_STATIC]  = { "static",  0, eff_static_enter,  eff_static_tick,   NULL },
    [E_FADE]    = { "fade",    1, eff_fade_enter,    eff_fade_tick,     NULL },
    [E_BREATH]  = { "breath",  1, eff_breath_enter,  eff_breath_tick,   NULL },
    [E_STROBE]  = { "strobe",  1, eff_strobe_enter,  eff_strobe_tick,   NULL },
    [E_RAINBOW] = { "rainbow", 1, eff_rainbow_enter, eff_rainbow_tick,  NULL },
    [E_PULSE]   = { "pulse",   1, eff_pulse_enter,   eff_pulse_tick,    NULL },
    [E_RANDOM]  = { "random",  1, eff_random_enter,  eff_random_tick,   NULL },
};

static void params_default(void) {
    g_params.period        = 2000;
    g_params.bmin          = 0;
    g_params.bmax          = 255;
    g_params.on_ms         = 100;
    g_params.off_ms        = 200;
    g_params.edge_ms       = 20;
    g_params.step_ms       = 200;
    g_params.interval      = 1000;
    g_params.transition_ms = 300;
    g_params.fade_dur      = 500;
    g_params.saturation    = 0.9f;
    g_params.value         = 0.9f;
}

EffectParams *effect_get_params(void) { return &g_params; }

void effect_output(int r, int g, int b) {
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;
    g_cur_R = r; g_cur_G = g; g_cur_B = b;
    i2c_write_color(r, g, b);
}

int  effect_get_target_R(void) { return g_R; }
int  effect_get_target_G(void) { return g_G; }
int  effect_get_target_B(void) { return g_B; }
void effect_set_target(int r, int g, int b) { g_R = r; g_G = g; g_B = b; }

/* timerfd 跑的条件 = source==MIC && effect need_timer */
void effect_refresh_timer(void) {
    if (source_get() != SRC_MIC) {
        timer_stop();
        return;
    }
    if (g_mode >= 0 && g_mode < E_COUNT && g_effects[g_mode].need_timer) {
        timer_start();
    } else {
        timer_stop();
    }
}

void effect_init(void) {
    params_default();
    g_mode = E_OFF;
    g_R = g_G = g_B = 0;
    g_cur_R = g_cur_G = g_cur_B = 0;
    LOGI("effect", "init ok");
}

void effect_set_mode(EffectMode mode) {
    if (mode < 0 || mode >= E_COUNT) return;
    if (g_mode >= 0 && g_mode < E_COUNT) {
        if (g_effects[g_mode].exit) g_effects[g_mode].exit();
    }
    g_mode = mode;
    if (g_effects[g_mode].enter) g_effects[g_mode].enter();
    effect_refresh_timer();
    LOGI("effect", "mode -> %s", g_effects[g_mode].name);
}

void effect_tick(long now) {
    if (g_mode < 0 || g_mode >= E_COUNT) return;
    if (g_effects[g_mode].tick) g_effects[g_mode].tick(now);
}


void effect_set_fade_dur(int ms) { if (ms >= 10 && ms <= 10000) g_params.fade_dur = ms; }
void effect_set_breath(int period, int bmin, int bmax) {
    if (period >= 200 && period <= 10000) g_params.period = period;
    if (bmin >= 0 && bmin <= 255) g_params.bmin = bmin;
    if (bmax >= 0 && bmax <= 255) g_params.bmax = bmax;
}
void effect_set_strobe(int on, int off, int edge) {
    if (on >= 10 && on <= 2000)   g_params.on_ms = on;
    if (off >= 10 && off <= 2000) g_params.off_ms = off;
    if (edge >= 0 && edge <= 200) g_params.edge_ms = edge;
}
void effect_set_rainbow(int step_ms, float s, float v) {
    if (step_ms >= 10 && step_ms <= 2000) g_params.step_ms = step_ms;
    if (s >= 0 && s <= 1) g_params.saturation = s;
    if (v >= 0 && v <= 1) g_params.value = v;
}
void effect_set_pulse(int period, int bmin, int bmax) {
    if (period >= 100 && period <= 10000) g_params.period = period;
    if (bmin >= 0 && bmin <= 255) g_params.bmin = bmin;
    if (bmax >= 0 && bmax <= 255) g_params.bmax = bmax;
}
void effect_set_random(int interval, int trans_ms, float s) {
    if (interval >= 100 && interval <= 10000) g_params.interval = interval;
    if (trans_ms >= 0 && trans_ms <= 2000)    g_params.transition_ms = trans_ms;
    if (s >= 0 && s <= 1) g_params.saturation = s;
}

void effect_get_current(int *r, int *g, int *b) {
    if (r) *r = g_cur_R;
    if (g) *g = g_cur_G;
    if (b) *b = g_cur_B;
}

/* FADE 等一次性效果完成后调用：切回 STATIC，停 timerfd */
void effect_finish(void) {
    if (g_mode != E_STATIC && g_mode != E_OFF) {
        effect_set_mode(E_STATIC);
    }
}
const char *effect_get_mode_name(void) {
    if (g_mode < 0 || g_mode >= E_COUNT) return "?";
    return g_effects[g_mode].name;
}