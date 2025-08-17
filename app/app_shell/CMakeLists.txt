# ================================================
# APP Shell
# ================================================

add_library(lib_app_shell STATIC)
target_sources(lib_app_shell
    PRIVATE
    ./src/app_shell.c
    ./port/src/app_shell_port.c
)
target_include_directories(lib_app_shell
    PUBLIC
    ./inc
    ./port/inc
)
target_link_libraries(lib_app_shell
    PRIVATE
    lib_bsp_serialport
    lib_osal
    lib_shell
)
add_dependencies(lib_app_shell
    lib_bsp_serialport
    lib_osal
    lib_shell
)