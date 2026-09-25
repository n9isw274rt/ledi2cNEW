LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ledd

LOCAL_SRC_FILES := \
    ../src/main.c \
    ../src/i2c.c \
    ../src/socket.c \
    ../src/command.c \
    ../src/effect.c \
    ../src/effect_static.c \
    ../src/effect_fade.c \
    ../src/effect_breath.c \
    ../src/effect_strobe.c \
    ../src/effect_rainbow.c \
    ../src/effect_pulse.c \
    ../src/effect_random.c \
    ../src/source.c \
    ../src/audio.c \
    ../src/log.c \
    ../src/util.c \
    ../src/timer.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include

LOCAL_CFLAGS := -O2 -Wall -Wextra -Wno-unused-parameter

LOCAL_LDLIBS := -lm

include $(BUILD_EXECUTABLE)