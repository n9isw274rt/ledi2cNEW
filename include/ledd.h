#ifndef LEDD_H
#define LEDD_H

#define LEDD_VERSION       "2.0.1"
#define LEDD_I2C_BUS       6
#define LEDD_I2C_ADDR      0x5b
#define LEDD_SOCK_PATH     "/data/local/tmp/ledd.sock"
#define LEDD_FPS           50
#define LEDD_FRAME_MS      (1000 / LEDD_FPS)
#define LEDD_DEFAULT_BRIGHT    0
#define LEDD_MAX_CLIENTS       8
#define LEDD_LINE_BUF          512

/* 数据源：只有两种 */
typedef enum {
    SRC_MIC    = 0,   /* App 控制（含麦克风采集） */
    SRC_SYSTEM = 1,   /* audiohook 控制（系统音频） */
} DataSource;

typedef enum {
    E_OFF = 0,
    E_STATIC,
    E_FADE,
    E_BREATH,
    E_STROBE,
    E_RAINBOW,
    E_PULSE,
    E_RANDOM,
    E_COUNT
} EffectMode;

typedef struct { int r, g, b; } RGB;

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} LogLevel;

#endif