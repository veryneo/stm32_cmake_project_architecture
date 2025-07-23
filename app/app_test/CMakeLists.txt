# ================================================
# APP Test
# ================================================

add_library(lib_app_test STATIC)

target_sources(lib_app_test
    PRIVATE
    ./src/app_test.c
    ./port/src/app_test_port.c
)

target_include_directories(lib_app_test
    PUBLIC
    ./inc
    PRIVATE
    ./port/inc
)

target_link_libraries(lib_app_test
    PRIVATE
    lib_bsp_led
    lib_osal
)

add_dependencies(lib_app_test 
    lib_bsp_led 
    lib_osal
)

