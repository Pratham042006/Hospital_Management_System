#include "test_support.hpp"

#include <climits>
#include <cstdint>
#include <iostream>

// NFR-06 - Portability: the source must build on common C/C++ compilers with
// minimal changes and without unnecessary platform-specific dependencies.

// ---- Compile-time checks: standard C++17 and fixed-width integer conformance.
#if defined(_MSVC_LANG)
static_assert(_MSVC_LANG >= 201703L, "C++17 or newer is required");
#else
static_assert(__cplusplus >= 201703L, "C++17 or newer is required");
#endif
static_assert(sizeof(std::uint64_t) == 8, "password digest assumes a 64-bit integer");
static_assert(CHAR_BIT == 8, "8-bit bytes are assumed");

namespace {

int failures = 0;

#define CHECK(condition)                                                                    \
    do {                                                                                    \
        if (!(condition)) {                                                                 \
            std::cerr << "CHECK failed at line " << __LINE__ << ": " #condition << '\n'; \
            ++failures;                                                                     \
        }                                                                                   \
    } while (false)

const char* compilerName() {
#if defined(_MSC_VER)
    return "MSVC";
#elif defined(__clang__)
    return "Clang";
#elif defined(__GNUC__)
    return "GCC";
#else
    return "unknown";
#endif
}

const char* platformName() {
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#else
    return "unknown";
#endif
}

}  // namespace

int main() {
    std::cout << "Target platform: " << platformName() << ", Compiler: " << compilerName()
              << ", Pointer width: " << sizeof(void*) * CHAR_BIT << " bits\n";

    // Verify core system behavior that must be deterministic across all toolchains
    auto system = makeTestSystem();
    CHECK(system.login("admin", "test-admin-password").has_value());
    CHECK(!system.login("admin", "invalid-pass").has_value());
    CHECK(!system.login("nonexistent", "test-admin-password").has_value());
    CHECK(system.login("doctor", "test-doctor-password").has_value());

    if (failures != 0) {
        std::cerr << failures << " portability check assertion(s) failed.\n";
        return 1;
    }
    std::cout << "NFR-06 portability checks passed successfully.\n";
    return 0;
}
