#pragma once

/**
 * Logging macros and functions.
 */

#include <cstdarg>
#include <cstdio>

namespace nc {
    enum LogLevel {
        CRITICAL = 0,
        ERROR    = 1,
        WARNING  = 2,
        INFO     = 3,
        DEBUG    = 4,
    };

    void log_basic(int level, const char* func, const char* fmt, ...);

    void log_init();
    void log_set_color_enabled(bool enabled);
    void log_set_output(FILE* out);
    void log_set_verbosity(int level);
}

#define nc_critical(fmt, ...) nc::log_basic(nc::LogLevel::CRITICAL, __func__, fmt, ##__VA_ARGS__)
#define nc_error(fmt, ...) nc::log_basic(nc::LogLevel::ERROR, __func__, fmt, ##__VA_ARGS__)
#define nc_warning(fmt, ...) nc::log_basic(nc::LogLevel::WARNING, __func__, fmt, ##__VA_ARGS__)
#define nc_info(fmt, ...) nc::log_basic(nc::LogLevel::INFO, __func__, fmt, ##__VA_ARGS__)
#define nc_debug(fmt, ...) nc::log_basic(nc::LogLevel::DEBUG, __func__, fmt, ##__VA_ARGS__)
