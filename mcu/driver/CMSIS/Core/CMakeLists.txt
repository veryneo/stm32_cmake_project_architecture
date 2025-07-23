# ================================================
# CMSIS Core
# ================================================

add_library(lib_cmsis_core INTERFACE)

target_include_directories(lib_cmsis_core
    INTERFACE
    ./Include
)