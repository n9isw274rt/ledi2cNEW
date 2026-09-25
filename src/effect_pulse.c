#include "effect.h"

void eff_pulse_enter(void) {
    /* 无状态 */
}

void eff_pulse_tick(long now) {
    EffectParams *p = effect_get_params();
    int period = p->period > 0 ? p->period : 1000;
    int bmin = p->bmin;
    int bmax = p->bmax;
    int span = bmax - bmin;

    double phase = (double)(now % period) / (double)period;
    double s = phase * phase;   /* 二次缓动 */

    int scale = bmin + (int)(span * s);

    int r = effect_get_target_R() * scale / 255;
    int g = effect_get_target_G() * scale / 255;
    int b = effect_get_target_B() * scale / 255;
    effect_output(r, g, b);
}