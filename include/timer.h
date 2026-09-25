#ifndef TIMER_H
#define TIMER_H

int  timer_init(int interval_ms);
int  timer_get_fd(void);
void timer_start(void);
void timer_stop(void);
void timer_drain(void);

#endif /* TIMER_H */