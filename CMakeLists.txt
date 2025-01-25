cmake_minimum_required(VERSION 3.0)
project(FileSystemManager
        VERSION 1.0
        DESCRIPTION "A simple file system manager"
        LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source files
set(SOURCES
    utils.cpp
    base_object.cpp
    file.cpp
    directory.cpp
    file_manager_system.cpp
    main.cpp
)

# Add header files
set(HEADERS
    utils.h
    base_object.h
    file.h
    directory.h
    file_system_manager.h
)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# Define the executable
add_executable(FileSystemManager ${SOURCES} ${HEADERS})

# Add warnings for development
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(FileSystemManager PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(FileSystemManager PRIVATE /W4)
endif()

# Add a README target for convenience
add_custom_target(readme
    COMMAND ${CMAKE_COMMAND} -E cat README.md
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Displaying README.md"
)
