#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>

#include "ledd.h"
#include "log.h"
#include "util.h"
#include "i2c.h"
#include "timer.h"
#include "socket.h"
#include "source.h"
#include "effect.h"
#include "command.h"

#define MAX_EVENTS  16

static int g_epfd = -1;
static int g_sfd  = -1;   /* signalfd */

static int setup_signalfd(void) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    /* 必须先阻塞，否则信号不会被 signalfd 捕获 */
    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
        LOGE("main", "sigprocmask fail: %s", strerror(errno));
        return -1;
    }

    int fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (fd < 0) {
        LOGE("main", "signalfd fail: %s", strerror(errno));
        return -1;
    }
    return fd;
}

static int epoll_add(int fd) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(g_epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        LOGE("main", "epoll_ctl ADD fd=%d fail: %s", fd, strerror(errno));
        return -1;
    }
    return 0;
}

static void cleanup(void) {
    LOGI("main", "cleanup...");
    timer_stop();
    socket_close();
    i2c_close();
    if (g_sfd >= 0) close(g_sfd);
    if (g_epfd >= 0) close(g_epfd);
}

int main(void) {
    /* 1. 初始化 */
    log_init(LOG_INFO);
    LOGI("main", "ledd %s starting", LEDD_VERSION);

    if (i2c_init(LEDD_I2C_BUS, LEDD_I2C_ADDR) != 0) {
        LOGE("main", "i2c_init fail");
        return 1;
    }
    if (socket_init(LEDD_SOCK_PATH) != 0) {
        LOGE("main", "socket_init fail");
        i2c_close();
        return 1;
    }
    if (timer_init(LEDD_FRAME_MS) != 0) {
        LOGE("main", "timer_init fail");
        socket_close();
        i2c_close();
        return 1;
    }

    source_init();
    effect_init();
    command_init();

    /* 2. epoll */
    g_epfd = epoll_create1(EPOLL_CLOEXEC);
    if (g_epfd < 0) {
        LOGE("main", "epoll_create1 fail: %s", strerror(errno));
        cleanup();
        return 1;
    }

    g_sfd = setup_signalfd();
    if (g_sfd < 0) {
        cleanup();
        return 1;
    }

    if (epoll_add(socket_get_listen_fd()) != 0) { cleanup(); return 1; }
    if (epoll_add(timer_get_fd()) != 0)         { cleanup(); return 1; }
    if (epoll_add(g_sfd) != 0)                  { cleanup(); return 1; }

    LOGI("main", "ready, epfd=%d", g_epfd);

    /* 3. 主循环 */
    struct epoll_event events[MAX_EVENTS];
    char line[LEDD_LINE_BUF];

    while (1) {
        int n = epoll_wait(g_epfd, events, MAX_EVENTS, -1);

        if (n < 0) {
            if (errno == EINTR) continue;   /* 被无关信号打断，重试 */
            LOGE("main", "epoll_wait fail: %s", strerror(errno));
            break;
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            /* 信号 */
            if (fd == g_sfd) {
                struct signalfd_siginfo si;
                ssize_t r = read(g_sfd, &si, sizeof(si));
                (void)r;
                LOGI("main", "got signal %d, exit", si.ssi_signo);
                goto done;
            }

            /* 新连接 */
            if (fd == socket_get_listen_fd()) {
                /* 循环 accept，防止一次有多个连接排队 */
                while (1) {
                    int cfd = socket_accept();
                    if (cfd < 0) break;
                    if (epoll_add(cfd) != 0) {
                        socket_close_client(cfd);
                    }
                }
                continue;
            }

            /* timerfd */
            if (fd == timer_get_fd()) {
                timer_drain();
                effect_tick(now_ms());
                continue;
            }

            /* client 数据 */
            if (socket_is_client(fd)) {
                int ret = socket_recv_line(fd, line, sizeof(line));
                if (ret < 0) {
                    epoll_ctl(g_epfd, EPOLL_CTL_DEL, fd, NULL);
                    socket_close_client(fd);
                } else if (ret == 1) {
                    command_handle(line);
                }
                /* ret == 0: 暂无完整行，继续等 */
                continue;
            }
        }
    }

done:
    cleanup();
    LOGI("main", "ledd stopped");
    return 0;
}