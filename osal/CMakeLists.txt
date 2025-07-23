# ================================================
# OSAL (Operating System Abstraction Layer)
# ================================================

add_library(lib_osal STATIC)
target_sources(lib_osal 
    PRIVATE 
    ./src/osal.c
)
target_include_directories(lib_osal
    PUBLIC  
    ./inc
)
target_link_libraries(lib_osal     
    PRIVATE 
    lib_cmsis_rtos2
)
add_dependencies(lib_osal lib_cmsis_rtos2)