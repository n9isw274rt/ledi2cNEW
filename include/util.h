#ifndef UTIL_H
#define UTIL_H

long  now_ms(void);
int   clampi(int v, int lo, int hi);
float clampf(float v, float lo, float hi);
void  hsv2rgb(float h, float s, float v, int *r, int *g, int *b);

#endif /* UTIL_H */