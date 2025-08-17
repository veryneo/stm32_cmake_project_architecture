# ================================================
# MCU UART
# ================================================

add_library(lib_mcu_uart STATIC)

target_sources(lib_mcu_uart
    PRIVATE
    ./src/mcu_uart.c
)
target_include_directories(lib_mcu_uart
    PUBLIC
    ./inc
)
target_link_libraries(lib_mcu_uart
    PRIVATE
    lib_mcu_hal
)
add_dependencies(lib_mcu_uart 
    lib_mcu_hal
)