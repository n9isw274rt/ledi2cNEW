#include "effect.h"

void eff_strobe_enter(void) {
    /* 无状态 */
}

void eff_strobe_tick(long now) {
    EffectParams *p = effect_get_params();
    int on  = p->on_ms  > 0 ? p->on_ms  : 100;
    int off = p->off_ms > 0 ? p->off_ms : 200;
    int edge = p->edge_ms >= 0 ? p->edge_ms : 20;

    long cyc = on + off;
    long t = now % cyc;

    float v;
    if (t < on) {
        /* 亮：edge 上升，再保持 */
        if (edge > 0 && t < edge) {
            v = (float)t / (float)edge;
        } else {
            v = 1.0f;
        }
    } else {
        /* 灭：edge 下降，然后 0 */
        long toff = t - on;
        if (edge > 0 && toff < edge) {
            v = 1.0f - (float)toff / (float)edge;
        } else {
            v = 0.0f;
        }
    }

    int r = (int)(effect_get_target_R() * v);
    int g = (int)(effect_get_target_G() * v);
    int b = (int)(effect_get_target_B() * v);
    effect_output(r, g, b);
}