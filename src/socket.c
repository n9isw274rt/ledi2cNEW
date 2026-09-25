#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "socket.h"
#include "ledd.h"
#include "log.h"

/* ============ 每 client 状态 ============ */
typedef struct {
    int  fd;
    char buf[LEDD_LINE_BUF];
    int  pos;      /* 已缓存字节数 */
} Client;

static int   g_listen_fd = -1;
static Client g_clients[LEDD_MAX_CLIENTS];

/* ============ 内部 ============ */
static Client *find_client(int fd) {
    for (int i = 0; i < LEDD_MAX_CLIENTS; i++) {
        if (g_clients[i].fd == fd) return &g_clients[i];
    }
    return NULL;
}

static Client *alloc_client(void) {
    for (int i = 0; i < LEDD_MAX_CLIENTS; i++) {
        if (g_clients[i].fd < 0) return &g_clients[i];
    }
    return NULL;
}

/* 从 client 缓冲中取出一行（不含 \n），返回长度；没有完整行返回 0 */
static int extract_line(Client *cl, char *out, int size) {
    for (int i = 0; i < cl->pos; i++) {
        if (cl->buf[i] == '\n' || cl->buf[i] == '\r') {
            int len = i;
            if (len >= size) len = size - 1;
            memcpy(out, cl->buf, len);
            out[len] = 0;

            /* 跳过 \r\n 或 \n */
            int skip = i + 1;
            while (skip < cl->pos && (cl->buf[skip] == '\n' || cl->buf[skip] == '\r')) {
                skip++;
            }
            int remain = cl->pos - skip;
            if (remain > 0) memmove(cl->buf, cl->buf + skip, remain);
            cl->pos = remain;
            return len;
        }
    }
    return 0;
}

/* ============ 对外接口 ============ */

int socket_init(const char *path) {
    unlink(path);

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        LOGE("socket", "socket() fail: %s", strerror(errno));
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOGE("socket", "bind fail: %s", strerror(errno));
        close(fd);
        return -1;
    }

    chmod(path, 0666);

    if (listen(fd, LEDD_MAX_CLIENTS) < 0) {
        LOGE("socket", "listen fail: %s", strerror(errno));
        close(fd);
        return -1;
    }

    /* 非阻塞 */
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    g_listen_fd = fd;

    for (int i = 0; i < LEDD_MAX_CLIENTS; i++) {
        g_clients[i].fd = -1;
        g_clients[i].pos = 0;
    }

    LOGI("socket", "init ok, listen fd=%d path=%s", fd, path);
    return 0;
}

int socket_get_listen_fd(void) {
    return g_listen_fd;
}

int socket_accept(void) {
    int cfd = accept(g_listen_fd, NULL, NULL);
    if (cfd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOGW("socket", "accept fail: %s", strerror(errno));
        }
        return -1;
    }

    Client *cl = alloc_client();
    if (!cl) {
        LOGW("socket", "too many clients, reject fd=%d", cfd);
        close(cfd);
        return -1;
    }

    /* 非阻塞 */
    int flags = fcntl(cfd, F_GETFL, 0);
    fcntl(cfd, F_SETFL, flags | O_NONBLOCK);

    cl->fd = cfd;
    cl->pos = 0;

    LOGI("socket", "client accepted fd=%d", cfd);
    return cfd;
}

int socket_is_client(int fd) {
    return find_client(fd) != NULL;
}

int socket_recv_line(int fd, char *out, int size) {
    Client *cl = find_client(fd);
    if (!cl) return -1;

    /* 先看缓冲里有没有完整行 */
    if (extract_line(cl, out, size) > 0) return 1;

    /* 缓冲满且没找到 \n，丢一半（防死） */
    if (cl->pos >= LEDD_LINE_BUF) {
        LOGW("socket", "fd=%d line buffer overflow, drop", fd);
        cl->pos = 0;
    }

    /* recv 更多数据 */
    int n = recv(fd, cl->buf + cl->pos, LEDD_LINE_BUF - cl->pos, 0);

    if (n > 0) {
        cl->pos += n;
        if (extract_line(cl, out, size) > 0) return 1;
        return 0;   /* 还没有完整行 */
    }

    if (n == 0) {
        LOGI("socket", "fd=%d closed by peer", fd);
        return -1;
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return 0;
    }

    LOGW("socket", "fd=%d recv fail: %s", fd, strerror(errno));
    return -1;
}

void socket_close_client(int fd) {
    Client *cl = find_client(fd);
    if (!cl) return;
    close(cl->fd);
    cl->fd = -1;
    cl->pos = 0;
    LOGI("socket", "client closed fd=%d", fd);
}

int socket_send(int fd, const char *buf, int len) {
    if (len <= 0) return 0;
    int n = send(fd, buf, len, MSG_NOSIGNAL);
    if (n < 0) {
        LOGW("socket", "send fd=%d fail: %s", fd, strerror(errno));
        return -1;
    }
    return n;
}

void socket_close(void) {
    for (int i = 0; i < LEDD_MAX_CLIENTS; i++) {
        if (g_clients[i].fd >= 0) {
            close(g_clients[i].fd);
            g_clients[i].fd = -1;
        }
    }
    if (g_listen_fd >= 0) {
        close(g_listen_fd);
        g_listen_fd = -1;
    }
}