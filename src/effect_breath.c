#include "effect.h"
#include "util.h"

static float smoothstep(float t) {
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    return t * t * (3.0f - 2.0f * t);
}

void eff_breath_enter(void) {
    /* 无状态初始化 */
}

void eff_breath_tick(long now) {
    EffectParams *p = effect_get_params();
    int period = p->period > 0 ? p->period : 2000;

    double phase = (double)(now % period) / (double)period;
    double s;
    if (phase < 0.4) {
        s = smoothstep((float)(phase / 0.4));
    } else {
        s = 1.0 - smoothstep((float)((phase - 0.4) / 0.6));
    }

    int bmin = p->bmin;
    int bmax = p->bmax;
    int span = bmax - bmin;
    int scale = bmin + (int)(span * s);

    int r = effect_get_target_R() * scale / 255;
    int g = effect_get_target_G() * scale / 255;
    int b = effect_get_target_B() * scale / 255;

    effect_output(r, g, b);
}