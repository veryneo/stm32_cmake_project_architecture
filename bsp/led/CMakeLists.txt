# ================================================
# BSP LED
# ================================================

add_library(lib_bsp_led STATIC)

target_sources(lib_bsp_led
    PRIVATE
    ./adapter/src/bsp_led_adapter.c
    ./handler/src/bsp_led_handler.c
    ./driver/src/bsp_led_driver.c
)

target_include_directories(lib_bsp_led
    PUBLIC
    ./adapter/inc
    PRIVATE
    ./handler/inc
    ./driver/inc
)

target_link_libraries(lib_bsp_led
    PRIVATE
    lib_osal
    lib_mcu
)

add_dependencies(lib_bsp_led 
    lib_osal 
    lib_mcu
)
