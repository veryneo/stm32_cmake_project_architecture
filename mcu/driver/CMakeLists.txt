# ================================================
# MCU Driver
# ================================================

message(STATUS "MCU Driver")

if (MCU_MODEL STREQUAL "STM32WB55xx")
    message(STATUS "MCU model: ${MCU_MODEL}")
    
    set(MCU_MANUFACTURER    "ST")
    set(MCU_FAMILY          "STM32WBxx")
    set(MCU_HAL_FOLDER      "STM32WBxx_HAL_Driver")
    set(MCU_COMPILE_FLAGS   ${MCU_MODEL} USE_HAL_DRIVER) # Don't use a string "${MCU_MODEL} USE_HAL_DRIVER"
else()
    message(FATAL_ERROR "Unsupported MCU Model: ${MCU_MODEL}")
endif()

add_subdirectory(CMSIS)
add_subdirectory(${MCU_HAL_FOLDER})
