#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>

#include "i2c.h"
#include "ledd.h"
#include "log.h"
#include "util.h"

static int g_fd = -1;

/* 写 2 字节：{reg, val} */
static int i2c_w2(unsigned char reg, unsigned char val) {
    unsigned char buf[2] = {reg, val};
    if (write(g_fd, buf, 2) != 2) {
        LOGE("i2c", "w2(0x%02x,0x%02x) fail: %s", reg, val, strerror(errno));
        return -1;
    }
    return 0;
}

int i2c_init(int bus, int addr) {
    char path[32];
    snprintf(path, sizeof(path), "/dev/i2c-%d", bus);

    g_fd = open(path, O_RDWR);
    if (g_fd < 0) {
        LOGE("i2c", "open %s fail: %s", path, strerror(errno));
        return -1;
    }

    if (ioctl(g_fd, I2C_SLAVE_FORCE, addr) < 0) {
        LOGE("i2c", "ioctl I2C_SLAVE_FORCE fail: %s", strerror(errno));
        close(g_fd);
        g_fd = -1;
        return -1;
    }

    /* 初始化：0x12/0x13/0x14 = 0，0x11 = 默认亮度 */
    if (i2c_w2(0x12, 0x00) < 0) return -1;
    if (i2c_w2(0x13, 0x00) < 0) return -1;
    if (i2c_w2(0x14, 0x00) < 0) return -1;
    if (i2c_w2(0x11, (unsigned char)LEDD_DEFAULT_BRIGHT) < 0) return -1;

    LOGI("i2c", "init ok, /dev/i2c-%d addr=0x%02x", bus, addr);
    return 0;
}

/* 1 次传输写颜色：0x20 + 6 字节 */
int i2c_write_color(int r, int g, int b) {
    if (g_fd < 0) return -1;

    r = clampi(r, 0, 255);
    g = clampi(g, 0, 255);
    b = clampi(b, 0, 255);

    unsigned char buf[7] = {
        0x20,
        (unsigned char)r, (unsigned char)g, (unsigned char)b,
        (unsigned char)r, (unsigned char)g, (unsigned char)b,
    };

    if (write(g_fd, buf, 7) != 7) {
        LOGE("i2c", "write color fail: %s", strerror(errno));
        return -1;
    }
    return 0;
}

/* 单独写亮度：0x11 */
int i2c_write_bright(int bright) {
    if (g_fd < 0) return -1;
    bright = clampi(bright, 0, 3);
    if (bright > 3) bright = 3;
    return i2c_w2(0x11, (unsigned char)bright);
}

void i2c_close(void) {
    if (g_fd >= 0) {
        close(g_fd);
        g_fd = -1;
    }
}