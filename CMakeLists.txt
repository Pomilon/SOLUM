cmake_minimum_required(VERSION 3.10)

# Read version from file
file(READ "VERSION" SOLUM_VERSION_RAW)
string(STRIP "${SOLUM_VERSION_RAW}" SOLUM_VERSION)

project(libsolum VERSION ${SOLUM_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
include_directories(include third_party)

# Library
add_library(solum 
    src/parser.cpp 
    src/renderer.cpp
)
target_include_directories(solum PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# CLI Tool
add_executable(solum_cli src/main.cpp)
target_link_libraries(solum_cli solum)
target_compile_definitions(solum_cli PRIVATE libsolum_VERSION="${SOLUM_VERSION}")

# Tests
enable_testing()
find_package(GTest REQUIRED)
add_executable(test_solum tests/test_solum.cpp)
target_link_libraries(test_solum solum GTest::gtest_main)
add_test(NAME SolumTests COMMAND test_solum)

# Installation rules for easy adoption
install(TARGETS solum solum_cli
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY include/ DESTINATION include)

