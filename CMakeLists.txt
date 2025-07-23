cmake_minimum_required(VERSION 3.20)

project(STM32PRJ LANGUAGES C ASM)

set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
message(STATUS "Project Build type: ${CMAKE_BUILD_TYPE}")

set(MCU_MODEL "STM32WB55xx" CACHE STRING "MCU Model")


if(MCU_MODEL STREQUAL "STM32WB55xx")
    set(MCPU                        "-mcpu=Cortex-M4")
    set(MFPU				        "-mfpu=fpv4-sp-d16")
    set(MFLOAT_ABI                  "-mfloat-abi=hard")
endif()

set(RUNTIME_LIBRARY                 "--specs=nano.specs")		                                                                      
set(RUNTIME_LIBRARY_SYSCALLS 	    "--specs=nosys.specs")		                                                                      
set(EXE_NAME					    "${CMAKE_PROJECT_NAME}")											                                    
set(MAP_NAME					    "${CMAKE_PROJECT_NAME}")

# set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/mcu/driver/CMSIS/Device/ST/STM32WBxx/Source/gcc/linker/stm32wb55xx_flash_cm4.ld")

set(CMAKE_EXECUTABLE_SUFFIX         ".elf")
set(CMAKE_STATIC_LIBRARY_SUFFIX     ".a")
set(CMAKE_C_FLAGS                   "${MCPU} -std=gnu11 ${MFPU} ${MFLOAT_ABI} ${RUNTIME_LIBRARY} -mthumb -Wall -Werror -ffunction-sections -fdata-sections")
# set(CMAKE_EXE_LINKER_FLAGS          "-T${LINKER_SCRIPT} ${RUNTIME_LIBRARY_SYSCALLS} -Wl,-Map=${MAP_NAME}.map -Wl,--gc-sections -static -Wl,--start-group -lc -lm -Wl,--end-group")
set(CMAKE_EXE_LINKER_FLAGS          "${RUNTIME_LIBRARY_SYSCALLS} -Wl,-Map=${MAP_NAME}.map -Wl,--gc-sections -static -Wl,--start-group -lc -lm -Wl,--end-group")
set(CMAKE_ASM_FLAGS                 "${CMAKE_C_FLAGS} -x assembler-with-cpp")

set(CMAKE_VERBOSE_MAKEFILE          OFF)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release" OR 
   ${CMAKE_BUILD_TYPE} STREQUAL "RELEASE" OR 
   ${CMAKE_BUILD_TYPE} STREQUAL "release" )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 	${CMAKE_SOURCE_DIR}/output/release)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR
       ${CMAKE_BUILD_TYPE} STREQUAL "DEBUG" OR 
       ${CMAKE_BUILD_TYPE} STREQUAL "debug" )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 	${CMAKE_SOURCE_DIR}/output/debug)
endif()

add_subdirectory(project)
add_subdirectory(system)
add_subdirectory(app)
add_subdirectory(bsp)
add_subdirectory(osal)
add_subdirectory(middleware)
add_subdirectory(mcu)

