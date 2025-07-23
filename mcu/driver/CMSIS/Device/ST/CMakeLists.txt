# ================================================
# CMSIS Device ST
# ================================================

# Check if the MCU manufacturer is supported
if(NOT MCU_MANUFACTURER STREQUAL "ST")
    message(FATAL_ERROR "Unsupported MCU Manufacturer: ${MCU_MANUFACTURER}")
endif()

if (MCU_FAMILY STREQUAL "STM32WBxx")
    add_subdirectory(STM32WBxx)
else()
    message(FATAL_ERROR "Unsupported MCU Family: ${MCU_FAMILY}")
endif()