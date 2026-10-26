cmake_minimum_required(VERSION 3.16)

project(HospitalManagementSystem VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(hms_core
    src/auth.cpp
)

target_include_directories(hms_core PUBLIC include)
target_compile_options(hms_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(hms src/main.cpp)
target_link_libraries(hms PRIVATE hms_core)

enable_testing()
add_executable(hms_tests tests/test_auth.cpp)
target_link_libraries(hms_tests PRIVATE hms_core)
add_test(NAME hms_tests COMMAND hms_tests)
