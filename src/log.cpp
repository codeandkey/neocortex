#include "log.h"

#include <cstdio>
#include <ctime>

#include <unistd.h>

static const char* NC_LOG_COLORS[] = {
    "\x1b[0;31m",
    "\x1b[0;31m",
    "\x1b[0;33m",
    "\x1b[0;34m",
    "\x1b[0;36m",
};

static const char* NC_LOG_TAGS[] = {
    "CRITICAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
};

static bool nc_log_color_enabled = false;
static bool nc_log_color_supported = false;
static bool nc_log_ready = false;
static int  nc_log_verbosity = nc::LogLevel::DEBUG;
static FILE* nc_log_file = NULL;

void nc::log_basic(int level, const char* func, const char* fmt, ...) {
    if (!nc_log_ready) return;

    va_list args;
    va_start(args, fmt);

    /* Print out time of log message. */
    char datestr[64] = {0};
    time_t curtime = time(NULL);
    struct tm* curtm = localtime(&curtime);

    strftime(datestr, sizeof datestr, "%H:%M:%S", curtm);

    fprintf(nc_log_file, "%s ", datestr);

    /* write color if enabled */
    if (nc_log_color_enabled && nc_log_color_supported) {
        fprintf(nc_log_file, NC_LOG_COLORS[level]);
    }

    /* write tag */
    fprintf(nc_log_file, "%s ", NC_LOG_TAGS[level]);

    /* write color reset if needed */
    if (nc_log_color_enabled && nc_log_color_supported) {
        fprintf(nc_log_file, "\x1b[0;39m");
    }

    /* write content */
    vfprintf(nc_log_file, fmt, args);

    /* write newline */
    fprintf(nc_log_file, "\n");

    va_end(args);
}

void nc::log_init() {
    log_set_output(stderr);
    log_set_color_enabled(true);
    nc_log_ready = true;
}

void nc::log_set_color_enabled(bool enabled) {
    nc_log_color_enabled = enabled;
}

void nc::log_set_output(FILE* out) {
    nc_log_color_supported = isatty(fileno(out));
    nc_log_file = out;
}

void nc::log_set_verbosity(int level) {
    nc_log_verbosity = level;
}
