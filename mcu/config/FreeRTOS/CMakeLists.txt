# ================================================
# MCU Config FreeRTOS
# ================================================

if (MCU_MODEL STREQUAL "STM32WB55xx")
    message(STATUS "MCU model: ${MCU_MODEL}")
    
    set(MCU_MANUFACTURER    "ST")
    set(MCU_FAMILY          "STM32WBxx")
else()
    message(FATAL_ERROR "Unsupported MCU Model: ${MCU_MODEL}")
endif()

add_subdirectory(${MCU_MANUFACTURER})