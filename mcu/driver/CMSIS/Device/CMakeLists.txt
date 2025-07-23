# ================================================
# CMSIS Device
# ================================================

if (MCU_MANUFACTURER STREQUAL "ST")
    message(STATUS "MCU manufacturer: ${MCU_MANUFACTURER}")
    add_subdirectory(ST)
else()
    message(FATAL_ERROR "Unsupported MCU Manufacturer: ${MCU_MANUFACTURER}")
endif()