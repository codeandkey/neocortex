#include "log.h"

int main(int argc, char** argv) {
    nc::log_init();
    nc_info("Started neocortex.");

    return 0;
}
