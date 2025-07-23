# ================================================
# CMSIS RTOS2
# ================================================

add_library(lib_cmsis_rtos2 STATIC)
target_sources(lib_cmsis_rtos2
    PRIVATE
    # CMSIS RTOS2 common
    # ./Source/os_systick.c
    # ./Source/os_tick_ptim.c
    # ./Source/os_tick_gtim.c

    # CMSIS FreeRTOS specific
    ./FreeRTOS/Source/cmsis_os2.c
    # ./FreeRTOS/Source/freertos_evr.c
    # ./FreeRTOS/Source/clib_os.c
    # ./FreeRTOS/Source/handlers.c
)
target_include_directories(lib_cmsis_rtos2
    PUBLIC
    # CMSIS RTOS2 common
    ./Include   
    PRIVATE
    # CMSIS FreeRTOS specific
    ./FreeRTOS/Include
)
target_link_libraries(lib_cmsis_rtos2
    PUBLIC
    lib_cmsis_core
    PRIVATE
    lib_freertos
)

add_dependencies(lib_cmsis_rtos2
    lib_freertos
    lib_cmsis_core
)
