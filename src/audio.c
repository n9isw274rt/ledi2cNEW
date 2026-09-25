#include <stdio.h>
#include "audio.h"
#include "effect.h"

void audio_handle(const char *args) {
    int r, g, b;
    if (sscanf(args, "%d,%d,%d", &r, &g, &b) == 3) {
        effect_output(r, g, b);
    }
}