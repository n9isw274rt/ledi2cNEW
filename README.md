# ledI2cNEW v2.0.1

联想拯救者 Pro（L79031）Y 字氛围灯的 LED 控制守护进程。

通过 Unix socket 接收命令，驱动 AW9106（I2C-6 @ 0x5b）实现静态色、动态效果、音乐律动。

---

## 特性

- **模块化架构**  16 个源文件，职责单一，加效果 = 加文件 + 表项
- **事件驱动**  epoll + timerfd + signalfd，空闲 CPU 0%
- **按需启停**  timerfd 只在 source=mic 且效果需要定时时运行
- **多客户端**  最多 8 个长连接，每 client 独立行缓冲
- **I2C 合并写**  7 字节一次传输改色，比旧版快 10 倍
- **无文件通道**  纯 socket，无轮询，无配置持久化

---

## 架构

    App 
           Unix socket  ledd  I2C  AW9106  LED
    audiohook 

## 目录结构

    ledi2cNEW/
      src/        源码（16 个 .c）
      include/    头文件
      jni/        NDK 构建脚本
      docs/       详细文档

---

## 命令

统一格式 命令名:参数:参数，末尾换行。参数缺省用默认值。

### 静态

| 命令 | 说明 |
|------|------|
| STATIC:R:G:B | 静态色（RGB 兼容） |
| OFF | 关灯（不影响 source） |

### 动态效果

| 命令 | 参数 |
|------|------|
| FADE:R:G:B:bright:ms | 渐变，默认 500ms |
| BREATH:R:G:B:period:bmin:bmax | 呼吸，默认 2000/0/255 |
| STROBE:R:G:B:on:off:edge | 闪烁，默认 100/200/20 |
| RAINBOW:step_ms:sat:val | 彩虹，默认 200/0.9/0.9 |
| PULSE:R:G:B:period:bmin:bmax | 脉冲，默认 1000/0/255 |
| RANDOM:interval:trans:sat | 随机，默认 1000/300/0.9 |

### 控制

| 命令 | 说明 |
|------|------|
| SOURCE:mic | 数据源 = App |
| SOURCE:system | 数据源 = audiohook |
| AUDIO:R,G,B | 音频颜色（逗号分隔） |
| STATUS | 查状态（走日志） |
| VERSION | 查版本（走日志） |

---

## 编译

    cd C:\android\ledi2cNEW
    & "$NDK\ndk-build.cmd" NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=jni\Android.mk NDK_APPLICATION_MK=jni\Application.mk

## 部署

    adb push libs\arm64-v8a\ledd /data/local/tmp/ledd_new
    adb shell "su -c 'killall -9 ledd'"
    adb shell "su -c 'cp /data/local/tmp/ledd_new /system/bin/ledd'"
    adb shell "su -c 'chmod 755 /system/bin/ledd'"
    adb shell "su -c 'rm -f /data/local/tmp/ledd.sock'"
    adb shell "su -c 'nohup /system/bin/ledd > /data/local/tmp/ledd.log 2>&1 &'"

---

## 硬件

| 项 | 值 |
|----|-----|
| I2C 总线 | /dev/i2c-6 |
| 从机地址 | 0x5b |
| 驱动 | AW9106 |
| PWM 寄存器 | 0x20-0x25 |
| 亮度档 | 0x11 |

关键约束：

- 必须 I2C_SLAVE_FORCE
- 必须写满 7 字节才覆盖两组寄存器
- 禁止 unbind 驱动（kernel panic）

---

## timerfd 规则

    timerfd 跑 = (source == SRC_MIC) && (effect need_timer)

---

## 文档索引

docs/ 目录：PROTOCOL.txt / ARCHITECTURE.txt / BUILD.txt / HARDWARE.txt / CHANGELOG.txt / REFACTOR.txt

---

## 版本

**2.0.1**  2026-09-26

详见 docs/CHANGELOG.txt。
