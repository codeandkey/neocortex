#pragma once

#include <stdlib.h>
#include <stdio.h>

#define NC_LOG_FD stderr

#define NC_LOG_RED "\e[0;31m"
#define NC_LOG_GREEN "\e[0;32m"
#define NC_LOG_YELLOW "\e[0;33m"
#define NC_LOG_BLUE "\e[0;34m"
#define NC_LOG_CYAN "\e[0;36m"
#define NC_LOG_RESET "\e[0m"

#define nc_log(x, ...) fprintf(NC_LOG_FD, "[%s] " x "\n", __func__, ##__VA_ARGS__)

#define nc_info(x, ...) nc_log(x, ##__VA_ARGS__)
#define nc_error(x, ...) nc_log(NC_LOG_RED "(ERROR) " x NC_LOG_RESET, ##__VA_ARGS__)
#define nc_abort(x, ...) (nc_error("(CRITICAL) " x, ##__VA_ARGS__), exit(1))

#ifdef NC_DEBUG
#define nc_debug(x, ...) nc_log(NC_LOG_CYAN "(DEBUG) " x NC_LOG_RESET, ##__VA_ARGS__)
#define nc_assertf(c, x, ...) if (!(c)) nc_abort("assertion %s failed: " x, #c, ##__VA_ARGS__)
#define nc_assert(c) if (!(c)) nc_abort("assertion %s failed!", #c)
#else
#define nc_assertf(c, x, ...)
#define nc_assert(c)
#define nc_debug(x, ...)
#endif
