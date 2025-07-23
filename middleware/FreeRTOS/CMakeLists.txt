# ================================================
# FreeRTOS
# ================================================

add_library(lib_freertos STATIC)

target_sources(lib_freertos 
    PRIVATE 
    # source
    ./Source/event_groups.c
    ./Source/list.c
    ./Source/queue.c
    ./Source/stream_buffer.c
    ./Source/tasks.c
    ./Source/timers.c
    # portable (memory management)
    # ./Source/portable/MemMang/heap_1.c
    # ./Source/portable/MemMang/heap_2.c
    # ./Source/portable/MemMang/heap_3.c
    ./Source/portable/MemMang/heap_4.c
    # ./Source/portable/MemMang/heap_5.c
)

target_include_directories(lib_freertos
    PUBLIC  
    # common
    ./Source/include
)

if (MCU_MODEL STREQUAL "STM32WB55xx")
    target_sources(lib_freertos
        PRIVATE
        # portable (compiler & processor)
        ./Source/portable/GCC/ARM_CM4F/port.c
    )

    target_include_directories(lib_freertos
        PUBLIC
        # portable (compiler & processor)
        ./Source/portable/GCC/ARM_CM4F
    )
endif()

target_link_libraries(lib_freertos
    PUBLIC
    lib_mcu_config_freertos
)
add_dependencies(lib_freertos 
    lib_mcu_config_freertos
)