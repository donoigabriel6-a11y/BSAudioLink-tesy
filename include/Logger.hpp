#pragma once

#include <android/log.h>
#include <cstdarg>

struct LoggerOptions {
    LoggerOptions(bool = false, bool = true) {}
};

class Logger {
public:
    template <typename ModInfoType>
    explicit Logger(const ModInfoType&, LoggerOptions = LoggerOptions()) {}

    void debug(const char* format, ...) const {
        va_list args;
        va_start(args, format);
        __android_log_vprint(ANDROID_LOG_DEBUG, "BSAudioLink", format, args);
        va_end(args);
    }

    void warn(const char* format, ...) const {
        va_list args;
        va_start(args, format);
        __android_log_vprint(ANDROID_LOG_WARN, "BSAudioLink", format, args);
        va_end(args);
    }

    void info(const char* format, ...) const {
        va_list args;
        va_start(args, format);
        __android_log_vprint(ANDROID_LOG_INFO, "BSAudioLink", format, args);
        va_end(args);
    }

    void error(const char* format, ...) const {
        va_list args;
        va_start(args, format);
        __android_log_vprint(ANDROID_LOG_ERROR, "BSAudioLink", format, args);
        va_end(args);
    }

    void flush() const {}
};
