project(demo C CXX)
cmake_minimum_required(VERSION 3.5)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(-DprojCOVERAGE_TEST=0)

set(FREERTOS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/FreeRTOS")

include_directories(.
                    ${FREERTOS_DIR}/Source/include
                    ${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix
                    ${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix/utils
                    ${FREERTOS_DIR}/Demo/Common/include
                    ${FREERTOS_DIR}/Demo/Posix_GCC)

FILE(GLOB_RECURSE FREERTOS_SOURCES
    ${FREERTOS_DIR}/Source/*.c
)

# Memory manager (use malloc() / free() )
list(APPEND FREERTOS_SOURCES "${FREERTOS_DIR}/Source/portable/MemMang/heap_3.c")
# Posix port
list(APPEND FREERTOS_SOURCES "${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c")
list(APPEND FREERTOS_SOURCES "${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix/port.c")

# Makefile: CFLAGS (-DprojCOVERAGE_TEST=0 as definition)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3 -O0 -D_WINDOWS_")

# ?? Makefile:  LDFLAGS (-pthread with link library)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ggdb3 -O0")

message("Project: ${PROJECT_NAME}")
message("Sources: ${FREERTOS_SOURCES}")

add_executable(${PROJECT_NAME}
    main.c
    FreeRTOSConfig.h
    console.h console.c
)

# set_target_properties(${PROJECT_NAME} PROPERTIES
#        PUBLIC_HEADER "${OSDK_LIB_HEADERS}")

add_library(freertos STATIC ${FREERTOS_SOURCES})
target_link_libraries(${PROJECT_NAME} freertos)

# LDFLAGS := -pthread
target_link_libraries(${PROJECT_NAME} pthread)
# Other way to do it
#find_package(Threads REQUIRED)
#set(FPRIME_USE_POSIX ON)