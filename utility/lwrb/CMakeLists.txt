# ================================================
# Ring Buffer (Light Weight Ring Buffer)
# ================================================

add_library(lib_ringbuf STATIC)
target_sources(lib_ringbuf
    PRIVATE
    ./src/lwrb/lwrb.c
    ./src/lwrb/lwrb_ex.c
)
target_include_directories(lib_ringbuf
    PUBLIC
    ./src/include/lwrb
    ./src/include       # For lwrb.c/lwrb_ex.c, these files use include "lwrb/lwrb.h"
)