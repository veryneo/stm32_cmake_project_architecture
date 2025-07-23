# ================================================
# MCU Config FreeRTOS ST STM32WBxx
# ================================================

if (NOT MCU_MANUFACTURER STREQUAL "ST" OR 
    NOT MCU_FAMILY STREQUAL "STM32WBxx")
    message(FATAL_ERROR "Unsupported MCU Manufacturer or Family: ${MCU_MANUFACTURER} ${MCU_FAMILY}")
endif()

message(STATUS "MCU Family: ${MCU_FAMILY}")

add_library(lib_mcu_config_freertos INTERFACE)
target_include_directories(lib_mcu_config_freertos INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)