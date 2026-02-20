cmake_minimum_required(VERSION 3.10)
project(ST3215 VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Library source files
set(LIBRARY_SOURCES
    src/port_handler.cpp
    src/protocol_packet_handler.cpp
    src/st3215.cpp
    src/group_sync_write.cpp
    src/group_sync_read.cpp
)

# Create shared library
add_library(st3215 SHARED ${LIBRARY_SOURCES})
target_include_directories(st3215 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Create static library
add_library(st3215_static STATIC ${LIBRARY_SOURCES})
target_include_directories(st3215_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(st3215_static PROPERTIES OUTPUT_NAME st3215)

# Add pthread for threading support
find_package(Threads REQUIRED)
target_link_libraries(st3215 PRIVATE Threads::Threads)
target_link_libraries(st3215_static PRIVATE Threads::Threads)

# Examples (optional)
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS st3215 st3215_static
    EXPORT st3215Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT st3215Targets
    FILE ST3215Targets.cmake
    NAMESPACE ST3215::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ST3215
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ST3215ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ST3215Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ST3215Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ST3215
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ST3215Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ST3215ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ST3215
)
