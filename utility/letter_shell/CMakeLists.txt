# ================================================
# Shell (Letter Shell)
# ================================================

add_library(lib_shell STATIC)

target_sources(lib_shell
    PRIVATE
    # Core
    ./src/shell.c
    ./src/shell_cmd_list.c
    ./src/shell_ext.c
    ./src/shell_companion.c
    # Extension
    ./extension/log/log.c
)
target_include_directories(lib_shell
    PUBLIC
    ./src
    ./port
    ./extension/log
)
target_compile_definitions(lib_shell
    PUBLIC
    -DSHELL_CFG_USER="shell_cfg_user.h"
)
target_link_libraries(lib_shell
    PRIVATE
    lib_osal
)
add_dependencies(lib_shell
    lib_osal
)