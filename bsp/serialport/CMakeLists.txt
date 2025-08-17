# ================================================
# BSP Serialport
# ================================================

add_library(lib_bsp_serialport STATIC)

target_sources(lib_bsp_serialport
    PRIVATE
    ./adapter/src/bsp_serialport_adapter.c
    ./handler/src/bsp_serialport_handler.c
    ./driver/src/bsp_serialport_driver.c
)
target_include_directories(lib_bsp_serialport
    PUBLIC
    ./adapter/inc
    PRIVATE
    ./handler/inc
    ./driver/inc
)
target_link_libraries(lib_bsp_serialport
    PRIVATE
    lib_osal
    lib_mcu
    lib_ringbuf
)
add_dependencies(lib_bsp_serialport 
    lib_osal 
    lib_mcu
    lib_ringbuf
)