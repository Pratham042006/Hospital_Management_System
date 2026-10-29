cmake_minimum_required(VERSION 3.16)

project(HospitalManagementSystem VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(hms_core
    src/appointment_repository.cpp
    src/auth.cpp
    src/doctor_repository.cpp
    src/patient_repository.cpp
    src/patient_validation.cpp
    src/hospital_system.cpp
)

target_include_directories(hms_core PUBLIC include)
if(MSVC)
    target_compile_options(hms_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(hms_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(hms
    src/main.cpp
    src/console_app.cpp
)
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

add_executable(security_tests tests/test_security.cpp)
target_link_libraries(security_tests PRIVATE hms_core)
add_test(NAME security_tests COMMAND security_tests)

add_executable(doctor_tests tests/test_doctors.cpp)
target_link_libraries(doctor_tests PRIVATE hms_core)
add_test(NAME doctor_tests COMMAND doctor_tests)

add_executable(appointment_scheduling_tests tests/test_appointment_scheduling.cpp)
target_link_libraries(appointment_scheduling_tests PRIVATE hms_core)
add_test(NAME appointment_scheduling_tests COMMAND appointment_scheduling_tests)

add_executable(appointment_modification_tests tests/test_appointment_modification.cpp)
target_link_libraries(appointment_modification_tests PRIVATE hms_core)
add_test(NAME appointment_modification_tests COMMAND appointment_modification_tests)

add_executable(usability_tests tests/test_usability.cpp)
target_link_libraries(usability_tests PRIVATE hms_core)
add_test(NAME usability_tests COMMAND usability_tests)

add_executable(persistence_performance_tests tests/test_persistence_performance.cpp)
target_link_libraries(persistence_performance_tests PRIVATE hms_core)
add_test(NAME persistence_performance_tests COMMAND persistence_performance_tests)

