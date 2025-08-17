# ================================================
# MCU
# ================================================

add_subdirectory(driver)
add_subdirectory(core)
add_subdirectory(gpio)
add_subdirectory(time)
add_subdirectory(uart)
add_subdirectory(config)

# A consolidated interface library to access all MCU modules
# Userf can only include "mcu.h" to use all MCU modules
add_library(lib_mcu INTERFACE)
target_include_directories(lib_mcu INTERFACE 
    ./inc
)
target_link_libraries(lib_mcu INTERFACE
    lib_mcu_core
    lib_mcu_gpio
    lib_mcu_time
    lib_mcu_uart
)
add_dependencies(lib_mcu 
    lib_mcu_core 
    lib_mcu_gpio 
    lib_mcu_time
    lib_mcu_uart
)

