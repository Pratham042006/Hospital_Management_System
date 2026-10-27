cmake_minimum_required(VERSION 3.16)

project(HospitalManagementSystem VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(hms_core
    src/auth.cpp
    src/patient_repository.cpp
    src/patient_validation.cpp
    src/hospital_system.cpp
)

target_include_directories(hms_core PUBLIC include)
target_compile_options(hms_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(hms src/main.cpp)
target_link_libraries(hms PRIVATE hms_core)

enable_testing()
add_executable(hms_tests tests/test_auth.cpp)
target_link_libraries(hms_tests PRIVATE hms_core)
add_test(NAME hms_tests COMMAND hms_tests)

add_executable(registration_tests tests/test_registration.cpp)
target_link_libraries(registration_tests PRIVATE hms_core)
add_test(NAME registration_tests COMMAND registration_tests)

add_executable(update_tests tests/test_update.cpp)
target_link_libraries(update_tests PRIVATE hms_core)
add_test(NAME update_tests COMMAND update_tests)

add_executable(search_tests tests/test_search.cpp)
target_link_libraries(search_tests PRIVATE hms_core)
add_test(NAME search_tests COMMAND search_tests)

add_executable(performance_tests tests/test_performance.cpp)
target_link_libraries(performance_tests PRIVATE hms_core)
add_test(NAME performance_tests COMMAND performance_tests)
