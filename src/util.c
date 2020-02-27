#include "util.h"

#include <execinfo.h>

/* Print out a backtrace. */
void nc_backtrace() {
    void* ptrs[256];

    int count = backtrace(ptrs, sizeof ptrs / sizeof *ptrs);

    fprintf(NC_LOG_FD, "backtrace:\n");
    backtrace_symbols_fd(ptrs, count, fileno(NC_LOG_FD));
}
