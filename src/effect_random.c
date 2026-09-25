#include <stdlib.h>
#include "effect.h"
#include "util.h"

/* 状态 */
static float g_cur_hue = 0.0f;
static float g_from_hue = 0.0f;
static float g_to_hue = 0.0f;
static long  g_last_change = 0;
static long  g_fade_start = 0;
static int   g_fading = 0;

static float smoothstep(float t) {
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    return t * t * (3.0f - 2.0f * t);
}

/* 色相差值：处理 0.9 -> 0.1 的绕圈 */
static float lerp_hue(float a, float b, float t) {
    float d = b - a;
    if (d > 0.5f)  d -= 1.0f;
    if (d < -0.5f) d += 1.0f;
    float r = a + d * t;
    if (r < 0) r += 1.0f;
    if (r >= 1.0f) r -= 1.0f;
    return r;
}

void eff_random_enter(void) {
    g_cur_hue = (float)(rand() % 1000) / 1000.0f;
    g_from_hue = g_cur_hue;
    g_to_hue = g_cur_hue;
    g_last_change = 0;
    g_fade_start = 0;
    g_fading = 0;

    int r, g, b;
    hsv2rgb(g_cur_hue, 0.9f, 0.9f, &r, &g, &b);
    effect_output(r, g, b);
}

void eff_random_tick(long now) {
    EffectParams *p = effect_get_params();
    int interval = p->interval > 0 ? p->interval : 1000;
    int trans    = p->transition_ms;
    float sat    = p->saturation;

    /* 首次 */
    if (g_last_change == 0) {
        g_last_change = now;
        return;
    }

    /* 到达间隔，选新色相，开始渐变 */
    if (!g_fading && (now - g_last_change >= interval)) {
        g_last_change = now;
        g_from_hue = g_cur_hue;
        g_to_hue = (float)(rand() % 1000) / 1000.0f;
        g_fade_start = now;
        if (trans > 0) {
            g_fading = 1;
        } else {
            g_cur_hue = g_to_hue;
            g_fading = 0;
        }
    }

    /* 渐变中 */
    if (g_fading) {
        long elapsed = now - g_fade_start;
        if (elapsed >= trans) {
            g_cur_hue = g_to_hue;
            g_fading = 0;
        } else {
            float t = (float)elapsed / (float)trans;
            t = smoothstep(t);
            g_cur_hue = lerp_hue(g_from_hue, g_to_hue, t);
        }
    }

    int r, g, b;
    hsv2rgb(g_cur_hue, sat, 0.9f, &r, &g, &b);
    effect_output(r, g, b);
}