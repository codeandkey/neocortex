#include "nn.h"
#include "util.h"

#include <cstdio>

using namespace neocortex;

static 

void nn::load(std::string path) {
    /* Read binary data from file */
    FILE* inp = fopen(path.c_str(), "rb");

    if (!inp) {
        throw util::fmterr("Failed to read weights from %s", path.c_str());
    }

    return 
}
