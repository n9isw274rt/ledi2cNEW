#ifndef EFFECT_H
#define EFFECT_H

#include "ledd.h"

typedef struct {
    const char *name;
    int         need_timer;
    void (*enter)(void);
    void (*tick)(long now);
    void (*exit)(void);
} EffectOps;

typedef struct {
    int   period;
    int   bmin;
    int   bmax;
    int   on_ms;
    int   off_ms;
    int   edge_ms;
    int   step_ms;
    int   interval;
    int   transition_ms;
    int   fade_dur;
    float saturation;
    float value;
} EffectParams;

EffectParams *effect_get_params(void);

void eff_static_enter(void);
void eff_static_tick(long now);
void eff_fade_enter(void);
void eff_fade_tick(long now);
void eff_breath_enter(void);
void eff_breath_tick(long now);
void eff_strobe_enter(void);
void eff_strobe_tick(long now);
void eff_rainbow_enter(void);
void eff_rainbow_tick(long now);
void eff_pulse_enter(void);
void eff_pulse_tick(long now);
void eff_random_enter(void);
void eff_random_tick(long now);

void effect_init(void);
void effect_set_mode(EffectMode mode);
void effect_tick(long now);
void effect_refresh_timer(void);

void effect_set_color(int r, int g, int b);

void effect_set_fade_dur(int ms);
void effect_set_breath(int period, int bmin, int bmax);
void effect_set_strobe(int on, int off, int edge);
void effect_set_rainbow(int step_ms, float s, float v);
void effect_set_pulse(int period, int bmin, int bmax);
void effect_set_random(int interval, int trans_ms, float s);

void effect_get_current(int *r, int *g, int *b);
const char *effect_get_mode_name(void);

void effect_output(int r, int g, int b);
int  effect_get_target_R(void);
int  effect_get_target_G(void);
int  effect_get_target_B(void);
void effect_set_target(int r, int g, int b);

#endif