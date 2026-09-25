#include "effect.h"
#include "util.h"

static float g_hue = 0.0f;
static long  g_last = 0;

void eff_rainbow_enter(void) {
    g_hue = 0.0f;
    g_last = 0;
}

void eff_rainbow_tick(long now) {
    EffectParams *p = effect_get_params();
    int step = p->step_ms > 0 ? p->step_ms : 200;
    float sat = p->saturation;
    float val = p->value;

    if (g_last == 0) g_last = now;

    /* 按时间累积色相，与帧率解耦 */
    while (now - g_last >= step) {
        g_last += step;
        g_hue += 0.01f;
        if (g_hue >= 1.0f) g_hue -= 1.0f;
    }

    int r, g, b;
    hsv2rgb(g_hue, sat, val, &r, &g, &b);
    effect_output(r, g, b);
}