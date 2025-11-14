cmake_minimum_required(VERSION 3.10)
project(NeuroLock VERSION 1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2")

# Find required packages
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${OPENSSL_INCLUDE_DIR})

# Source files
set(SOURCES
    src/main.c
    src/capture.c
    src/feature_extraction.c
    src/hashing.c
    src/template.c
    src/utils.c
)

# Create executable
add_executable(neurolock ${SOURCES})

# Link libraries
target_link_libraries(neurolock 
    ${OPENSSL_LIBRARIES}
    m  # Math library
)

# Windows-specific settings
if(WIN32)
    target_link_libraries(neurolock ws2_32 crypt32)
endif()

# Create templates directory on build
add_custom_command(TARGET neurolock POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory templates
    COMMENT "Creating templates directory"
)

# Install target
install(TARGETS neurolock DESTINATION bin)

# CPack configuration for packaging
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Print configuration
message(STATUS "")
message(STATUS "NeuroLock Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  OpenSSL Found: ${OPENSSL_FOUND}")
message(STATUS "  OpenSSL Version: ${OPENSSL_VERSION}")
message(STATUS "")
