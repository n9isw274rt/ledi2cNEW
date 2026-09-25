# ledd 2.0
================================================================================
ledd 2.0 - LED 控制守护进程
================================================================================

【一句话】
接收 socket 命令，控制 AW9106 LED 驱动的守护进程。

【架构】
    App 
             Unix socket
             ledd  I2C  AW9106  LED
            
    audiohook 

【特性】
    - 插件式效果引擎（加效果 = 加文件 + 表项）
    - 事件驱动（epoll + timerfd + signalfd）
    - 按需启停 timerfd（空闲 CPU  0%）
    - 多客户端长连接
    - I2C 合并写（1 次传输改色）
    - 无文件通道，无配置模块

【目录结构】
    ledi2cNEW/
       src/            源码
          main.c              入口 + epoll 主循环
          i2c.c               AW9106 通信
          socket.c            多客户端 socket
          command.c           命令分发（查表）
          effect.c            效果引擎（表）
          effect_static.c     静态
          effect_fade.c       渐变
          effect_breath.c     呼吸
          effect_strobe.c     闪烁
          effect_rainbow.c    彩虹
          effect_pulse.c      脉冲
          effect_random.c     随机
          source.c            数据源
          audio.c             音频颜色
          timer.c             timerfd
          log.c               分级日志
          util.c              工具
       include/        头文件
       jni/            NDK 构建脚本
       docs/           文档

【快速开始】
    1. 编译：见 BUILD.txt
    2. 部署：见 BUILD.txt
    3. 发命令：见 PROTOCOL.txt

【文档索引】
    README.txt       本文件
    PROTOCOL.txt     socket 协议
    ARCHITECTURE.txt 架构与模块
    BUILD.txt        编译部署
    HARDWARE.txt     硬件信息与坑
    CHANGELOG.txt    版本变更
    REFACTOR.txt     新旧对比

================================================================================