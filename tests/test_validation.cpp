#include "hms/validation_service.hpp"

#include <cassert>
#include <iostream>

void testValidationFramework() {
    using hms::ValidationService;

    // String validation
    assert(ValidationService::isNonEmpty("Valid string"));
    assert(!ValidationService::isNonEmpty(""));
    assert(!ValidationService::isNonEmpty("   \t\n  "));

    // ID validation
    assert(ValidationService::isValidId("P-", "P-00001"));
    assert(ValidationService::isValidId("D-", "D-12345"));
    assert(!ValidationService::isValidId("P-", "D-00001"));
    assert(!ValidationService::isValidId("P-", "P-"));
    assert(!ValidationService::isValidId("P-", "P-ABC"));

    // Date validation
    assert(ValidationService::isValidDate("2026-11-04"));
    assert(ValidationService::isValidDate("2024-02-29")); // Leap year
    assert(!ValidationService::isValidDate("2023-02-29")); // Non-leap year
    assert(!ValidationService::isValidDate("2026-13-01"));
    assert(!ValidationService::isValidDate("invalid-date"));

    // Future or today date validation
    assert(ValidationService::isFutureOrTodayDate("2099-01-01"));
    assert(!ValidationService::isFutureOrTodayDate("1990-01-01"));

    // Time validation
    assert(ValidationService::isValidTime("09:30"));
    assert(ValidationService::isValidTime("23:59"));
    assert(!ValidationService::isValidTime("24:00"));
    assert(!ValidationService::isValidTime("12:60"));
    assert(!ValidationService::isValidTime("bad"));

    // Phone validation
    assert(ValidationService::isValidPhone("9876543210"));
    assert(ValidationService::isValidPhone("+1 (555) 019-2834"));
    assert(!ValidationService::isValidPhone("123")); // too short
    assert(!ValidationService::isValidPhone("1234567890123456")); // too long

    // Email validation
    assert(ValidationService::isValidEmail("user@hospital.org"));
    assert(ValidationService::isValidEmail("")); // optional
    assert(!ValidationService::isValidEmail("no-at-sign.com"));
    assert(!ValidationService::isValidEmail("missing-domain@"));

    // Amount validation
    assert(ValidationService::isPositiveAmount(10.50));
    assert(!ValidationService::isPositiveAmount(0.0));
    assert(!ValidationService::isPositiveAmount(-5.0));
    assert(ValidationService::isNonNegativeAmount(0.0));
    assert(ValidationService::isNonNegativeAmount(100.0));
    assert(!ValidationService::isNonNegativeAmount(-0.01));

    // Sanitization
    assert(ValidationService::sanitizeInput("  trim me  ") == "trim me");

    std::cout << "testValidationFramework passed\n";
}

int main() {
    testValidationFramework();
    std::cout << "All validation tests passed successfully.\n";
    return 0;
}
