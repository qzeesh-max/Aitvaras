cmake_minimum_required(VERSION 3.20)
project(Aitvaras VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

# Require GCC with reflection support (-freflection)
add_compile_options(-freflection -O3 -Wall -Wextra)

add_library(aitvaras INTERFACE)
target_include_directories(aitvaras INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_marshaler test/test_marshaler.cpp)
target_link_libraries(test_marshaler PRIVATE aitvaras gtest_main)

include(GoogleTest)
gtest_discover_tests(test_marshaler)

add_executable(test_protocols test/test_protocols.cpp test/test_protocols_generated.cpp)
target_link_libraries(test_protocols PRIVATE aitvaras gtest_main)
gtest_discover_tests(test_protocols)

add_executable(test_appendage_lifecycle test/test_appendage_lifecycle.cpp)
target_link_libraries(test_appendage_lifecycle PRIVATE aitvaras gtest_main)
gtest_discover_tests(test_appendage_lifecycle)
