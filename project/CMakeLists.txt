# ================================================
# Project
# ================================================

add_executable(${EXE_NAME})

target_sources(${EXE_NAME} 
    PRIVATE
    ./main.c
    ./syscalls.c
    ./sysmem.c
)
target_link_libraries(${EXE_NAME} 
    PRIVATE   
    lib_system_core
    lib_mcu
    lib_cmsis_device_startup
    lib_cmsis_device_linker
)
add_dependencies(${EXE_NAME}    
    lib_system_core
    lib_mcu
    lib_cmsis_device_startup
    lib_cmsis_device_linker
)

# Generate .bin file (Transfer .elf file to .bin file)
if(CMAKE_OBJCOPY)
    add_custom_command(
        TARGET ${EXE_NAME} 
        POST_BUILD 
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXE_NAME}> $<TARGET_FILE_DIR:${EXE_NAME}>/${EXE_NAME}.bin) 
endif()
