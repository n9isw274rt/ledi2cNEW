#include "effect.h"
#include "i2c.h"

void eff_static_enter(void) {
    /* 进入静态时直接写一次，之后 timerfd 停，不再刷 */
    int r = effect_get_target_R();
    int g = effect_get_target_G();
    int b = effect_get_target_B();
    effect_output(r, g, b);
}

void eff_static_tick(long now) {
    /* timerfd 停掉后不会调用；保留以防万一 */
    (void)now;
}