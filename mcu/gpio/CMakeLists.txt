# ================================================
# MCU GPIO
# ================================================

add_library(lib_mcu_gpio STATIC)
target_sources(lib_mcu_gpio
    PRIVATE
    ./src/mcu_gpio.c
)
target_include_directories(lib_mcu_gpio
    PUBLIC
    ./inc
)
target_link_libraries(lib_mcu_gpio
    PRIVATE
    lib_mcu_hal
)
add_dependencies(lib_mcu_gpio 
    lib_mcu_hal
)