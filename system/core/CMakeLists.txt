# ================================================
# System Core
# ================================================

add_library(lib_system_core STATIC)

target_sources(lib_system_core 
    PRIVATE
    ./src/system_core.c
)
target_include_directories(lib_system_core
    PUBLIC
    ./inc
)
target_link_libraries(lib_system_core
    PRIVATE
    lib_app_test
    lib_bsp_led
    lib_osal
    lib_mcu
)
add_dependencies(lib_system_core 
    lib_app_test 
    lib_bsp_led 
    lib_osal 
    lib_mcu
)
