#include <time.h>
#include <math.h>
#include "util.h"

long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void hsv2rgb(float h, float s, float v, int *r, int *g, int *b) {
    h = h - (int)h;
    if (h < 0) h += 1.0f;
    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h * 6.0f, 2.0f) - 1));
    float m = v - c;
    float rr = 0, gg = 0, bb = 0;
    int seg = (int)(h * 6.0f);
    switch (seg) {
        case 0: rr = c; gg = x; bb = 0; break;
        case 1: rr = x; gg = c; bb = 0; break;
        case 2: rr = 0; gg = c; bb = x; break;
        case 3: rr = 0; gg = x; bb = c; break;
        case 4: rr = x; gg = 0; bb = c; break;
        case 5: rr = c; gg = 0; bb = x; break;
    }
    *r = (int)((rr + m) * 255);
    *g = (int)((gg + m) * 255);
    *b = (int)((bb + m) * 255);
}