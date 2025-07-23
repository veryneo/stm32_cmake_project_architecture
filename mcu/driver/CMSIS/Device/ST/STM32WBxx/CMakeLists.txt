# ================================================
# CMSIS Device ST STM32WBxx
# ================================================

if(NOT MCU_MANUFACTURER STREQUAL "ST" OR 
   NOT MCU_FAMILY STREQUAL "STM32WBxx")
    message(FATAL_ERROR "Unsupported MCU Manufacturer or Family: ${MCU_MANUFACTURER} ${MCU_FAMILY}")
endif()

message(STATUS "MCU Family: ${MCU_FAMILY}")

string(TOLOWER ${MCU_MODEL} MCU_MODEL_LOWER)

# Library for CMSIA device startup file
add_library(lib_cmsis_device_startup STATIC)
target_sources(lib_cmsis_device_startup
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Source/gcc/startup_${MCU_MODEL_LOWER}_cm4.s
)

# Library for CMSIS device linker script
set(LINKER_SCRIPT_PATH "${CMAKE_CURRENT_LIST_DIR}/Source/gcc/linker/${MCU_MODEL_LOWER}_flash_cm4.ld")
add_library(lib_cmsis_device_linker INTERFACE)
target_link_options(lib_cmsis_device_linker
    INTERFACE
    -T${LINKER_SCRIPT_PATH}
)









