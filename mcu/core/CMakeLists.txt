# ================================================
# MCU Core
# ================================================

add_library(lib_mcu_core STATIC)
target_sources(lib_mcu_core
    PRIVATE
    ./src/mcu_core.c
)
target_include_directories(lib_mcu_core
    PUBLIC
    ./inc
)
target_link_libraries(lib_mcu_core
    PRIVATE
    lib_mcu_hal
)
add_dependencies(lib_mcu_core 
    lib_mcu_hal
)