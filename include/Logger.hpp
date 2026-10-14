#pragma once

#include <android/log.h>
#include <cstdarg>
#include <cstdio>
#include <sys/stat.h>
#include <time.h>

namespace {
    inline void appendAudioLinkFileLog(int priority, const char* format, va_list args) {
        // Keep a persistent on-device diagnostic log so users don't have to run
        // ADB just to confirm that AudioLink loaded and initialized.
        constexpr const char* directory = "/sdcard/ModData/com.beatgames.beatsaber/Logs";
        constexpr const char* path = "/sdcard/ModData/com.beatgames.beatsaber/Logs/BSAudioLink.log";
        mkdir("/sdcard/ModData", 0775);
        mkdir("/sdcard/ModData/com.beatgames.beatsaber", 0775);
        mkdir(directory, 0775);
        FILE* file = std::fopen(path, "a");
        if (!file) return;
        time_t now = std::time(nullptr);
        struct tm local{};
        localtime_r(&now, &local);
        char timestamp[32]{};
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &local);
        char message[2048]{};
        va_list copy;
        va_copy(copy, args);
        std::vsnprintf(message, sizeof(message), format, copy);
        va_end(copy);
        const char* level = priority >= ANDROID_LOG_ERROR ? "ERROR" :
                            priority >= ANDROID_LOG_WARN ? "WARN" :
                            priority >= ANDROID_LOG_INFO ? "INFO" : "DEBUG";
        std::fprintf(file, "[%s] [%s] %s\n", timestamp, level, message);
        std::fclose(file);
    }
}

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
        appendAudioLinkFileLog(ANDROID_LOG_DEBUG, format, args);
        va_end(args);
    }

    void warn(const char* format, ...) const {
        va_list args;
        va_start(args, format);
        __android_log_vprint(ANDROID_LOG_WARN, "BSAudioLink", format, args);
        appendAudioLinkFileLog(ANDROID_LOG_WARN, format, args);
        va_end(args);
    }

    void info(const char* format, ...) const {
        va_list args;
        va_start(args, format);
        __android_log_vprint(ANDROID_LOG_INFO, "BSAudioLink", format, args);
        appendAudioLinkFileLog(ANDROID_LOG_INFO, format, args);
        va_end(args);
    }

    void critical(const char* format, ...) const {
        va_list args;
        va_start(args, format);
        __android_log_vprint(ANDROID_LOG_FATAL, "BSAudioLink", format, args);
        appendAudioLinkFileLog(ANDROID_LOG_FATAL, format, args);
        va_end(args);
    }

    void error(const char* format, ...) const {
        va_list args;
        va_start(args, format);
        __android_log_vprint(ANDROID_LOG_ERROR, "BSAudioLink", format, args);
        appendAudioLinkFileLog(ANDROID_LOG_ERROR, format, args);
        va_end(args);
    }

    void flush() const {}
};
