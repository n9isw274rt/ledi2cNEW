#include "effect.h"
#include "util.h"

static int  from_R, from_G, from_B;
static int  to_R,   to_G,   to_B;
static long start_ms;
static int  active;

static float smoothstep(float t) {
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    return t * t * (3.0f - 2.0f * t);
}

void eff_fade_enter(void) {
    int cr, cg, cb;
    effect_get_current(&cr, &cg, &cb);
    from_R = cr; from_G = cg; from_B = cb;
    to_R = effect_get_target_R();
    to_G = effect_get_target_G();
    to_B = effect_get_target_B();
    start_ms = now_ms();
    active = 1;
}

void eff_fade_tick(long now) {
    if (!active) return;
    int dur = effect_get_params()->fade_dur;
    long elapsed = now - start_ms;
    if (elapsed >= dur) {
        effect_output(to_R, to_G, to_B);
        active = 0;
        effect_finish();   /* 切回 STATIC，停 timerfd */
        return;
    }
    float t = (float)elapsed / (float)dur;
    t = smoothstep(t);
    int r = from_R + (int)((to_R - from_R) * t);
    int g = from_G + (int)((to_G - from_G) * t);
    int b = from_B + (int)((to_B - from_B) * t);
    effect_output(r, g, b);
}