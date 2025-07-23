# ================================================
# MCU Time
# ================================================

add_library(lib_mcu_time STATIC)
target_sources(lib_mcu_time
    PRIVATE
    ./src/mcu_time.c
)
target_include_directories(lib_mcu_time
    PUBLIC
    ./inc
)
target_link_libraries(lib_mcu_time
    PRIVATE
    lib_mcu_hal
)
add_dependencies(lib_mcu_time 
    lib_mcu_hal
)