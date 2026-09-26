#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <errno.h>

#include "timer.h"
#include "log.h"

static int  g_tfd = -1;
static int  g_interval_ms = 20;

int timer_init(int interval_ms) {
    g_interval_ms = interval_ms > 0 ? interval_ms : 20;

    g_tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (g_tfd < 0) {
        LOGE("timer", "timerfd_create fail: %s", strerror(errno));
        return -1;
    }

    LOGI("timer", "init ok, interval=%dms", g_interval_ms);
    return 0;
}

int timer_get_fd(void) {
    return g_tfd;
}

void timer_start(void) {
    if (g_tfd < 0) return;

    struct itimerspec its;
    memset(&its, 0, sizeof(its));

    long ns = (long)g_interval_ms * 1000000L;
    its.it_interval.tv_sec  = 0;
    its.it_interval.tv_nsec = ns;
    its.it_value.tv_sec     = 0;
    its.it_value.tv_nsec    = ns;   /* 相对：20ms 后首次触发 */

    /* 相对模式：it_value 是"时长"，不是绝对时间点 */
    if (timerfd_settime(g_tfd, 0, &its, NULL) < 0) {
        LOGE("timer", "start fail: %s", strerror(errno));
        return;
    }
}

void timer_stop(void) {
    if (g_tfd < 0) return;

    struct itimerspec its;
    memset(&its, 0, sizeof(its));
    if (timerfd_settime(g_tfd, 0, &its, NULL) < 0) {
        LOGE("timer", "stop fail: %s", strerror(errno));
        return;
    }
}

void timer_drain(void) {
    if (g_tfd < 0) return;
    unsigned long long exp = 0;
    ssize_t n = read(g_tfd, &exp, sizeof(exp));
    (void)n;
}
