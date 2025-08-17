# ================================================
# OSAL (Operating System Abstraction Layer)
# ================================================

# Library: lib_osal_core
add_library(lib_osal_core STATIC)
target_sources(lib_osal_core 
    PRIVATE 
    ./core/src/osal_core.c
)
target_include_directories(lib_osal_core
    PUBLIC  
    ./core/inc
)
target_link_libraries(lib_osal_core     
    PRIVATE 
    lib_cmsis_rtos2
)
add_dependencies(lib_osal_core 
    lib_cmsis_rtos2
)

# Library: lib_osal_extension
add_library(lib_osal_extension STATIC)
target_sources(lib_osal_extension 
    PRIVATE 
    ./extension/src/osal_extension.c
)
target_include_directories(lib_osal_extension
    PUBLIC  
    ./extension/inc
)
target_compile_definitions(lib_osal_extension
    PRIVATE
    OSAL_EX_FREERTOS
)
target_link_libraries(lib_osal_extension
    lib_freertos
)
add_dependencies(lib_osal_extension
    lib_freertos
)

# Library: lib_osal
add_library(lib_osal INTERFACE)
target_include_directories(lib_osal
    INTERFACE
    ./inc
)
target_link_libraries(lib_osal
    INTERFACE
    lib_osal_core
    lib_osal_extension
)
add_dependencies(lib_osal
    lib_osal_core
    lib_osal_extension
)
add_dependencies(lib_osal
    lib_freertos
)